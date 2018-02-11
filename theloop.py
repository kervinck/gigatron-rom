#!/usr/bin/env python
#-----------------------------------------------------------------------
#
#  Core video, sound and interpreter loop for Gigatron TTL microcomputer
#  - 6.25MHz clock
#  - Rendering 160x120 pixels at 6.25MHz with flexible videoline programming
#  - Must stay above 31 kHz horizontal sync --> 200 cycles/scanline
#  - Must stay above 59.94 Hz vertical sync --> 521 scanlines/frame
#  - 4 channels sound
#  - 16-bits vCPU interpreter
#  - Builtin vCPU programs
#  - Serial input handler
#  - Soft reset button (keep 'Start' button down for 2 seconds)
#
#  Hopefully in ROM v1
#  XXX Audio: Move shift table to page 7, then add waveform synthesis [ROMv1]
#
#  After ROM v1 release
#  XXX Readability of asm.py instructions, esp. make d() implicit
#  XXX GCL: Prefix notation for high/low byte >X++ instead of X>++
#  XXX GCL: Rethink i, i. i; i= x, x. x= x: consistency, also POKEW, STLW etc
#  XXX How it works memo: brief description of every software function
#
#  Probably not in ROM v1
#  XXX Music sequencer (combined with LED sequencer)
#  XXX Adjustable return address for LUP/LOOKUP trampolines
#  XXX Loader: make noise when data comes in
#  XXX vCPU: Multiplication (mulShift8?)
#  XXX vCPU: PUSHA, POPA
#  XXX vCPU: Interrupts / Task switching (e.g for clock, LED sequencer)
#  XXX Scroll out the top line of text, or generic vertical scroll SYS call
#  XXX Intro: Rising planet?
#  XXX Multitasking/threading/sleeping (start with date/time clock in GCL)
#  XXX Scoping for variables or some form of local variables? $i ("localized")
#  XXX Simple GCL programs might be compiled by the host instead of offline?
#  XXX vCPU: Clear just vAC[0:7] (Workaround is not bad: |255 ^255)
#  XXX Random dots screensaver
#  XXX Star field
#
#  Future applications after ROM v1
#  XXX Pacman ghosts. Sprites by scan line 4 reset method? ("videoG"=graphics)
#  XXX Audio: Decay, using Karplus-Strong
#  XXX ROM data compression (starting with Jupiter and Racer image)
#  XXX Font screen 16x8 chars
#  XXX Info screen (zero page)
#  XXX Gigatron layout balls/bricks game
#  XXX Maze game. Berzerk/Robotron? Pac Mac
#  XXX Horizontal scroller. Flappy Bird
#  XXX Primes, Fibonacci (bignum), Queens
#  XXX Game of Life (edit <-> stop <-> slow <-> fast)
#  XXX Game #5 Shooter. Space Invaders, Demon Attack, Galaga style
#  XXX Exhibition mode: flip between applications in auto-play mode
#-----------------------------------------------------------------------

from sys import argv
from os  import getenv

from asm import *
import gcl
import font

# Gigatron clock
cpuClock = 6.250e+06

# Output pin assignment for VGA
R, G, B, hSync, vSync = 1, 4, 16, 64, 128
syncBits = hSync+vSync # Both pulses negative

# When the XOUT register is in the circuit, the rising edge triggers its update.
# The loop can therefore not be agnostic to the horizontal pulse polarity.
assert(syncBits & hSync != 0)

# VGA 640x480 defaults (to be adjusted below!)
vFront = 10     # Vertical front porch
vPulse = 2      # Vertical sync pulse
vBack  = 33     # Vertical back porch
vgaLines = vFront + vPulse + vBack + 480
vgaClock = 25.175e+06

# Video adjustments for Gigatron
# 1. Our clock is (slighty) slower than 1/4th VGA clock. Not all monitors will
#    accept the decreased frame rate, so we restore the frame rate to above
#    minimum 59.94 Hz by cutting some lines from the vertical front porch.
vFrontAdjust = vgaLines - int(4 * cpuClock / vgaClock * vgaLines)
vFront -= vFrontAdjust
# 2. Extend vertical sync pulse so we can feed the game controller the same
#    signal. This is needed for controllers based on the 4021 instead of 74165
vPulseExtension = max(0, 8-vPulse)
vPulse += vPulseExtension
# 3. Borrow these lines from the back porch so the refresh rate remains
#    unaffected
vBack -= vPulseExtension

# Game controller bits (actual controllers in kit have negative output)
# +-------------------------------------+
# |       Up                      B*    |
# |  Left + Right              B     A* |
# |      Down     Select Start    A     |
# +-------------------------------------+ *=Auto fire
buttonRight     = 1
buttonLeft      = 2
buttonDown      = 4
buttonUp        = 8
buttonStart     = 16
buttonSelect    = 32
buttonB         = 64
buttonA         = 128

# Compile option: True restricts the calling of interpreter to calls from
# page 2, for 2 cycles less interpreter ENTER/EXIT overhead
fastRunVcpu = True

#-----------------------------------------------------------------------
#
#  RAM page 0: variables
#
#-----------------------------------------------------------------------

# Memory size in pages from auto-detect
memSize         = zpByte()

# The current channel number for sound generation. Advanced every scan line
# and independent of the vertical refresh to maintain constant oscillation.
channel         = zpByte()

# Next sound sample being synthesized
sample          = zpByte()
# To save one instruction in the critical inner loop, `sample' is always
# reset with its own address instead of, for example, the value 0. Compare:
# 1 instruction reset
#       st sample,[sample]
# 2 instruction reset:
#       ld 0
#       st [sample]
# The difference is unhearable. This is fine when the reset/address
# value is low and doesn't overflow with 4 channels added to it.
# There is an alternative, but it requires pull-down diodes on the data bus:
#       st [sample],[sample]
assert 4*63 + sample < 256
# We pin this reset/address value to 3, so `sample' swings from 3 to 255
assert sample == 3

# Booting
bootCount       = zpByte() # 0 for cold boot
bootCheck       = zpByte() # Checksum

# Entropy harvested from SRAM startup and controller input
entropy         = zpByte(3)

# Visible video
videoY          = zpByte() # Counts up from 0 to 238 in steps of 2
                           # Counts up during vertical blank (-44/-40 to 0)
frameX          = zpByte() # Starting byte within page
frameY          = zpByte() # Page of current pixel row (updated by videoA)
nextVideo       = zpByte() # Jump offset to scan line handler (videoA, B, C...)
videoDorF       = zpByte() # Handler for every 4th line (videoD or videoF)

# Vertical blank (reuse some variables used in the visible part)
videoSync0      = frameX   # Vertical sync type on current line (0xc0 or 0x40)
videoSync1      = frameY   # Same during horizontal pulse

# Frame counter is good enough as system clock
frameCount      = zpByte(1)

# Serial input (game controller)
serialRaw       = zpByte() # New raw serial read
serialLast      = zpByte() # Previous serial read
buttonState     = zpByte() # Clearable button state
resetTimer      = zpByte() # After 2 seconds of holding 'Start', do a soft reset

# Extended output (blinkenlights in bit 0:3 and audio in but 4:7). This
# value must be present in AC during a rising hSync edge. It then gets
# copied to the XOUT register by the hardware. The XOUT register is only
# accessible in this indirect manner because it isn't part of the core
# CPU architecture.
xout            = zpByte()
xoutMask        = zpByte() # The blinkenlights and sound on/off state

# vCPU interpreter
vTicks          = zpByte()  # Interpreter ticks are units of 2 clocks
vPC             = zpByte(2) # Interpreter program counter, points into RAM
vAC             = zpByte(2) # Interpreter accumulator, 16-bits
vLR             = zpByte(2) # Return address, for returning after CALL
vSP             = zpByte(1) # Stack pointer
vTmp            = zpByte()
if fastRunVcpu:
  vReturn       = zpByte(1) # Return into video loop
  reserved31    = zpByte(1)
else:
  vReturn       = zpByte(2) # Return into video loop

# For future ROM extensions
reserved32       = zpByte()

# ROM type/version, numbering scheme to be determined, could be as follows:
# bit 4:7       Version
# bit 0:3       >=8 Formal revisions 8=alpa, 9=beta, 10=beta2...c=release, d=patch
#                <8 experimental/informal revisions
# Perhaps it should just identify the application bindings,
# so don't call it romVersion already
romType         = zpByte(1)

# SYS function arguments and results/scratch
sysFn           = zpByte(2)
sysArgs         = zpByte(8)

# Play sound if non-zero, count down and stop sound when zero
soundTimer      = zpByte()

# Fow now the LED state machine itself is hard-coded in the program ROM
ledTimer        = zpByte() # Number of ticks until next LED change
ledState        = zpByte() # Current LED state
ledTempo        = zpByte() # Next value for ledTimer after LED state change

# All bytes above, except 0x80, are free for temporary/scratch/stacks etc
zpFree          = zpByte(0)
print 'zpFree %04x' % zpFree

#-----------------------------------------------------------------------
#
#  RAM page 1: video line table
#
#-----------------------------------------------------------------------

# Byte 0-239 define the video lines
videoTable = 0x0100 # Indirection table: Y[0] dX[0]  ..., Y[119] dX[119]

# Highest bytes are for channel 1 variables

# Sound synthesis  ch1   ch2   ch3   ch4
wavX = 249
keyL = 250
keyH = 251
     # 252 reserved for shift table
wavA = 253
oscL = 254
oscH = 255

#-----------------------------------------------------------------------
#
#  RAM page 2: shift table
#
#-----------------------------------------------------------------------

audioTable = 0x0200

#-----------------------------------------------------------------------
#
#  RAM page 3-7: application code GCL
#
#-----------------------------------------------------------------------

vCpuStart = 0x0300

#-----------------------------------------------------------------------
#  Memory layout
#-----------------------------------------------------------------------

screenPages   = 0x80 - 120 # Default start of screen memory: 0x0800 to 0x7fff

#-----------------------------------------------------------------------
#  Application definitions
#-----------------------------------------------------------------------

maxTicks = 28/2 # Duration of slowest virtual opcode
define('$maxTicks', maxTicks)

vOverheadInt = 9 # Overhead of jumping in and out. Cycles, not ticks
vOverheadExt = 5 if fastRunVcpu else 7

maxSYS = -999 # Largest time slice for 'SYS
minSYS = +999 # Smallest time slice for 'SYS'

def runVcpu(n, ref=None):
  """Run interpreter for exactly n cycles"""
  comment = 'Run vCPU for %s cycles' % n
  if ref:
    comment += ' (%s)' % ref
  if n % 2 != (vOverheadExt + vOverheadInt) % 2:
    nop()
    comment = C(comment)
    n -= 1
  n -= vOverheadExt + vOverheadInt

  print 'runVcpu at %04x cycles %3s info %s' % (pc(), n, ref)
  n -= 2*maxTicks

  assert n >= 0 and n % 2 == 0

  global maxSYS, minSYS
  maxSYS = max(maxSYS, n + 2*maxTicks)
  minSYS = min(minSYS, n + 2*maxTicks)
  # Tell GCL compiler this range, so it can check SYS call operands
  define('$maxSYS', maxSYS)
  define('$minSYS', minSYS)

  n /= 2
  returnPc = pc() + (5 if fastRunVcpu else 7)
  ld(val(returnPc&255))         #0
  comment = C(comment)
  st(d(vReturn))                #1
  if fastRunVcpu:
    # In this mode [vReturn+1] will not be used
    assert returnPc>>8 == 2
  else:
    # Allow interpreter to be called from anywhere
    ld(val(returnPc>>8))        #2
    st(d(vReturn+1))            #3
  ld(val(hi('ENTER')),regY)     #4
  jmpy(d(lo('ENTER')))          #5
  ld(val(n))                    #6

#-----------------------------------------------------------------------
#
#  ROM page 0: Boot
#
#-----------------------------------------------------------------------

align(0x100, 0x100)

# Give a first sign of life that can be checked with a voltmeter
ld(val(0b0000));                C('LEDs |OOOO|')
ld(val(syncBits^hSync), regOUT) # Prepare XOUT update, hSync goes down, RGB to black
ld(val(syncBits), regOUT)       # hSync goes up, updating XOUT

# Simple RAM test and size check by writing to [1<<n] and see if [0] changes.
ld(val(1));                     C('RAM test and count')
label('.countMem0')
st(d(memSize), busAC|ea0DregY)
ld(val(255))
xora(d(0), busRAM|eaYDregAC)
st(d(0), busAC|eaYDregAC)       # Test if we can change and read back ok
st(d(0))                        # Preserve (inverted) memory value in [0]
xora(d(0), busRAM|eaYDregAC)
bne(d(pc()))                    # Just hang here on apparent RAM failure
ld(val(255))
xora(d(0), busRAM|eaYDregAC)
st(d(0), busAC|eaYDregAC)
xora(d(0), busRAM)
beq(d(lo('.countMem1')))        # Wrapped and [0] changed as well
ldzp(d(memSize))
bra(d(lo('.countMem0')))
adda(busAC)
label('.countMem1')

# Momentarily wait to allow for debouncing of the reset switch by spinning
# roughly 2^15 times at 2 clocks per loop: 6.5ms@10MHz to 10ms@6.3MHz
# Real-world switches normally bounce shorter than that.
# "[...] 16 switches exhibited an average 1557 usec of bouncing, with,
#  as I said, a max of 6200 usec" (From: http://www.ganssle.com/debouncing.htm)
# Relevant for the breadboard version, as the kit doesn't have a reset switch.

ld(val(255));                   C('Debounce reset button')
label('.debounce')
st(d(0))
bne(d(pc()))
suba(val(1))
ldzp(d(0))
bne(d(lo('.debounce')))
suba(val(1))

# Update LEDs (memory is present and counted, reset is stable)
ld(val(0b0001));                C('LEDs |*OOO|')
ld(val(syncBits^hSync),regOUT)
ld(val(syncBits),regOUT)

# Scan the entire RAM space to collect entropy for a random number generator.
# The 16-bit address space is scanned, even if less RAM was detected.
ld(val(0));                     C('Collect entropy from RAM')
st(d(vAC+0),busAC|ea0DregX)
st(d(vAC+1),busAC|ea0DregY)
label('.initEnt0')
ldzp(d(entropy+0))
bpl(d(lo('.initEnt1')))
adda(busRAM|eaYXregAC)
xora(val(191))
label('.initEnt1')
st(d(entropy+0))
ldzp(d(entropy+1))
bpl(d(lo('.initEnt2')))
adda(d(entropy+0),busRAM)
xora(val(193))
label('.initEnt2')
st(d(entropy+1))
adda(d(entropy+2),busRAM)
st(d(entropy+2))
ldzp(d(vAC+0))
adda(val(1))
bne(d(lo('.initEnt0')))
st(d(vAC+0),busAC|ea0DregX)
ldzp(d(vAC+1))
adda(val(1))
bne(d(lo('.initEnt0')))
st(d(vAC+1),busAC|ea0DregY)

# Update LEDs
ld(val(0b0011));                 C('LEDs |**OO|')
ld(val(syncBits^hSync),regOUT)
ld(val(syncBits),regOUT)

# Determine if this is a cold or a warm start. We do this by checking the
# boot counter and comparing it to a simplistic checksum. The assumption
# is that after a cold start the checksum is invalid.

ldzp(d(bootCount));             C('Cold or warm boot?')
adda(d(bootCheck),busRAM)
adda(d(0x5a))
bne(d(lo('cold')))
ld(val(0))
label('warm')
ldzp(d(bootCount))              # if warm start: bootCount += 1
adda(val(1))
label('cold')
st(d(bootCount))                # if cold start: bootCount = 0
xora(val(255))
suba(val(0x5a-1))
st(d(bootCheck))

# vCPU reset handler
vReset = videoTable + 240 # we have 9 unused bytes behind the video table
ld(val((vReset&255)-2));     C('Setup vCPU reset handler')
st(d(vPC))
adda(val(2),regX)
ld(val(vReset>>8))
st(d(vPC+1),busAC|regY)
st(d(lo('LDI')),         eaYXregOUTIX)
st(d(lo('SYS_Reset_36')),eaYXregOUTIX)
st(d(lo('STW')),         eaYXregOUTIX)
st(d(sysFn),             eaYXregOUTIX)
st(d(lo('SYS')),         eaYXregOUTIX)
st(d(256-36/2+maxTicks), eaYXregOUTIX)
st(d(lo('SYS')),         eaYXregOUTIX) # SYS_Exec_88
st(d(256-88/2+maxTicks), eaYXregOUTIX)

ld(val(255));                   C('Setup serial input')
st(d(frameCount))
st(d(serialRaw))
st(d(serialLast))
st(d(buttonState))
st(d(resetTimer))

# XXX Everything below should at one point migrate to Reset.gcl

# Init the shift2-right table for sound
ld(val(audioTable>>8),regY);    C('Setup shift2 table')
ld(val(0))
st(d(channel))
label('.loop')
st(d(vAC+0))
adda(busAC)
adda(busAC, regX)
ldzp(d(vAC+0))
st(eaYXregAC)
adda(val(1))
xora(val(0x40))
bne(d(lo('.loop')))
xora(val(0x40))

# Init LED sequencer
ld(val(120));                   C('Setup LED sequencer')
st(d(ledTimer))
ld(val(60/6))
st(d(ledTempo))
ld(val(0))
st(d(ledState))

ld(val(0b0111));                C('LEDs |***O|')
ld(val(syncBits^hSync),regOUT)
ld(val(syncBits),regOUT)

ld(val(0b1111));                C('LEDs |****|')
ld(val(syncBits^hSync),regOUT)
ld(val(syncBits),regOUT)
st(d(xout)) # Setup for control by video loop
st(d(xoutMask))

ld(d(hi('vBlankStart')),busD|ea0DregY);C('Enter video loop')
jmpy(d(lo('vBlankStart')))
ld(val(syncBits))

#-----------------------------------------------------------------------
# Room for extension
#-----------------------------------------------------------------------

nop()
nop()
nop()
nop()
nop()
nop()
nop()
nop()
nop()

#-----------------------------------------------------------------------
# Extension SYS_Reset_36: Soft reset
#-----------------------------------------------------------------------

# SYS_Reset_36 initiates an immediate Gigatron reset from within the vCPU.
# The reset sequence itself is mostly implemented in GCL by Reset.gcl .
# This must first be loaded into RAM. But as that takes more than 1 scanline,
# some vCPU bootstrapping code gets loaded with SYS_Exec_88. The caller of
# SYS_Reset_36 provides the SYS instruction to execute that.

label('SYS_Reset_36')
assert(pc()>>8==0)
value = getenv('romType')
value = int(value, 0) if value else 0
ld(d(value));                           C('Set ROM type/version')#15
st(d(romType))                          #16
ld(val(0))                              #17
st(d(vSP))                              #18 Reset stack pointer
assert(vCpuStart&255==0)
st(d(vLR))                              #19
st(d(soundTimer))                       #20
ld(val(vCpuStart>>8))                   #21
st(d(vLR+1))                            #22
ld(d(lo('videoF')))                     #23 Do this before first visible pixels
st(d(videoDorF))                        #24
ld(d(lo('SYS_Exec_88')))                #25
st(d(sysFn))                            #26 High byte (remains) 0
ld(d(lo('Reset')))                      #27
st(d(sysArgs+0))                        #28
ld(d(hi('Reset')))                      #29
st(d(sysArgs+1))                        #30
# Return to interpreter
ld(val(hi('REENTER')),regY)             #31
jmpy(d(lo('REENTER')))                  #32
ld(val(-36/2))                          #33

#-----------------------------------------------------------------------
# Extension SYS_Exec_88: Load code from ROM into memory and execute it
#-----------------------------------------------------------------------
#
# This loads the vCPU code with consideration of the current vSP
# Used during reset, but also for switching between applications
# or for loading data from ROM during an application.
#
# ROM stream format is [<addrH> <addrL> <n&255> n*<byte>]* 0
# on top of lookup tables.
#
# Variables:
#       sysArgs[0:1]    ROM pointer (input set by caller)
#       sysArgs[2:3]    RAM pointer (variable)
#       sysArgs[4]      State counter (variable)
#       vLR             vCPU continues here (input set by caller)

label('SYS_Exec_88')
assert(pc()>>8==0)
ld(val(0))                              #15 Address of loader on zero page
st(d(vPC+1),busAC|regY)                 #16
ldzp(d(vSP))                            #17 Below the current stack pointer
suba(d(53+2))                           #18 (AC -> *+0)
st(d(vTmp),busAC|regX)                  #19
adda(val(-2))                           #20 (AC -> *-2)
st(d(vPC))                              #21
# Start of manually compiled vCPU section
st(d(lo('PUSH')    ),eaYXregOUTIX)      #22 *+0
st(d(lo('BRA')     ),eaYXregOUTIX)      #23 *+1
adda(val(26))                           #24 (AC -> *+24)
st(                  eaYXregOUTIX)      #25 *+2
st(d(lo('ST')      ),eaYXregOUTIX)      #26 *+3 Chunk copy loop
st(d(sysArgs+3     ),eaYXregOUTIX)      #27 *+4 High-address came first
st(d(lo('CALL')    ),eaYXregOUTIX)      #28 *+5
adda(val(33-24))                        #29 (AC -> *+33)
st(                  eaYXregOUTIX)      #30 *+6
st(d(lo('ST')      ),eaYXregOUTIX)      #31 *+7
st(d(sysArgs+2     ),eaYXregOUTIX)      #32 *+8 Then the low address
st(d(lo('CALL')    ),eaYXregOUTIX)      #33 *+9
st(                  eaYXregOUTIX)      #34 *+10
st(d(lo('ST')      ),eaYXregOUTIX)      #35 *+11 Byte copy loop
st(d(sysArgs+4     ),eaYXregOUTIX)      #36 *+12 Byte count (0 means 256)
st(d(lo('CALL')    ),eaYXregOUTIX)      #37 *+13
st(                  eaYXregOUTIX)      #38 *+14
st(d(lo('POKE')    ),eaYXregOUTIX)      #39 *+15
st(d(sysArgs+2     ),eaYXregOUTIX)      #40 *+16
st(d(lo('INC')     ),eaYXregOUTIX)      #41 *+17
st(d(sysArgs+2     ),eaYXregOUTIX)      #42 *+18
st(d(lo('LD')      ),eaYXregOUTIX)      #43 *+19
st(d(sysArgs+4     ),eaYXregOUTIX)      #44 *+20
st(d(lo('SUBI')    ),eaYXregOUTIX)      #45 *+21
st(d(1             ),eaYXregOUTIX)      #46 *+22
st(d(lo('BCC')     ),eaYXregOUTIX)      #47 *+23
st(d(lo('NE')      ),eaYXregOUTIX)      #48 *+24
adda(val(11-2-33))                      #49 (AC -> *+9)
st(                  eaYXregOUTIX)      #50 *+25
st(d(lo('CALL')    ),eaYXregOUTIX)      #51 *+26 Go to next block
adda(val(33-9))                         #52 (AC -> *+33)
st(                  eaYXregOUTIX)      #53 *+27
st(d(lo('BCC')     ),eaYXregOUTIX)      #54 *+28
st(d(lo('NE')      ),eaYXregOUTIX)      #55 *+29
adda(val(3-2-33))                       #56 (AC -> *+1)
st(                  eaYXregOUTIX)      #57 *+30
st(d(lo('POP')     ),eaYXregOUTIX)      #58 *+31 End
st(d(lo('RET')     ),eaYXregOUTIX)      #59 *+32
# Pointer constant pointing to the routine below (for use by CALL)
adda(val(35-1))                         #60 (AC -> *+35)
st(                  eaYXregOUTIX)      #61 *+33
st(d(0             ),eaYXregOUTIX)      #62 *+34
# Routine to read next byte from ROM and advance read pointer
st(d(lo('LD')      ),eaYXregOUTIX)      #63 *+35 Test for end of ROM table
st(d(sysArgs+0     ),eaYXregOUTIX)      #64 *+36
st(d(lo('XORI')    ),eaYXregOUTIX)      #65 *+37
st(d(251           ),eaYXregOUTIX)      #66 *+38
st(d(lo('BCC')     ),eaYXregOUTIX)      #67 *+39
st(d(lo('NE')      ),eaYXregOUTIX)      #68 *+40
adda(val(46-2-35))                      #69 (AC -> *+44)
st(                  eaYXregOUTIX)      #70 *+41
st(d(lo('ST')      ),eaYXregOUTIX)      #71 *+42 Wrap to next ROM page
st(d(sysArgs+0     ),eaYXregOUTIX)      #72 *+43
st(d(lo('INC')     ),eaYXregOUTIX)      #73 *+44
st(d(sysArgs+1     ),eaYXregOUTIX)      #74 *+45
st(d(lo('LDW')     ),eaYXregOUTIX)      #75 *+46 Read next byte from ROM table
st(d(sysArgs+0     ),eaYXregOUTIX)      #76 *+47
st(d(lo('LUP')     ),eaYXregOUTIX)      #77 *+48
st(d(0             ),eaYXregOUTIX)      #78 *+49
st(d(lo('INC')     ),eaYXregOUTIX)      #79 *+50 Increment read pointer
st(d(sysArgs+0     ),eaYXregOUTIX)      #80 *+51
st(d(lo('RET')     ),eaYXregOUTIX)      #81 *+52 Return
# Return to interpreter
nop()                                   #82
ld(val(hi('REENTER')),regY)             #83
jmpy(d(lo('REENTER')))                  #84
ld(val(-88/2))                          #85

#-----------------------------------------------------------------------
# Extension SYS_Out_22: Send byte to output port
#-----------------------------------------------------------------------

label('SYS_Out_22')
ld(d(sysArgs+0),busRAM|regOUT)  #15
nop()                           #16
ld(val(hi('REENTER')),regY)     #17
jmpy(d(lo('REENTER')))          #18
ld(val(-22/2))                  #19

#-----------------------------------------------------------------------
# Extension SYS_In_24: Read a byte from the input port
#-----------------------------------------------------------------------

label('SYS_In_24')
st(d(vAC),busIN)                #15
ld(val(0))                      #16
st(d(vAC+1))                    #17
nop()                           #18
ld(val(hi('REENTER')),regY)     #19
jmpy(d(lo('REENTER')))          #20
ld(val(-24/2))                  #21

#-----------------------------------------------------------------------
# Extension SYS_Random_34: Update entropy and copy to vAC
#-----------------------------------------------------------------------

# This same algorithm runs automatically once per vertical blank.
# Use this function to get numbers at a higher rate.
label('SYS_Random_34')
ldzp(d(frameCount))             #15
xora(d(entropy+1),busRAM)       #16
xora(d(serialRaw),busRAM)       #17
adda(d(entropy+0),busRAM)       #18
st(d(entropy+0))                #19
st(d(vAC+0))                    #20
adda(d(entropy+2),busRAM)       #21
st(d(entropy+2))                #22
bmi(d(lo('.sysRnd0')))          #23
bra(d(lo('.sysRnd1')))          #24
xora(val(64+16+2+1))            #25
label('.sysRnd0')
xora(val(64+32+8+4))            #25
label('.sysRnd1')
adda(d(entropy+1),busRAM)       #26
st(d(entropy+1))                #27
st(d(vAC+1))                    #28
ld(val(hi('REENTER')),regY)     #29
jmpy(d(lo('REENTER')))          #30
ld(val(-34/2))                  #31

label('SYS_LSRW7_30')
ldzp(d(vAC))                    #15
anda(d(128),regX)               #16
ldzp(d(vAC+1))                  #17
adda(busAC)                     #18
ora(ea0XregAC,busRAM)           #19
st(d(vAC))                      #20
ldzp(d(vAC+1))                  #21
anda(d(128),regX)               #22
ld(ea0XregAC,busRAM)            #23
st(d(vAC+1))                    #24
ld(d(hi('REENTER')),regY)       #25
jmpy(d(lo('REENTER')))          #26
ld(d(-30/2))                    #27

assert pc()&255==0

#-----------------------------------------------------------------------
#
#  ROM page 1-2: Video loop
#
#-----------------------------------------------------------------------
align(0x100, 0x200)

# Back porch A: first of 4 repeated scan lines
# - Fetch next Yi and store it for retrieval in the next scan lines
# - Calculate Xi from dXi, but there is no cycle time left to store it as well
label('videoA')
assert(lo('videoA') == 0)       # videoA starts at the page boundary
ld(d(lo('videoB')))             #29
st(d(nextVideo))                #30
ld(d(videoTable>>8), regY)      #31
ld(d(videoY), busRAM|regX)      #32
ld(eaYXregAC, busRAM)           #33
st(eaYXregOUTIX)                #34 Just to increment X
st(d(frameY))                   #35
ld(eaYXregAC, busRAM)           #36
adda(d(frameX), busRAM|regX)    #37
ld(d(frameY), busRAM|regY)      #38
ld(val(syncBits))               #39

# Stream 160 pixels from memory location <Yi,Xi> onwards
# Superimpose the sync signal bits to be robust against misprogramming
label('pixels')
for i in range(160):
  ora(eaYXregOUTIX, busRAM)     #40-199
  if i==0: C('Pixel burst')
ld(val(syncBits), regOUT);      C('<New scan line start>')#0 Back to black

# Front porch
ldzp(d(channel));C('Advance to next sound channel')#1
label('soundF')
anda(val(3))                    #2
adda(val(1))                    #3
ld(val(syncBits^hSync), regOUT);C('Start horizontal pulse')#4

# Horizontal sync
label('sound2')
st(d(channel), busAC|ea0DregY)  #5 Sound
ld(val(0x7f))                   #6
anda(d(oscL), busRAM|eaYDregAC) #7
adda(d(keyL), busRAM|eaYDregAC) #8
st(d(oscL), busAC|eaYDregAC)    #9
anda(val(0x80), regX)           #10
ld(busRAM|ea0XregAC)            #11
adda(d(oscH), busRAM|eaYDregAC) #12
adda(d(keyH), busRAM|eaYDregAC) #13
st(d(oscH), busAC|eaYDregAC)    #14
nop()                           #15 Was: xora [y,wavX]
nop()                           #16 Was: adda [y,wavA]
anda(val(0xfc),regX)            #17
ld(d(audioTable>>8),regY)       #18
ld(busRAM|eaYXregAC)            #19
adda(d(sample), busRAM|ea0DregAC)#20
st(d(sample))                   #21
wait(26-22)                     #22
ldzp(d(xout))                   #26
bra(d(nextVideo) | busRAM)      #27
ld(val(syncBits), regOUT);      C('End horizontal pulse')#28

# Back porch B: second of 4 repeated scan lines
# - Recompute Xi from dXi and store for retrieval in the next scan lines
label('videoB')
ld(d(lo('videoC')))             #29
st(d(nextVideo))                #30
ld(d(videoTable>>8), regY)      #31
ldzp(d(videoY))                 #32
adda(d(1), regX)                #33
ldzp(d(frameX))                 #34
adda(eaYXregAC, busRAM)         #35
st(d(frameX), busAC|ea0DregX)   #36 Undocumented opcode "store in RAM and X"!
ld(d(frameY), busRAM|regY)      #37
bra(d(lo('pixels')))            #38
ld(val(syncBits))               #39

# Back porch C: third of 4 repeated scan lines
# - Nothing new to do, Yi and Xi are known
label('videoC')
ldzp(d(sample));                C('New sound sample is ready')#29 First something that didn't fit in the audio loop
ora(d(0x0f))                    #30
anda(d(xoutMask),busRAM|ea0DregAC)#31
st(d(xout))                     #32 Update [xout] with new sample (4 channels just updated)
st(val(sample),ea0DregAC|busD); C('Reset for next sample')#33 Reset for next sample
ldzp(d(videoDorF));             C('Mode for scan line 4')#34 Now back to video business
st(d(nextVideo))                #35
ld(d(frameX),busRAM|regX)       #36
ld(d(frameY),busRAM|regY)       #37
bra(d(lo('pixels')))            #38
ld(val(syncBits))               #39

# Back porch D: last of 4 repeated scan lines
# - Calculate the next frame index
# - Decide if this is the last line or not
label('videoD')                 # Default video mode
ld(d(frameX), busRAM|regX)      #29
ldzp(d(videoY))                 #30
suba(d((120-1)*2))              #31
beq(d(lo('.last')))             #32
ld(d(frameY), busRAM|regY)      #33
adda(d(120*2))                  #34 More pixel lines to go
st(d(videoY))                   #35
ld(d(lo('videoA')))             #36
st(d(nextVideo))                #37
bra(d(lo('pixels')))            #38
ld(val(syncBits))               #39
label('.last')
wait(36-34)                     #34 No more pixel lines
ld(d(lo('videoE')))             #36
st(d(nextVideo))                #37
bra(d(lo('pixels')))            #38
ld(val(syncBits))               #39

# Back porch "E": after the last line
# - Go back to program page 0 and enter vertical blank
label('videoE') # Exit visible area
ld(d(hi('vBlankStart')),ea0DregY)#29
jmpy(d(lo('vBlankStart'))   )   #30
ld(val(syncBits))               #31

# Back porch "F": scan lines and fast mode
label('videoF')                 # Fast video mode
ldzp(d(videoY))                 #29
suba(d((120-1)*2))              #30
bne(d(lo('.notlast')))          #31
adda(d(120*2))                  #32
bra(d(lo('.join')))             #33
ld(d(lo('videoE')))             #34 No more visible lines
label('.notlast')
st(d(videoY))                   #33 More visible lines
ld(d(lo('videoA')))             #34
label('.join')
st(d(nextVideo))                #35
runVcpu(199-36, 'line41-521 typeF')#36 Application (every 4th of scan lines 41-521)
ld(d(hi('soundF')), busD|ea0DregY)#199 XXX This is on the current page
jmpy(d(lo('soundF')));          C('<New scan line start>')#0
ldzp(d(channel))                #1 Advance to next sound channel

# Vertical blank part of video loop
label('vBlankStart')            # Start of vertical blank interval
assert(pc()&255<16)             # Assure that we are in the beginning of the next page

st(d(videoSync0));              C('Start of vertical blank interval')#32
ld(val(syncBits^hSync))         #33
st(d(videoSync1))               #34

# (Re)initialize carry table for robustness
st(d(0x00), ea0DregAC|busD);    C('Carry table')#35
ld(val(0x01))                   #36
st(d(0x80))                     #37

# It is nice to set counter before vCPU starts
ld(val(1-2*(vFront+vPulse+vBack-2)))#38 -2 because first and last are different
st(d(videoY))                   #39

# Uptime frame count (3 cycles)
ldzp(d(frameCount));            C('Frame counter')#40
adda(val(1))                    #41
st(d(frameCount))               #42

# Mix entropy (11 cycles)
xora(d(entropy+1),busRAM);      C('Mix entropy')#43
xora(d(serialRaw),busRAM)       #44 Mix in serial input
adda(d(entropy+0),busRAM)       #45
st(d(entropy+0))                #46
adda(d(entropy+2),busRAM)       #47 Some hidden state
st(d(entropy+2))                #48
bmi(d(lo('.rnd0')))             #49
bra(d(lo('.rnd1')))             #50
xora(val(64+16+2+1))            #51
label('.rnd0')
xora(val(64+32+8+4))            #51
label('.rnd1')
adda(d(entropy+1),busRAM)       #52
st(d(entropy+1))                #53

# LED sequencer (19 cycles)
ldzp(d(ledTimer));              C('Blinkenlight sequencer')#54
bne(d(lo('.leds4')))            #55
ld(d(lo('.leds0')))             #56
adda(d(ledState)|busRAM)        #57
bra(busAC)                      #58
bra(d(lo('.leds1')))            #59
label('.leds0')
ld(d(0b1111));C('LEDs |****|')  #60
ld(d(0b0111));C('LEDs |***O|')  #60
ld(d(0b0011));C('LEDs |**OO|')  #60
ld(d(0b0001));C('LEDs |*OOO|')  #60
ld(d(0b0010));C('LEDs |O*OO|')  #60
ld(d(0b0100));C('LEDs |OO*O|')  #60
ld(d(0b1000));C('LEDs |OOO*|')  #60
ld(d(0b0100));C('LEDs |OO*O|')  #60
ld(d(0b0010));C('LEDs |O*OO|')  #60
ld(d(0b0001));C('LEDs |*OOO|')  #60
ld(d(0b0011));C('LEDs |**OO|')  #60
ld(d(0b0111));C('LEDs |***O|')  #60
ld(d(0b1111));C('LEDs |****|')  #60
ld(d(0b1110));C('LEDs |O***|')  #60
ld(d(0b1100));C('LEDs |OO**|')  #60
ld(d(0b1000));C('LEDs |OOO*|')  #60
ld(d(0b0100));C('LEDs |OO*O|')  #60
ld(d(0b0010));C('LEDs |O*OO|')  #60
ld(d(0b0001));C('LEDs |*OOO|')  #60
ld(d(0b0010));C('LEDs |O*OO|')  #60
ld(d(0b0100));C('LEDs |OO*O|')  #60
ld(d(0b1000));C('LEDs |OOO*|')  #60
ld(d(0b1100));C('LEDs |OO**|')  #60
ld(d(0b1110+128))               #60
C('LEDs |O***|')
label('.leds1')
st(d(xoutMask))                 #61 Temporarily park new state here
bmi(d(lo('.leds2')))            #62
bra(d(lo('.leds3')))            #63
ldzp(d(ledState))               #64
label('.leds2')
ld(val(-1))                     #64
label('.leds3')
adda(val(1))                    #65
st(d(ledState))                 #66
bra(d(lo('.leds5')))            #67
ldzp(d(ledTempo))               #68 Setup the LED timer for the next period
label('.leds4')
wait(67-57)                     #57
ldzp(d(ledTimer))               #67
suba(d(1))                      #68
label('.leds5')
st(d(ledTimer))                 #69
ldzp(d(xoutMask))               #70 Low 4 bits are the LED output
anda(val(0b00001111))           #71 High bits will be restored below
st(d(xoutMask))                 #72

# When the total number of scan lines per frame is not an exact multiple of the
# (4) channels, there will be an audible discontinuity if no measure is taken.
# This static noise can be suppressed by swallowing the first `lines mod 4'
# partial samples after transitioning into vertical blank. This is easiest if
# the modulo is 0 (do nothing) or 1 (reset sample while in the first blank scan
# line). For the two other cases there is no solution yet: give a warning.
soundDiscontinuity = (vFront+vPulse+vBack) % 4
extra = 0
if soundDiscontinuity == 1:
  st(val(sample), ea0DregAC|busD) # XXX We're swallowing _2_ samples here!
  C('Sound continuity')
  extra += 1
if soundDiscontinuity > 1:
  print "Warning: sound discontinuity not supressed"

runVcpu(189-73-extra, 'line0')  #73 Application cycles (scan line 0)

# Sound on/off (6 cycles)
ldzp(d(soundTimer));            C('Sound on/off')#189
bne(d(lo('.snd0')))             #190
bra(d(lo('.snd1')))             #191
ld(val(0))                      #192 Sound off
label('.snd0')
ld(val(0xf0))                   #192 Sound on
label('.snd1')
ora(d(xoutMask),busRAM)         #193
st(d(xoutMask))                 #194

# Sound timer count down (XXX Replace by sequencer) (5 cycles)
ldzp(d(soundTimer));            C('Sound timer')#195
beq(d(lo('.snd2')))             #196
bra(d(lo('.snd3')))             #197
suba(val(1))                    #198
label('.snd2')
ld(val(0))                      #198
label('.snd3')
st(d(soundTimer))               #199

ld(d(videoSync0), busRAM|regOUT);C('<New scan line start>')#0

label('sound1')
ldzp(d(channel));               C('Advance to next sound channel')#1
anda(val(3))                    #2
adda(val(1))                    #3
ld(d(videoSync1), busRAM|regOUT);C('Start horizontal pulse')#4
st(d(channel), busAC|ea0DregY)  #5
ld(val(0x7f))                   ;C('Update sound channel')#6
anda(d(oscL), busRAM|eaYDregAC) #7
adda(d(keyL), busRAM|eaYDregAC) #8
st(d(oscL), busAC|eaYDregAC)    #9
anda(val(0x80), regX)           #10
ld(busRAM|ea0XregAC)            #11
adda(d(oscH), busRAM|eaYDregAC) #12
adda(d(keyH), busRAM|eaYDregAC) #13
st(d(oscH), busAC|eaYDregAC)    #14
nop()                           #15 Was: xora [y,wavX]
nop()                           #16 Was: adda [y,wavA]
anda(val(0b11111100),regX)      #17
ld(d(audioTable>>8),regY)       #18
ld(busRAM|eaYXregAC)            #19
adda(d(sample), busRAM|ea0DregAC)#20
st(d(sample))                   #21
wait(26-22)                     #22
ldzp(d(xout))                   #26
nop()                           #27
ld(d(videoSync0), busRAM|regOUT);C('End horizontal pulse')#28

# Count through the vertical blank interval until its last scan line
ldzp(d(videoY))                 #29
bpl(d(lo('vBlankLast')))        #30
adda(d(2))                      #31
st(d(videoY))                   #32

# Determine if we're in the vertical sync pulse
suba(d(1-2*(vBack-1)))          #33
bne(d(lo('vSync0')))            #34 Tests for end of vPulse
adda(d(2*vPulse))               #35
ld(val(syncBits))               #36 Entering vertical back porch
bra(d(lo('vSync2')))            #37
st(d(videoSync0))               #38
label('vSync0')
bne(d(lo('vSync1')))            #36 Tests for start of vPulse
ld(val(syncBits^vSync))         #37
bra(d(lo('vSync3')))            #38 Entering vertical sync pulse
st(d(videoSync0))               #39
label('vSync1')
ldzp(d(videoSync0))             #38 Load current value
label('vSync2')
nop()                           #39
label('vSync3')
xora(d(hSync))                  #40 Precompute, as during the pulse there is no time
st(d(videoSync1))               #41

# Capture the serial input before the '595 shifts it out
# Note: postpone post-processing until back at scan line 0
ldzp(d(videoY));                C('Capture serial input')#42
xora(val(1-2*(vBack-1-1)))      #43 Exactly when the 74HC595 has captured all 8 controller bits
bne(d(lo('.ser0')))             #44
bra(d(lo('.ser1')))             #45
st(d(serialRaw),busIN)          #46
label('.ser0')
nop()                           #46
label('.ser1')

# Update [xout] with the next sound sample every 4 scan lines.
# Keep doing this on 'videoC equivalent' scan lines in vertical blank.
ldzp(d(videoY))                 #47
anda(d(6))                      #48
bne(d(lo('vBlankNormal')))      #49
ldzp(d(sample))                 #50
label('vBlankSample')
ora(d(0x0f));                   C('New sound sample is ready')#51
anda(d(xoutMask),busRAM|ea0DregAC)#52
st(d(xout))                     #53
st(val(sample), ea0DregAC|busD); C('Reset for next sample')#54

runVcpu(199-55, 'line1-39 typeC')#55 Appplication cycles (scan line 1-43 with sample update)
bra(d(lo('sound1')))            #199
ld(d(videoSync0), busRAM|regOUT);C('<New scan line start>')#0 Ends the vertical blank pulse at the right cycle

label('vBlankNormal')
runVcpu(199-51, 'line1-39 typeABD')#51 Application cycles (scan line 1-43 without sample update)
bra(d(lo('sound1')))            #199
ld(d(videoSync0), busRAM|regOUT);C('<New scan line start>')#0 Ends the vertical blank pulse at the right cycle

# Last blank line before transfering to visible area
label('vBlankLast')

# pChange = pNew & ~pOld
# nChange = nNew | ~nOld {DeMorgan}

# Filter raw serial input captured in last vblank (8 cycles)
ld(val(255));                   C('Filter controller input')#32
xora(d(serialLast),busRAM)      #33
ora(d(serialRaw),busRAM)        #34 Catch button-press events
anda(d(buttonState),busRAM)     #35 Keep active button presses
ora(d(serialRaw),busRAM)        #36 Auto-reset already-released buttons
st(d(buttonState))              #37
ldzp(d(serialRaw))              #38
st(d(serialLast))               #39

# Respond to reset button (11 cycles)
xora(val(~buttonStart));        C('Check for soft reset')#40
bne(d(lo('.restart0')))         #41
ldzp(d(resetTimer))             #42 As long as button pressed
suba(val(1))                    #43 ... count down the timer
st(d(resetTimer))               #44
anda(d(127))                    #45
beq(d(lo('.restart2')))         #46
ld(val((vReset&255)-2))         #47 Start force reset when hitting 0
bra(d(lo('.restart1')))         #48 ... otherwise do nothing yet
bra(d(lo('.restart3')))         #49
label('.restart0')
ld(val(127))                    #43 Restore to ~2 seconds when not pressed
st(d(resetTimer))               #44
wait(49-45)                     #45
bra(d(lo('.restart3')))         #49
label('.restart1')
nop()                           #50
label('.restart2')
st(d(vPC))                      #48 Continue force reset
ld(val(vReset>>8))              #49
st(d(vPC+1))                    #50
label('.restart3')

# --- Switch video mode when (only) select is pressed
ldzp(d(buttonState))            #51
xora(val(~buttonSelect))        #52
beq(d(lo('.select0')))          #53
bra(d(lo('.select1')))          #54
ld(val(0))                      #55
label('.select0')
ld(val(lo('videoD')^lo('videoF')))#55
label('.select1')
xora(d(videoDorF),busRAM)       #56
st(d(videoDorF))                #57
ldzp(d(buttonState))            #58
ora(val(buttonSelect))          #59
st(d(buttonState))              #60

runVcpu(196-61, 'line40')       #61 Application cycles (scan line 40)
# vAC==0 now
st(d(videoY))                   #196
st(d(frameX))                   #197
st(d(nextVideo))                #198 videoA=0
ldzp(d(channel))                #199 Advance to next sound channel
anda(val(3));                   C('<New scan line start>')#0
adda(val(1))                    #1
ld(d(hi('sound2')), busD|ea0DregY)#2
jmpy(d(lo('sound2')))           #3
ld(val(syncBits^hSync), regOUT) #4 Start horizontal pulse

#-----------------------------------------------------------------------
#
#  ROM page 3: Application interpreter primary page
#
#-----------------------------------------------------------------------

# Enter the timing-aware application interpreter (aka virtual CPU, vCPU)
#
# This routine will execute as many as possible instructions in the
# alotted time. When time runs out, it synchronizes such that the total
# duration matches the caller's request. Durations are counted in `ticks',
# which are multiples of 2 clock cycles.
#
# Synopsis: Use the runVcpu() macro as entry point

# We let 'ENTER' begin one word before the page boundary, for a bit extra
# precious space in the packed interpreter code page. Although ENTER's
# first instruction is bra() which normally doesn't cross page boundaries,
# in this case it will still jump into the right space, because branches
# from $xxFF land in the next page anyway.
while pc()&255 < 255:
  nop()
label('ENTER')
bra(d(lo('.next2')))            #0 Enter at '.next2' (so no startup overhead)
C('vCPU interpreter')

# --- Page boundary ---

align(0x100,0x100)
ld(d(vPC+1),busRAM|regY)        #1

# Fetch next instruction and execute it, but only if there are sufficient
# ticks left for the slowest instruction.
label('NEXT')
adda(d(vTicks),busRAM);         C('Track elapsed ticks')#0 Actually counting down (AC<0)
blt(d(lo('EXIT')));             C('Escape near time out')#1
label('.next2')
st(d(vTicks))                   #2
ldzp(d(vPC));                   C('Advance vPC')#3
adda(val(2))                    #4
st(d(vPC),busAC|ea0DregX)       #5
ld(busRAM|eaYXregAC);           C('Fetch opcode')#6 Fetch opcode (actually a branch target)
st(eaYXregOUTIX);               #7 Just X++
bra(busAC);                     C('Dispatch')#8
ld(busRAM|eaYXregAC);           C('Prefetch operand')#9

# Resync with caller and return
label('EXIT')
adda(val(maxTicks))             #3
bgt(d(pc()));                   C('Resync')#4
suba(val(1))                    #5
if fastRunVcpu:
  ld(val(2),regY)               #6
else:
  ld(d(vReturn+1),busRAM|regY)  #6
jmpy(d(vReturn+0)|busRAM);      C('Return to caller')#7
ld(val(0))                      #8 AC should be 0 already. Still..
assert vOverheadInt ==          9

# Instruction LDWI: Load immediate constant (AC=$DDDD), 20 cycles
label('LDWI')
st(d(vAC))                      #10
st(eaYXregOUTIX)                #11 Just to increment X
ld(busRAM|eaYXregAC)            #12 Fetch second operand
st(d(vAC+1))                    #13
ldzp(d(vPC))                    #14 Advance vPC one more
adda(val(1))                    #15
st(d(vPC))                      #16
ld(val(-20/2))                  #17
bra(d(lo('NEXT')))              #18
#nop()                          #(19)
#
# Instruction LD: Load from zero page (AC=[D]), 18 cycles
label('LD')
ld(busAC,regX)                  #10,19 (overlap with LDWI)
ldzp(busRAM|ea0XregAC)          #11
st(d(vAC))                      #12
ld(val(0))                      #13
st(d(vAC+1))                    #14
ld(val(-18/2))                  #15
bra(d(lo('NEXT')))              #16
#nop()                          #(17)
#
# Instruction LDW: Word load from zero page (AC=[D],[D+1]), 20 cycles
label('LDW')
ld(busAC,regX)                  #10,17 (overlap with LD)
adda(val(1))                    #11
st(d(vTmp))                     #12 Address of high byte
ld(busRAM|ea0XregAC)            #13
st(d(vAC))                      #14
ld(d(vTmp),busRAM|regX)         #15
ld(busRAM|ea0XregAC)            #16
st(d(vAC+1))                    #17
bra(d(lo('NEXT')))              #18
ld(val(-20/2))                  #19
#nop()                          #(20)
#
# Instruction STW: Word load from zero page (AC=[D],[D+1]), 20 cycles
label('STW')
ld(busAC,regX)                  #10,20 (overlap with LDW)
adda(val(1))                    #11
st(d(vTmp))                     #12 Address of high byte
ldzp(d(vAC))                    #13
st(ea0XregAC)                   #14
ld(d(vTmp),busRAM|regX)         #15
ldzp(d(vAC+1))                  #16
st(ea0XregAC)                   #17
bra(d(lo('NEXT')))              #18
ld(val(-20/2))                  #19

# Instruction BCC: Test AC sign and branch conditionally, 28 cycles
label('BCC')
ldzp(d(vAC+1))                  #10 First inspect high byte ACH
bne(d(lo('.cond2')))            #11
st(d(vTmp))                     #12
ldzp(d(vAC))                    #13 Additionally inspect low byte ACL
beq(d(lo('.cond3')))            #14
ld(val(1))                      #15
st(d(vTmp))                     #16
ld(busRAM|eaYXregAC)            #17 Operand is the conditional
label('.cond1')
bra(busAC)                      #18
ldzp(d(vTmp))                   #19

# Conditional EQ: Branch if zero (if(ALC==0)PCL=D)
label('EQ')
bne(d(lo('.cond4')))            #20
label('.cond2')
beq(d(lo('.cond5')));           C('AC=0 in EQ, AC!=0 from BCC...')#21,13 (overlap with BCC)
ld(busRAM|eaYXregAC)            #22,14 (overlap with BCC)
#
# (continue BCC)
#label('.cond2')
#nop()                          #13
#nop()                          #14
nop()                           #15
label('.cond3')
bra(d(lo('.cond1')))            #16
ld(busRAM|eaYXregAC)            #17 Operand is the conditional
label('.cond4')
ldzp(d(vPC));                   C('False condition')#22
bra(d(lo('.cond6')))            #23
adda(val(1))                    #24
label('.cond5')
st(eaYXregOUTIX);               C('True condition')#23 Just X++
ld(busRAM|eaYXregAC)            #24
label('.cond6')
st(d(vPC))                      #25
bra(d(lo('NEXT')))              #26
ld(val(-28/2))                  #27

# Conditional GT: Branch if positive (if(ALC>0)PCL=D)
label('GT')
ble(d(lo('.cond4')))            #20
bgt(d(lo('.cond5')))            #21
ld(busRAM|eaYXregAC)            #22

# Conditional LT: Branch if negative (if(ALC<0)PCL=D), 16 cycles
label('LT')
bge(d(lo('.cond4')))            #20
blt(d(lo('.cond5')))            #21
ld(busRAM|eaYXregAC)            #22

# Conditional GE: Branch if positive or zero (if(ALC>=0)PCL=D)
label('GE')
blt(d(lo('.cond4')))            #20
bge(d(lo('.cond5')))            #21
ld(busRAM|eaYXregAC)            #22

# Conditional LE: Branch if negative or zero (if(ALC<=0)PCL=D)
label('LE')
bgt(d(lo('.cond4')))            #20
ble(d(lo('.cond5')))            #21
ld(busRAM|eaYXregAC)            #22

# Instruction LDI: Load immediate constant (AC=$DD), 16 cycles
label('LDI')
st(d(vAC))                      #10
ld(val(0))                      #11
st(d(vAC+1))                    #12
ld(val(-16/2))                  #13
bra(d(lo('NEXT')))              #14
#nop()                          #(15)
#
# Instruction ST: Store in zero page ([D]=ACL), 16 cycles
label('ST')
ld(busAC,regX)                  #10,15 (overlap with LDI)
ldzp(d(vAC))                    #11
st(d(vAC),busAC|ea0XregAC)      #12
ld(val(-16/2))                  #13
bra(d(lo('NEXT')))              #14
#nop()                          #(15)
#
# Instruction POP: (LR=[SP++]), 26 cycles
label('POP')
ld(d(vSP),busRAM|regX)          #10,15 (overlap with ST)
ld(busRAM,ea0XregAC)            #11
st(d(vLR))                      #12
ldzp(d(vSP))                    #13
adda(val(1),regX)               #14
ld(busRAM,ea0XregAC)            #15
st(d(vLR+1))                    #16
ldzp(d(vSP))                    #17
adda(val(2))                    #18
st(d(vSP))                      #19
label('next1')
ldzp(d(vPC))                    #20
suba(val(1))                    #21
st(d(vPC))                      #22
ld(val(-26/2))                  #23
bra(d(lo('NEXT')))              #24
#nop()                          #(25)
#
# Conditional NE: Branch if not zero (if(ALC!=0)PCL=D)
label('NE')
beq(d(lo('.cond4')))            #20,25 (overlap with POP)
bne(d(lo('.cond5')))            #21
ld(busRAM|eaYXregAC)            #22

# Instruction PUSH: ([--SP]=LR), 26 cycles
label('PUSH')
ldzp(d(vSP))                    #10
suba(d(1),regX)                 #11
ldzp(d(vLR+1))                  #12
st(ea0XregAC)                   #13
ldzp(d(vSP))                    #14
suba(val(2))                    #15
st(d(vSP),busAC|regX)           #16
ldzp(d(vLR))                    #17
bra(d(lo('next1')))             #18
st(ea0XregAC)                   #19

# Instruction LUP: ROM lookup (AC=ROM[AC+256*D]), 26 cycles
label('LUP')
ld(d(vAC+1),busRAM|regY)        #10
jmpy(d(251));                   C('Trampoline offset')#11
adda(d(vAC),busRAM)             #12

# Instruction ANDI: Logical-AND with constant (AC&=D), 16 cycles
label('ANDI')
anda(d(vAC),busRAM)             #10
st(d(vAC))                      #11
ld(val(0))                      #12 Clear high byte
st(d(vAC+1))                    #13
bra(d(lo('NEXT')))              #14
ld(val(-16/2))                  #15

# Instruction ORI: Logical-OR with constant (AC|=D), 14 cycles
label('ORI')
ora(d(vAC),busRAM)              #10
st(d(vAC))                      #11
bra(d(lo('NEXT')))              #12
ld(val(-14/2))                  #13

# Instruction XORI: Logical-XOR with constant (AC^=D), 14 cycles
label('XORI')
xora(d(vAC),busRAM)             #10
st(d(vAC))                      #11
bra(d(lo('NEXT')))              #12
ld(val(-14/2))                  #13

# Instruction BRA: Branch unconditionally (PCL=D), 14 cycles
label('BRA')
st(d(vPC))                      #10
ld(val(-14/2))                  #11
bra(d(lo('NEXT')))              #12
#nop()                          #(13)
#
# Instruction INC: Increment zero page byte ([D]++), 16 cycles
label('INC')
ld(busAC,regX)                  #10,13 (overlap with BRA)
ld(busRAM,ea0XregAC)            #11
adda(val(1))                    #12
st(ea0XregAC)                   #13
bra(d(lo('NEXT')))              #14
ld(val(-16/2))                  #15

# Instruction ADDW: Word addition with zero page (AC+=[D]+256*[D+1]), 28 cycles
label('ADDW')
# The non-carry paths could be 26 cycles at the expense of (much) more code.
# But a smaller size is better so more instructions fit in this code page.
# 28 cycles is still 4.5 usec. The 6502 equivalent takes 20 cycles or 20 usec.
ld(busAC,regX)                  #10 Address of low byte to be added
adda(val(1))                    #11
st(d(vTmp))                     #12 Address of high byte to be added
ldzp(d(vAC))                    #13 Add the low bytes
adda(busRAM|ea0XregAC)          #14
st(d(vAC))                      #15 Store low result
bmi(d(lo('.addw0')))            #16 Now figure out if there was a carry
suba(busRAM|ea0XregAC)          #17 Gets back the initial value of vAC
bra(d(lo('.addw1')))            #18
ora(busRAM|ea0XregAC)           #19 Bit 7 is our lost carry
label('.addw0')
anda(busRAM|ea0XregAC)          #18 Bit 7 is our lost carry
nop()                           #19
label('.addw1')
anda(val(0x80),regX)            #20 Move the carry to bit 0 (0 or +1)
ld(busRAM,ea0XregAC)            #21
adda(d(vAC+1),busRAM)           #22 Add the high bytes with carry
ld(d(vTmp),busRAM|regX)         #23
adda(busRAM|ea0XregAC)          #24
st(d(vAC+1))                    #25 Store high result
bra(d(lo('NEXT')))              #26
ld(val(-28/2))                  #27

# Instruction PEEK: (AC=[AC]), 26 cycles
label('PEEK')
ld(val(hi('peek')),regY)        #10
jmpy(d(lo('peek')))             #11
#ldzp(d(vPC))                   #12
#
# Instruction SYS: Native call, <=256 cycles (<=128 ticks, in reality less)
#
# The 'SYS' vCPU instruction first checks the number of desired ticks given by
# the operand. As long as there are insufficient ticks available in the current
# time slice, the instruction will be retried. This will effectively wait for
# the next scan line if the current slice is almost out of time. Then a jump to
# native code is made. This code can do whatever it wants, but it must return
# to the 'REENTER' label when done. When returning, AC must hold (the negative
# of) the actual consumed number of whole ticks for the entire virtual
# instruction cycle (from NEXT to NEXT). This duration may not exceed the prior
# declared duration in the operand + 28 (or maxTicks). The operand specifies the
# (negative) of the maximum number of *extra* ticks that the native call will
# need. The GCL compiler automatically makes this calculation from gross number
# of cycles to excess number of ticks.
# SYS functions can modify vPC to implement repetition. For example to split
# up work into multiple chucks.
label('retry')
ldzp(d(vPC));                   C('Retry until sufficient time')#13,12 (overlap with PEEK)
suba(val(2))                    #14
st(d(vPC))                      #15
bra(d(lo('REENTER')))           #16
ld(val(-20/2))                  #17
label('SYS')
adda(d(vTicks),busRAM)          #10
blt(d(lo('retry')))             #11
ld(d(sysFn+1),busRAM|regY)      #12
jmpy(d(sysFn)|busRAM)           #13
#nop()                          #(14)
#
# Instruction SUBW: Word subtract with zero page (AC-=[D]+256*[D+1]), 28 cycles
# All cases can be done in 26 cycles, but the code will become much larger
label('SUBW')
ld(busAC,regX)                  #10,14 (overlap with SYS) Address of low byte to be subtracted
adda(val(1))                    #11
st(d(vTmp))                     #12 Address of high byte to be subtracted
ldzp(d(vAC))                    #13
bmi(d(lo('.subw0')))            #14
suba(busRAM|ea0XregAC)          #15
st(d(vAC))                      #16 Store low result
bra(d(lo('.subw1')))            #17
ora(busRAM|ea0XregAC)           #18 Bit 7 is our lost carry
label('.subw0')
st(d(vAC))                      #16 Store low result
anda(busRAM|ea0XregAC)          #17 Bit 7 is our lost carry
nop()                           #18
label('.subw1')
anda(val(0x80),regX)            #19 Move the carry to bit 0
ldzp(d(vAC+1))                  #20
suba(busRAM,ea0XregAC)          #21
ld(d(vTmp),busRAM|regX)         #22
suba(busRAM|ea0XregAC)          #23
st(d(vAC+1))                    #24
ld(val(-28/2))                  #25
label('REENTER')
bra(d(lo('NEXT')));             C('Return from SYS calls')#26
ld(d(vPC+1),busRAM|regY)        #27

# Instruction DEF: Define data or code (AC,PCL=PC+2,D), 18 cycles
label('DEF')
ld(val(hi('def')),regY)         #10
jmpy(d(lo('def')))              #11
#st(d(vTmp))                    #12
#
# Instruction CALL: (LR=PC+2,PC=[D]-2), 26 cycles
label('CALL')
st(d(vTmp))                     #10,12 (overlap with DEF)
ldzp(d(vPC))                    #11
adda(val(2));                   C('Point to instruction after CALL')#12
st(d(vLR))                      #13
ldzp(d(vPC+1))                  #14
st(d(vLR+1))                    #15
ld(d(vTmp),busRAM|regX)         #16
ld(busRAM|ea0XregAC)            #17
suba(val(2));                   C('Because NEXT will add 2')#18
st(d(vPC))                      #19
ldzp(d(vTmp))                   #20
adda(val(1),regX)               #21
ld(busRAM|ea0XregAC)            #22
st(d(vPC+1),busAC|regY)         #23
bra(d(lo('NEXT')))              #24
ld(val(-26/2))                  #25

# ALLOCA implementation
# Instruction ALLOCA: (SP+=D), 14 cycles
label('ALLOC')
adda(d(vSP),busRAM)             #10
st(d(vSP))                      #11
bra(d(lo('NEXT')))              #12
ld(val(-14/2))                  #13

# The instructions below are all implemented in the second code page. Jumping
# back and forth makes each 6 cycles slower, but it also saves space in the
# primary page for the instructions above. Most of them are in fact not very
# critical, as evidenced by the fact that they weren't needed for the first
# Gigatron applications (Snake, Racer, Mandelbrot, Loader). By providing them
# in this way, at least they don't need to be implemented as a SYS extension.

# Instruction ADDI: Add small positive constant (AC+=D), 28 cycles
label('ADDI')
ld(val(hi('addi')),regY)        #10
jmpy(d(lo('addi')))             #11
st(d(vTmp))                     #12

# Instruction SUBI: Subtract small positive constant (AC+=D), 28 cycles
label('SUBI')
ld(val(hi('subi')),regY)        #10
jmpy(d(lo('subi')))             #11
st(d(vTmp))                     #12

# Instruction LSLW: Logical shift left (AC<<=1), 28 cycles
# Useful, because ADDW can't add vAC to itself. Also more compact.
label('LSLW')
ld(val(hi('lslw')),regY)        #10
jmpy(d(lo('lslw')))             #11
ldzp(d(vAC))                    #12

# Instruction STLW: Store on stack (), 26 cycles
label('STLW')
ld(val(hi('stlw')),regY)        #10
jmpy(d(lo('stlw')))             #11
#nop()                          #12
#
# Instruction LDLW: Load from stack (), 26 cycles
label('LDLW')
ld(val(hi('ldlw')),regY)        #10,12 (overlap with STLW)
jmpy(d(lo('ldlw')))             #11
#nop()                          #12
#
# Instruction POKE: ([[D+1],[D]]=ACL), 28 cycles
label('POKE')
ld(val(hi('poke')),regY)        #10,12 (overlap with LDLW)
jmpy(d(lo('poke')))             #11
st(d(vTmp))                     #12

# Instruction POKEW: (), 28 cycles
label('POKEW')
ld(val(hi('pokew')),regY)       #10
jmpy(d(lo('pokew')))            #11
st(d(vTmp))                     #12

# Instruction PEEKW: (), 28 cycles
label('PEEKW')
ld(val(hi('peekw')),regY)       #10
jmpy(d(lo('peekw')))            #11
#nop()                          #12
#
# Instruction ANDW: (AC&=[D]+256*[D+1]), 28 cycles
label('ANDW')
ld(val(hi('andw')),regY)        #10,12 (overlap with PEEKW)
jmpy(d(lo('andw')))             #11
#nop()                          #12
#
# Instruction ORW: (AC|=[D]+256*[D+1]), 28 cycles
label('ORW')
ld(val(hi('orw')),regY)         #10,12 (overlap with ANDW)
jmpy(d(lo('orw')))              #11
#nop()                          #12
#
# Instruction XORW: (AC^=[D]+256*[D+1]), 26 cycles
label('XORW')
ld(val(hi('xorw')),regY)        #10,12 (overlap with ORW)
jmpy(d(lo('xorw')))             #11
st(d(vTmp))                     #12
# We keep XORW 2 cycles faster than ANDW/ORW, because that
# can be useful for comparing numbers for equality a tiny
# bit faster than with SUBW

# Instruction RET: Function return (PC=LR-2), 16 cycles
label('RET')
ldzp(d(vLR))                    #10
assert(pc()&255 == 0)

#-----------------------------------------------------------------------
#
#  ROM page 4: Application interpreter extension
#
#-----------------------------------------------------------------------
align(0x100, 0x100)

# (Continue RET)
suba(val(2))                    #11
st(d(vPC))                      #12
ldzp(d(vLR+1))                  #13
st(d(vPC+1))                    #14
ld(val(hi('REENTER')),regY)     #15
jmpy(d(lo('REENTER')))          #16
ld(val(-20/2))                  #17

# DEF implementation
label('def')
ldzp(d(vPC))                    #13
adda(val(2))                    #14
st(d(vAC))                      #15
ldzp(d(vPC+1))                  #16
st(d(vAC+1))                    #17
ldzp(d(vTmp))                   #18
st(d(vPC))                      #19
ld(val(hi('REENTER')),regY)     #20
ld(val(-26/2))                  #21
jmpy(d(lo('REENTER')))          #22
nop()                           #23

# ADDI implementation
label('addi')
adda(d(vAC),busRAM)             #13
st(d(vAC))                      #14 Store low result
bmi(d(lo('.addi0')))            #15 Now figure out if there was a carry
suba(d(vTmp),busRAM)            #16 Gets back the initial value of vAC
bra(d(lo('.addi1')))            #17
ora(d(vTmp),busRAM)             #18 Bit 7 is our lost carry
label('.addi0')
anda(d(vTmp),busRAM)            #17 Bit 7 is our lost carry
nop()                           #18
label('.addi1')
anda(val(0x80),regX)            #19 Move the carry to bit 0 (0 or +1)
ld(busRAM,ea0XregAC)            #20
adda(d(vAC+1),busRAM)           #21 Add the high bytes with carry
st(d(vAC+1))                    #22 Store high result
ld(val(hi('REENTER')),regY)     #23
jmpy(d(lo('REENTER')))          #24
ld(val(-28/2))                  #25

# SUBI implementation
label('subi')
ldzp(d(vAC))                    #13
bmi(d(lo('.subi0')))            #14
suba(d(vTmp),busRAM)            #15
st(d(vAC))                      #16 Store low result
bra(d(lo('.subi1')))            #17
ora(d(vTmp),busRAM)             #18 Bit 7 is our lost carry
label('.subi0')
st(d(vAC))                      #16 Store low result
anda(d(vTmp),busRAM)            #17 Bit 7 is our lost carry
nop()                           #18
label('.subi1')
anda(val(0x80),regX)            #19 Move the carry to bit 0
ldzp(d(vAC+1))                  #20
suba(busRAM,ea0XregAC)          #21
st(d(vAC+1))                    #22
ld(val(hi('REENTER')),regY)     #23
jmpy(d(lo('REENTER')))          #24
ld(val(-28/2))                  #25

# LSLW implementation
label('lslw')
anda(d(128),regX)               #13
adda(d(vAC),busRAM)             #14
st(d(vAC))                      #15
ld(ea0XregAC,busRAM)            #16
adda(d(vAC+1),busRAM)           #17
adda(d(vAC+1),busRAM)           #18
st(d(vAC+1))                    #19
ldzp(d(vPC))                    #20
suba(d(1))                      #21
st(d(vPC))                      #22
ld(val(hi('REENTER')),regY)     #23
jmpy(d(lo('REENTER')))          #24
ld(val(-28/2))                  #25

# STLW implementation
label('stlw')
adda(d(vSP),busRAM)             #13
st(d(vTmp))                     #14
adda(d(1),regX)                 #15
ldzp(d(vAC+1))                  #16
st(ea0XregAC)                   #17
ld(d(vTmp),busRAM|regX)         #18
ldzp(d(vAC))                    #19
st(ea0XregAC)                   #20
ld(val(hi('REENTER')),regY)     #21
jmpy(d(lo('REENTER')))          #22
ld(val(-26/2))                  #23

# LDLW implementation
label('ldlw')
adda(d(vSP),busRAM)             #13
st(d(vTmp))                     #14
adda(d(1),regX)                 #15
ld(ea0XregAC,busRAM)            #16
st(d(vAC+1))                    #17
ld(d(vTmp),busRAM|regX)         #18
ld(ea0XregAC,busRAM)            #19
st(d(vAC))                      #20
ld(val(hi('REENTER')),regY)     #21
jmpy(d(lo('REENTER')))          #22
ld(val(-26/2))                  #23

# POKE implementation
label('poke')
adda(d(1),regX)                 #13
ld(busRAM,ea0XregAC)            #14
ld(busAC,regY)                  #15
ld(d(vTmp),busRAM|regX)         #16
ld(busRAM,ea0XregAC)            #17
ld(busAC,regX)                  #18
ldzp(d(vAC))                    #19
st(eaYXregAC)                   #20
ld(val(hi('REENTER')),regY)     #21
jmpy(d(lo('REENTER')))          #22
ld(val(-26/2))                  #23

# PEEK implementation
label('peek')
suba(val(1))                    #13
st(d(vPC))                      #14
ld(d(vAC),busRAM|regX)          #15
ld(d(vAC+1),busRAM|regY)        #16
ld(busRAM|eaYXregAC)            #17
st(d(vAC))                      #18
label('lupReturn')              #Nice coincidence that lupReturn can be here
ld(val(0))                      #19
st(d(vAC+1))                    #20
ld(val(hi('REENTER')),regY)     #21
jmpy(d(lo('REENTER')))          #22
ld(val(-26/2))                  #23
#
# POKEW implementation
label('pokew')
adda(d(1),regX)                 #13,25 (overlap with peek)
ld(busRAM,ea0XregAC)            #14
ld(busAC,regY)                  #15
ld(d(vTmp),busRAM|regX)         #16
ld(busRAM,ea0XregAC)            #17
ld(busAC,regX)                  #18
ldzp(d(vAC))                    #19
st(eaYXregOUTIX)                #20
ldzp(d(vAC+1))                  #21
st(eaYXregAC)                   #22
ld(val(hi('REENTER')),regY)     #23
jmpy(d(lo('REENTER')))          #24
ld(val(-28/2))                  #25

# PEEKW implementation
label('peekw')
ldzp(d(vPC))                    #13
suba(val(1))                    #14
st(d(vPC))                      #15
ld(d(vAC),busRAM|regX)          #16
ld(d(vAC+1),busRAM|regY)        #17
ld(busRAM|eaYXregAC)            #18
st(eaYXregOUTIX)                #19
st(d(vAC))                      #20
ld(busRAM|eaYXregAC)            #21
st(d(vAC+1))                    #22
ld(val(hi('REENTER')),regY)     #23
jmpy(d(lo('REENTER')))          #24
ld(val(-28/2))                  #25

# ANDW implementation
label('andw')
st(d(vTmp))                     #13
adda(d(1),regX)                 #14
ld(busRAM|ea0XregAC)            #15
anda(d(vAC+1),busRAM)           #16
st(d(vAC+1))                    #17
ld(d(vTmp),busRAM|regX)         #18
ld(busRAM|ea0XregAC)            #19
anda(d(vAC),busRAM)             #20
st(d(vAC))                      #21
ld(val(-28/2))                  #22
ld(val(hi('REENTER')),regY)     #23
jmpy(d(lo('REENTER')))          #24
#nop()                          #(25)

# ORW implementation
label('orw')
st(d(vTmp))                     #13,25 (overlap with andw)
adda(d(1),regX)                 #14
ld(busRAM|ea0XregAC)            #15
ora(d(vAC+1),busRAM)            #16
st(d(vAC+1))                    #17
ld(d(vTmp),busRAM|regX)         #18
ld(busRAM|ea0XregAC)            #19
ora(d(vAC),busRAM)              #20
st(d(vAC))                      #21
ld(val(-28/2))                  #22
ld(val(hi('REENTER')),regY)     #23
jmpy(d(lo('REENTER')))          #24
#nop()                          #(25)

# XORW implementation
label('xorw')
adda(d(1),regX)                 #13,25 (overlap with orw)
ld(busRAM|ea0XregAC)            #14
xora(d(vAC+1),busRAM)           #15
st(d(vAC+1))                    #16
ld(d(vTmp),busRAM|regX)         #17
ld(busRAM|ea0XregAC)            #18
xora(d(vAC),busRAM)             #19
st(d(vAC))                      #20
ld(val(hi('REENTER')),regY)     #21
jmpy(d(lo('REENTER')))          #22
ld(val(-26/2))                  #23

#-----------------------------------------------------------------------
#
#  vCPU extension functions (for acceleration and compaction) follow below.
#
#  The naming convention is: SYS_<CamelCase>_<N>
#
#  With <N> the maximum number of cycles the function will run
#  (counted from NEXT to NEXT). This is the same number that must
#  be passed to the 'SYS' vCPU instruction as operand, and it will
#  appear in the GCL code upon use.
#
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Extension SYS_NextByteIn_32
#-----------------------------------------------------------------------

# sysArgs[0:1] Current address
# sysArgs[2]   Checksum
# sysArgs[3]   Wait value (videoY)

label('SYS_NextByteIn_32')
ldzp(d(videoY))                 #15
xora(d(sysArgs+3),busRAM)       #16
bne(d(lo('.sysNbi')))           #17
ld(d(sysArgs+0),busRAM|regX)    #18
ld(d(sysArgs+1),busRAM|regY)    #19
ld(busIN)                       #20
st(eaYXregAC)                   #21
adda(d(sysArgs+2),busRAM)       #22
st(d(sysArgs+2))                #23
ldzp(d(sysArgs+0))              #24
adda(d(1))                      #25
st(d(sysArgs+0))                #26
ld(val(hi('REENTER')),regY)     #27
jmpy(d(lo('REENTER')))          #28
ld(val(-32/2))                  #29
# Restart instruction
label('.sysNbi')
ldzp(d(vPC))                    #19
suba(d(2))                      #20
st(d(vPC))                      #21
ld(val(-28/2))                  #22
ld(val(hi('REENTER')),regY)     #23
jmpy(d(lo('REENTER')))          #24
nop()                           #25

#-----------------------------------------------------------------------
# Extension SYS_PayloadCopy_34
#-----------------------------------------------------------------------

# sysArgs[0:1] Source address
# sysArgs[4]   Copy count
# sysArgs[5:6] Destination address

label('SYS_PayloadCopy_34')
ldzp(d(sysArgs+4))              #15 Copy count
beq(d(lo('.sysCc0')))           #16
suba(d(1))                      #17
st(d(sysArgs+4))                #18
ld(d(sysArgs+0),busRAM|regX)    #19 Current pointer
ld(d(sysArgs+1),busRAM|regY)    #20
ld(eaYXregAC,busRAM)            #21
ld(d(sysArgs+5),busRAM|regX)    #22 Target pointer
ld(d(sysArgs+6),busRAM|regY)    #23
st(eaYXregAC)                   #24
ldzp(d(sysArgs+5))              #25 Increment target
adda(d(1))                      #26
st(d(sysArgs+5))                #27
bra(d(lo('.sysCc1')))           #28
label('.sysCc0')
ld(val(hi('REENTER')),regY)     #18,29
wait(30-19)                     #19
label('.sysCc1')
jmpy(d(lo('REENTER')))          #30
ld(d(-34/2))                    #31

#-----------------------------------------------------------------------
# Extension SYS_Draw4_30:
#-----------------------------------------------------------------------

# sysArgs[0:3]  Pixels
# sysArgs[4:5]  Position on screen

label('SYS_Draw4_30')
ld(d(sysArgs+4),busRAM|regX)    #15
ld(d(sysArgs+5),busRAM|regY)    #16
ldzp(d(sysArgs+0))              #17
st(eaYXregOUTIX)                #18
ldzp(d(sysArgs+1))              #19
st(eaYXregOUTIX)                #20
ldzp(d(sysArgs+2))              #21
st(eaYXregOUTIX)                #22
ldzp(d(sysArgs+3))              #23
st(eaYXregOUTIX)                #24
ld(val(hi('REENTER')),regY)     #25
jmpy(d(lo('REENTER')))          #26
ld(val(-30/2))                  #27

#-----------------------------------------------------------------------
# Extension SYS_VDrawBits_134:
#-----------------------------------------------------------------------

# Draw slice of a character
# sysArgs[0]    Color 0 (background)
# sysArgs[1]    Color 1 (pen)
# sysArgs[2]    8 bits, highest bit first (destructive)
# sysArgs[4:5]  Position on screen

label('SYS_VDrawBits_134')
ld(d(sysArgs+4),busRAM|regX)    #15
ld(val(0))                      #16
label('.vdb0')
st(d(vTmp))                     #17+i*14
adda(d(sysArgs+5),busRAM|regY)  #18+i*14 Y=[sysPos+1]+vTmp
ldzp(d(sysArgs+2))              #19+i*14 Select color
bmi(d(lo('.vdb1')))             #20+i*14
bra(d(lo('.vdb2')))             #21+i*14
ldzp(d(sysArgs+0))              #22+i*14
label('.vdb1')
ldzp(d(sysArgs+1))              #22+i*14
label('.vdb2')
st(eaYXregAC)                   #23+i*14 Draw pixel
ldzp(d(sysArgs+2))              #24+i*14 Shift byte left
adda(busAC)                     #25+i*14
st(d(sysArgs+2))                #26+i*14
ldzp(d(vTmp))                   #27+i*14 Loop counter
suba(val(7))                    #28+i*14
bne(d(lo('.vdb0')))             #29+i*14
adda(val(8))                    #30+i*14
ld(val(hi('REENTER')),regY)     #129
jmpy(d(lo('REENTER')))          #130
ld(val(-134/2))                 #131

label('SYS_LSRW8_24')
ldzp(d(vAC+1))                  #15
st(d(vAC))                      #16
ld(d(0))                        #17
st(d(vAC+1))                    #18
ld(d(hi('REENTER')),regY)       #19
jmpy(d(lo('REENTER')))          #20
ld(d(-24/2))                    #21

label('SYS_LSLW8_24')
ldzp(d(vAC))                    #15
st(d(vAC+1))                    #16
ld(d(0))                        #17
st(d(vAC))                      #18
ld(d(hi('REENTER')),regY)       #19
jmpy(d(lo('REENTER')))          #20
ld(d(-24/2))                    #21

#-----------------------------------------------------------------------
#  ROM page 5-6: Shift table and code
#-----------------------------------------------------------------------

# Lookup table for i>>n, with n in 1..6
# Indexing ix = i & ~b | (b-1), where b = 1<<(n-1)
#       ...
#       lda  <.ret
#       st   [vTmp]
#       ld   >shiftTable,y
#       <calculate ix>
#       jmp  y,ac
#       bra  $ff
# .ret: ...
#
# i >> 7 can be always be done with RAM: [i&128]
#       ...
#       anda $80,x
#       ld   [x]
#       ...

align(0x100, 0x200)

label('shiftTable')
shiftTable = pc()
for ix in range(255):
  for n in range(1,7): # Find first zero
    if ~ix & (1 << (n-1)):
      break
  pattern = ['x' if i<n else '1' if ix&(1<<i) else '0' for i in range(8)]
  ld(val(ix>>n)); C('0b%s >> %d' % (''.join(reversed(pattern)), n))

assert(pc()&255 == 255)
bra(d(vTmp)|busRAM); C('Jumps back into next page')

label('SYS_LSRW1_48')
assert(pc()&255 == 0)#First instruction on this page must be a nop
nop()                           #15
ld(d(hi('shiftTable')),regY);   C('Logical shift right 1 bit (X >> 1)')#16
ld(d(lo('.sysLsrw1a')));        C('Shift low byte')#17
st(d(vTmp))                     #18
ldzp(d(vAC))                    #19
anda(d(0b11111110))             #20
jmpy(busAC)                     #21
bra(d(255));                    C('Actually: bra $%04x' % (shiftTable+255))#22
label('.sysLsrw1a')
st(d(vAC))                      #26
ldzp(d(vAC+1));                 C('Transfer bit 8')#27
anda(d(1))                      #28
adda(d(127))                    #29
anda(d(128))                    #30
ora(d(vAC)|busRAM)              #31
st(d(vAC))                      #32
ld(d(lo('.sysLsrw1b')));        C('Shift high byte')#33
st(d(vTmp))                     #34
ldzp(d(vAC+1))                  #35
anda(d(0b11111110))             #36
jmpy(busAC)                     #37
bra(d(255));                    C('Actually: bra $%04x' % (shiftTable+255))#38
label('.sysLsrw1b')
st(d(vAC+1))                    #42
ld(d(hi('REENTER')),regY)       #43
jmpy(d(lo('REENTER')))          #44
ld(d(-48/2))                    #45

label('SYS_LSRW2_52')
ld(d(hi('shiftTable')),regY);   C('Logical shift right 2 bit (X >> 2)')#15
ld(d(lo('.sysLsrw2a')));        C('Shift low byte')#16
st(d(vTmp))                     #17
ldzp(d(vAC))                    #18
anda(d(0b11111100))             #19
ora( d(0b00000001))             #20
jmpy(busAC)                     #21
bra(d(255));                    C('Actually: bra $%04x' % (shiftTable+255))#22
label('.sysLsrw2a')
st(d(vAC))                      #26
ldzp(d(vAC+1));                 C('Transfer bit 8:9')#27
adda(busAC)                     #28
adda(busAC)                     #29
adda(busAC)                     #30
adda(busAC)                     #31
adda(busAC)                     #32
adda(busAC)                     #33
ora(d(vAC)|busRAM)              #34
st(d(vAC))                      #35
ld(d(lo('.sysLsrw2b')));        C('Shift high byte')#36
st(d(vTmp))                     #37
ldzp(d(vAC+1))                  #38
anda(d(0b11111100))             #39
ora( d(0b00000001))             #40
jmpy(busAC)                     #41
bra(d(255));                    C('Actually: bra $%04x' % (shiftTable+255))#42
label('.sysLsrw2b')
st(d(vAC+1))                    #46
ld(d(hi('REENTER')),regY)       #47
jmpy(d(lo('REENTER')))          #48
ld(d(-52/2))                    #49

label('SYS_LSRW3_52')
ld(d(hi('shiftTable')),regY);   C('Logical shift right 3 bit (X >> 3)')#15
ld(d(lo('.sysLsrw3a')));        C('Shift low byte')#16
st(d(vTmp))                     #17
ldzp(d(vAC))                    #18
anda(d(0b11111000))             #19
ora( d(0b00000011))             #20
jmpy(busAC)                     #21
bra(d(255));                    C('Actually: bra $%04x' % (shiftTable+255))#22
label('.sysLsrw3a')
st(d(vAC))                      #26
ldzp(d(vAC+1));                 C('Transfer bit 8:10')#27
adda(busAC)                     #28
adda(busAC)                     #29
adda(busAC)                     #30
adda(busAC)                     #31
adda(busAC)                     #32
ora(d(vAC)|busRAM)              #33
st(d(vAC))                      #34
ld(d(lo('.sysLsrw3b')));        C('Shift high byte')#35
st(d(vTmp))                     #36
ldzp(d(vAC+1))                  #37
anda(d(0b11111000))             #38
ora( d(0b00000011))             #39
jmpy(busAC)                     #40
bra(d(255));                    C('Actually: bra $%04x' % (shiftTable+255))#41
label('.sysLsrw3b')
st(d(vAC+1))                    #45
ld(d(-52/2))                    #46
ld(d(hi('REENTER')),regY)       #47
jmpy(d(lo('REENTER')))          #48
#nop()                          #49

label('SYS_LSRW4_50')
ld(d(hi('shiftTable')),regY);   C('Logical shift right 4 bit (X >> 4)')#15,49
ld(d(lo('.sysLsrw4a')));        C('Shift low byte')#16
st(d(vTmp))                     #17
ldzp(d(vAC))                    #18
anda(d(0b11110000))             #19
ora( d(0b00000111))             #20
jmpy(busAC)                     #21
bra(d(255));                    C('Actually: bra $%04x' % (shiftTable+255))#22
label('.sysLsrw4a')
st(d(vAC))                      #26
ldzp(d(vAC+1));                 C('Transfer bit 8:11')#27
adda(busAC)                     #28
adda(busAC)                     #29
adda(busAC)                     #30
adda(busAC)                     #31
ora(d(vAC)|busRAM)              #32
st(d(vAC))                      #33
ld(d(lo('.sysLsrw4b')));        C('Shift high byte')#34
st(d(vTmp))                     #35
ldzp(d(vAC+1))                  #36
anda(d(0b11110000))             #37
ora( d(0b00000111))             #38
jmpy(busAC)                     #39
bra(d(255));                    C('Actually: bra $%04x' % (shiftTable+255))#40
label('.sysLsrw4b')
st(d(vAC+1))                    #44
ld(d(hi('REENTER')),regY)       #45
jmpy(d(lo('REENTER')))          #46
ld(d(-50/2))                    #47

label('SYS_LSRW5_50')
ld(d(hi('shiftTable')),regY);   C('Logical shift right 5 bit (X >> 5)')#15
ld(d(lo('.sysLsrw5a')));        C('Shift low byte')#16
st(d(vTmp))                     #17
ldzp(d(vAC))                    #18
anda(d(0b11100000))             #19
ora( d(0b00001111))             #20
jmpy(busAC)                     #21
bra(d(255));                    C('Actually: bra $%04x' % (shiftTable+255))#22
label('.sysLsrw5a')
st(d(vAC))                      #26
ldzp(d(vAC+1));             C('Transfer bit 8:13')#27
adda(busAC)                     #28
adda(busAC)                     #29
adda(busAC)                     #30
ora(d(vAC)|busRAM)              #31
st(d(vAC))                      #32
ld(d(lo('.sysLsrw5b')));        C('Shift high byte')#33
st(d(vTmp))                     #34
ldzp(d(vAC+1))                  #35
anda(d(0b11100000))             #36
ora( d(0b00001111))             #37
jmpy(busAC)                     #38
bra(d(255));                    C('Actually: bra $%04x' % (shiftTable+255))#39
label('.sysLsrw5b')
st(d(vAC+1))                    #44
ld(d(-50/2))                    #45
ld(d(hi('REENTER')),regY)       #46
jmpy(d(lo('REENTER')))          #47
#nop()                          #48

label('SYS_LSRW6_48')
ld(d(hi('shiftTable')),regY);   C('Logical shift right 6 bit (X >> 6)')#15,44
ld(d(lo('.sysLsrw6a')));        C('Shift low byte')#16
st(d(vTmp))                     #17
ldzp(d(vAC))                    #18
anda(d(0b11000000))             #19
ora( d(0b00011111))             #20
jmpy(busAC)                     #21
bra(d(255));                    C('Actually: bra $%04x' % (shiftTable+255))#22
label('.sysLsrw6a')
st(d(vAC))                      #26
ldzp(d(vAC+1));                 C('Transfer bit 8:13')#27
adda(busAC)                     #28
adda(busAC)                     #29
ora(d(vAC)|busRAM)              #30
st(d(vAC))                      #31
ld(d(lo('.sysLsrw6b')));        C('Shift high byte')#32
st(d(vTmp))                     #33
ldzp(d(vAC+1))                  #34
anda(d(0b11000000))             #35
ora( d(0b00011111))             #36
jmpy(busAC)                     #37
bra(d(255));                    C('Actually: bra $%04x' % (shiftTable+255))#38
label('.sysLsrw6b')
st(d(vAC+1))                    #42
ld(d(hi('REENTER')),regY)       #43
jmpy(d(lo('REENTER')))          #44
ld(d(-48/2))                    #45

label('SYS_LSLW4_46')
ld(d(hi('shiftTable')),regY);   C('Logical shift left 4 bit (X << 4)')#15
ld(d(lo('.sysLsrl4')))          #16
st(d(vTmp))                     #17
ldzp(d(vAC+1))                  #18
adda(busAC)                     #19
adda(busAC)                     #20
adda(busAC)                     #21
adda(busAC)                     #22
st(d(vAC+1))                    #23
ldzp(d(vAC))                    #24
anda(d(0b11110000))             #25
ora( d(0b00000111))             #26
jmpy(busAC)                     #27
bra(d(255));                    C('Actually: bra $%04x' % (shiftTable+255))#28
label('.sysLsrl4')
ora(d(vAC+1),busRAM)            #32
st(d(vAC+1))                    #33
ldzp(d(vAC))                    #34
adda(busAC)                     #35
adda(busAC)                     #36
adda(busAC)                     #37
adda(busAC)                     #38
st(d(vAC))                      #39
ld(d(-46/2))                    #40
ld(d(hi('REENTER')),regY)       #41
jmpy(d(lo('REENTER')))          #42
#nop()                          #43

#-----------------------------------------------------------------------
# Extension SYS_Read3_40: Read 3 consecutive bytes from ROM
#-----------------------------------------------------------------------

# sysArgs[0:2]  Bytes (output)
# sysArgs[6:7]  ROM pointer (input)

label('SYS_Read3_40')
ld(d(sysArgs+7),busRAM|regY)    #15,32
jmpy(d(128-7))                  #16 trampoline3a
ldzp(d(sysArgs+6))              #17
label('txReturn')
st(d(sysArgs+2))                #34
ld(val(hi('REENTER')),regY)     #35
jmpy(d(lo('REENTER')))          #36
ld(val(-40/2))                  #37

def trampoline3a():
  """Read 3 bytes from ROM page"""
  while pc()&255 < 128-7:
    nop()
  bra(busAC)                    #18
  C('Trampoline for page $%02x00 reading (entry)' % (pc()>>8))
  bra(d(123))                   #19
  st(d(sysArgs+0))              #21
  ldzp(d(sysArgs+6))            #22
  adda(val(1))                  #23
  bra(busAC)                    #24
  bra(d(250))                   #25 trampoline3b

def trampoline3b():
  """Read 3 bytes from ROM page (continue)"""
  while pc()&255 < 256-6:
    nop()
  st(d(sysArgs+1))              #27
  C('Trampoline for page $%02x00 reading (continue)' % (pc()>>8))
  ldzp(d(sysArgs+6))            #28
  adda(val(2))                  #29
  ld(d(hi('txReturn')),regY)    #30
  bra(busAC)                    #31
  jmpy(d(lo('txReturn')))       #32

#-----------------------------------------------------------------------
# Extension SYS_Unpack_56: Unpack 3 bytes into 4 pixels
#-----------------------------------------------------------------------

# sysArgs[0:2]  Packed bytes (input)
# sysArgs[0:3]  Pixels (output)

label('SYS_Unpack_56')
ld(val(audioTable>>8),regY)     #15
ldzp(d(sysArgs+2))              #16 a[2]>>2
anda(val(0xfc),regX)            #17
ld(eaYXregAC|busRAM)            #18
st(d(sysArgs+3));               C('-> Pixel 3')#19

ldzp(d(sysArgs+2))              #20 (a[2]&3)<<4
anda(val(0x03))                 #21
adda(busAC)                     #22
adda(busAC)                     #23
adda(busAC)                     #24
adda(busAC)                     #25
st(d(sysArgs+2));               #26

ldzp(d(sysArgs+1))              #27 | a[1]>>4
anda(val(0xfc),regX)            #28
ld(eaYXregAC|busRAM)            #29
anda(val(0xfc),regX)            #30
ld(eaYXregAC|busRAM)            #31
ora(d(sysArgs+2),busRAM)        #32
st(d(sysArgs+2));               C('-> Pixel 2')#33

ldzp(d(sysArgs+1))              #34 (a[1]&15)<<2
anda(val(0x0f))                 #35
adda(busAC)                     #36
adda(busAC)                     #37
st(d(sysArgs+1))                #38

ldzp(d(sysArgs+0))              #39 | a[0]>>6
anda(val(0xfc),regX)            #40
ld(eaYXregAC|busRAM)            #41
anda(val(0xfc),regX)            #42
ld(eaYXregAC|busRAM)            #43
anda(val(0xfc),regX)            #44
ld(eaYXregAC|busRAM)            #45
ora(d(sysArgs+1),busRAM)        #46
st(d(sysArgs+1));               C('-> Pixel 1')#47

ldzp(d(sysArgs+0))              #48 a[1]&63
anda(val(0x3f))                 #49
st(d(sysArgs+0));               C('-> Pixel 0')#50

ld(val(hi('REENTER')),regY)     #51
jmpy(d(lo('REENTER')))          #52
ld(val(-56/2))                  #53

#-----------------------------------------------------------------------
#
#  ROM page 7-8: Gigatron font data
#
#-----------------------------------------------------------------------

align(0x100, 0x100)

label('font32up')
for ch in range(32, 32+50):
  comment = 'Char %s' % repr(chr(ch))
  for byte in font.font[ch-32]:
    ld(val(byte))
    comment = C(comment)

trampoline()

#-----------------------------------------------------------------------

align(0x100, 0x100)

label('font82up')
for ch in range(32+50, 128):
  comment = 'Char %s' % repr(chr(ch))
  for byte in font.font[ch-32]:
    ld(val(byte))
    comment = C(comment)

trampoline()

#-----------------------------------------------------------------------
#
#  ROM page 9: Key table for music
#
#-----------------------------------------------------------------------

align(0x100, 0x100)
notes = 'CCDDEFFGGAAB'
sampleRate = cpuClock / 200.0 / 4
label('notesTable')
for i in range(0, 250, 2):
  j = i/2-1
  freq = 440.0*2.0**((j-57)/12.0)
  if j>=0 and freq <= sampleRate/2.0:
    key = int(round(32768 * freq / sampleRate))
    octave, note = j/12, notes[j%12]
    sharp = '-' if notes[j%12-1] != note else '#'
    comment = '%s%s%s (%0.1f Hz)' % (note, sharp, octave, freq)
  else:
    key, comment = 0, None
  ld(val(key&127)); C(comment)
  ld(val(key>>7))
trampoline()

#-----------------------------------------------------------------------
#
#  ROM page 10: Inversion table
#
#-----------------------------------------------------------------------

align(0x100, 0x100)
label('invTable')

# Unit 64, table offset 16 (=1/4), value offset 1: (x+16)*(y+1) == 64*64 - e
for i in range(251):
  ld(val(4096/(i+16)-1))

trampoline()

#-----------------------------------------------------------------------
#  ROM page 11: Built-in full resolution images
#-----------------------------------------------------------------------

f = open('Images/gigatron.rgb', 'rb')
raw = f.read()
f.close()
align(0x100)
label('gigatronRaw')
for i in xrange(len(raw)):
  if i&255 < 251:
    ld(val(ord(raw[i])))
  elif i&255 == 251:
    trampoline()

def importImage(rgbName, width, height, ref):
  f = open(rgbName)
  raw = f.read()
  f.close()
  align(0x100)
  label(ref)
  for y in range(0, height, 2):
    for j in range(2):
      align(0x80)
      comment = 'Pixels for %s line %s' % (ref, y+j)
      for x in range(0, width, 4):
        bytes = []
        for i in range(4):
          R = ord(raw[3 * ((y + j) * width + x + i) + 0])
          G = ord(raw[3 * ((y + j) * width + x + i) + 1])
          B = ord(raw[3 * ((y + j) * width + x + i) + 2])
          bytes.append( (R/85) + 4*(G/85) + 16*(B/85) )

        # Pack 4 pixels in 3 bytes
        ld(val( ((bytes[0]&0b111111)>>0) + ((bytes[1]&0b000011)<<6) )); comment = C(comment)
        ld(val( ((bytes[1]&0b111100)>>2) + ((bytes[2]&0b001111)<<4) ))
        ld(val( ((bytes[2]&0b110000)>>4) + ((bytes[3]&0b111111)<<2) ))
      if j==0:
        trampoline3a()
      else:
        trampoline3b()

importImage('Images/Parrot-160x120.rgb',  160, 120, 'packedParrot')
importImage('Images/Baboon-160x120.rgb',  160, 120, 'packedBaboon')
importImage('Images/Jupiter-160x120.rgb', 160, 120, 'packedJupiter')

#-----------------------------------------------------------------------
#  Application specific SYS extensions
#-----------------------------------------------------------------------

label('SYS_RacerUpdateVideoX_40')
ld(d(sysArgs+2),busRAM|regX)    #15 q,
ld(d(sysArgs+3),busRAM|regY)    #16
ld(eaYXregAC,busRAM)            #17
st(d(vTmp))                     #18
suba(d(sysArgs+4),busRAM)       #19 X-
ld(d(sysArgs+0),busRAM|regX)    #20 p.
ld(d(sysArgs+1),busRAM|regY)    #21
st(eaYXregAC,busAC)             #22
ld(d(sysArgs+0),busRAM)         #23 p 4- p=
suba(d(4))                      #24
st(d(sysArgs+0))                #25
ldzp(d(vTmp))                   #26 q,
st(d(sysArgs+4))                #27 X=
ld(d(sysArgs+2),busRAM)         #28 q<++
adda(d(1))                      #29
st(d(sysArgs+2))                #30
bne(d(lo('.sysRacer0')))        #31 Self-repeat by adjusting vPC
ldzp(d(vPC))                    #32
bra(d(lo('.sysRacer1')))        #33
nop()                           #34
label('.sysRacer0')
suba(d(2))                      #33
st(d(vPC))                      #34
label('.sysRacer1')
ld(val(hi('REENTER')),regY)     #35
jmpy(d(lo('REENTER')))          #36
ld(val(-40/2))                  #37

label('SYS_RacerUpdateVideoY_40')
ldzp(d(sysArgs+3))              #15 8&
anda(d(8))                      #16
bne(d(lo('.sysRacer2')))        #17 [if<>0 1]
bra(d(lo('.sysRacer3')))        #18
ld(d(0))                        #19
label('.sysRacer2')
ld(d(1))                        #19
label('.sysRacer3')
st(d(vTmp))                     #20 tmp=
ld(d(sysArgs+1),busRAM|regY)    #21
ld(d(sysArgs+0),busRAM)         #22 p<++ p<++
adda(d(2))                      #23
st(d(sysArgs+0),busAC|regX)     #24
xora(d(238))                    #25 238^
st(d(vAC))                      #26
st(d(vAC+1))                    #27
ldzp(d(sysArgs+2))              #28 SegmentY
anda(d(254))                    #29 254&
adda(d(vTmp),busRAM)            #30 tmp+
st(eaYXregAC,busAC)             #31
ldzp(d(sysArgs+2))              #32 SegmentY<++
adda(d(1))                      #33
st(d(sysArgs+2))                #34
ld(val(hi('REENTER')),regY)     #35
jmpy(d(lo('REENTER')))          #36
ld(val(-40/2))                  #37

#-----------------------------------------------------------------------
# Extension SYS_LoaderProcessInput_48
#-----------------------------------------------------------------------

# sysArgs[0:1] Source address
# sysArgs[2]   Checksum
# sysArgs[4]   Copy count
# sysArgs[5:6] Destination address

label('SYS_LoaderProcessInput_48')
ld(d(sysArgs+1),busRAM|regY)    #15
ldzp(d(sysArgs+2))              #16
bne(d(lo('.sysPi0')))           #17
ld(d(sysArgs+0),busRAM)         #18
suba(d(65),regX)                #19 Point at first byte of buffer
ld(eaYXregAC,busRAM)            #20 Command byte
st(eaYXregOUTIX)                #21 X++
xora(d(ord('L')))               #22 This loader lumps everything under 'L'
bne(d(lo('.sysPi1')))           #23
ld(eaYXregAC,busRAM);           C('Valid command')#24 Length byte
st(eaYXregOUTIX)                #25 X++
anda(d(63))                     #26 Bit 6:7 are garbage
st(d(sysArgs+4))                #27 Copy count
ld(eaYXregAC,busRAM)            #28 Low copy address
st(eaYXregOUTIX)                #29 X++
st(d(sysArgs+5))                #30
ld(eaYXregAC,busRAM)            #31 High copy address
st(eaYXregOUTIX)                #32 X++
st(d(sysArgs+6))                #33
ldzp(d(sysArgs+4))              #34
bne(d(lo('.sysPi2')))           #35
# Execute code (don't care about checksum anymore)
ldzp(d(sysArgs+5));             C('Execute')#36 Low run address
suba(d(2))                      #37
st(d(vPC))                      #38
st(d(vLR))                      #39
ldzp(d(sysArgs+6))              #40 High run address
st(d(vPC+1))                    #41
st(d(vLR+1))                    #42
ld(val(hi('REENTER')),regY)     #43
jmpy(d(lo('REENTER')))          #44
ld(d(-48/2))                    #45
# Invalid checksum
label('.sysPi0')
wait(25-19);                    C('Invalid checksum')#19 Reset checksum
# Unknown command
label('.sysPi1')
ld(d(ord('g')));                C('Unknown command')#25 Reset checksum
st(d(sysArgs+2))                #26
ld(val(hi('REENTER')),regY)     #27
jmpy(d(lo('REENTER')))          #28
ld(d(-32/2))                    #29
# Loading data
label('.sysPi2')
ld(d(sysArgs+0),busRAM);        C('Loading data')#37 Continue checksum
suba(d(1),regX)                 #38 Point at last byte
ld(eaYXregAC,busRAM)            #39
st(d(sysArgs+2))                #40
ld(val(hi('REENTER')),regY)     #41
jmpy(d(lo('REENTER')))          #42
ld(d(-46/2))                    #43

#-----------------------------------------------------------------------
#
#  ROM page XX: Skyline for Racer
#
#-----------------------------------------------------------------------

f = open('Images/RacerHorizon-256x16.rgb', 'rb')
raw = f.read()
f.close()

packed, quartet = [], []
for i in xrange(0, len(raw), 3):
  R, G, B = ord(raw[i+0]), ord(raw[i+1]), ord(raw[i+2])
  quartet.append((R/85) + 4*(G/85) + 16*(B/85))
  if len(quartet) == 4:
    # Pack 4 pixels in 3 bytes
    packed.append( ((quartet[0]&0b111111)>>0) + ((quartet[1]&0b000011)<<6) )
    packed.append( ((quartet[1]&0b111100)>>2) + ((quartet[2]&0b001111)<<4) )
    packed.append( ((quartet[2]&0b110000)>>4) + ((quartet[3]&0b111111)<<2) )
    quartet = []

label('zippedRacerHorizon')
for i in xrange(len(packed)):
  ld(val(packed[i]))
  if pc()&255 == 251:
    trampoline()

#-----------------------------------------------------------------------
#
#  ROM page XX: Bootstrap vCPU
#
#-----------------------------------------------------------------------

# For info
print 'SYS limits low %s high %s' % (repr(minSYS), repr(maxSYS))

# Export some zero page variables to GCL
# XXX Solve in another way (not through symbol table!)
define('memSize',    memSize)
define('entropy',    entropy)
define('frameCount', frameCount)
define('serialRaw',  serialRaw)
define('buttonState', buttonState)
define('sysFn',      sysFn)
for i in range(8):
  define('sysArgs%d' % i, sysArgs+i)
define('soundTimer', soundTimer)
define('vAC',        vAC)
define('vACH',       vAC+1)
define('vLR',        vLR)
# All these are hacks:
define('videoY',     videoY)
define('nextVideo',  nextVideo)
define('vPC+1',      vPC+1) # XXX trampoline() is probably in the wrong module

# Compile test GCL program

for gclSource in argv[1:]:
  name = gclSource.rsplit('.', 1)[0]
  print
  print 'Compile file %s label %s ROM %04x' % (gclSource, name, pc())
  label(name)
  program = gcl.Program(vCpuStart, name)
  zpReset(zpFree)
  for line in open(gclSource).readlines():
    program.line(line)
  program.end()
print

if pc()&255:
  trampoline()

#-----------------------------------------------------------------------
# Finish assembly
#-----------------------------------------------------------------------
end()
