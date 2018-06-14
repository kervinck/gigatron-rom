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
#  - Builtin vCPU programs (Snake, Racer, etc)
#  - Serial input handler
#  - Soft reset button (keep 'Start' button down for 2 seconds)
#
#-----------------------------------------------------------------------

from sys import argv
from os  import getenv

from asm import *
import gcl0x as gcl
import font

# Pre-loading the formal interface as a way to get warnings when
# accidently redefined with a different value
loadBindings('interface.json')

# Gigatron clock
cpuClock = 6.250e+06

# Output pin assignment for VGA
R, G, B, hSync, vSync = 1, 4, 16, 64, 128
syncBits = hSync+vSync # Both pulses negative

# When the XOUT register is in the circuit, the rising edge triggers its update.
# The loop can therefore not be agnostic to the horizontal pulse polarity.
assert syncBits & hSync != 0

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
# page 2, for 2 cycles less interpreter ENTER/EXIT overhead. Only if there are
# multiple video loops in the system that all use vCPU, this must be False.
fastRunVcpu = True

#-----------------------------------------------------------------------
#
#  RAM page 0: zero-page variables
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
userVars        = zpByte(0)

#-----------------------------------------------------------------------
#
#  RAM page 1: video line table
#
#-----------------------------------------------------------------------

# Byte 0-239 define the video lines
videoTable = 0x0100 # Indirection table: Y[0] dX[0]  ..., Y[119] dX[119]

# Highest bytes are for channel 1 variables

# Sound synthesis  ch1   ch2   ch3   ch4
wavA = 250
wavX = 251
keyL = 252
keyH = 253
oscL = 254
oscH = 255

#-----------------------------------------------------------------------
#  Memory layout
#-----------------------------------------------------------------------

userCode = 0x0200       # Application vCPU code
soundTable = 0x0700     # Wave form tables (doubles as right-shift-2 table)
screenMemory = 0x0800   # Default start of screen memory: 0x0800 to 0x7fff

#-----------------------------------------------------------------------
#  Application definitions
#-----------------------------------------------------------------------

maxTicks = 28/2 # Duration of vCPU's slowest virtual opcode

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

  n /= 2
  returnPc = pc() + (5 if fastRunVcpu else 7)
  ld(returnPc&255)              #0
  comment = C(comment)
  st([vReturn])                 #1
  if fastRunVcpu:
    # In this mode [vReturn+1] will not be used
    assert returnPc>>8 == 2
  else:
    # Allow interpreter to be called from anywhere
    ld(returnPc>>8)             #2
    st([vReturn+1])             #3
  ld(hi('ENTER'), Y)            #4
  jmpy('ENTER')                 #5
  ld(n)                         #6

#-----------------------------------------------------------------------
#
#  ROM page 0: Boot
#
#-----------------------------------------------------------------------

align(0x100, 0x100)

# Give a first sign of life that can be checked with a voltmeter
ld(0b0000);                     C('LEDs |OOOO|')
ld(syncBits^hSync, OUT)         # Prepare XOUT update, hSync goes down, RGB to black
ld(syncBits, OUT)               # hSync goes up, updating XOUT

# Simple RAM test and size check by writing to [1<<n] and see if [0] changes.
ld(1);                          C('RAM test and count')
label('.countMem0')
st([memSize], Y)
ld(255)
xora([Y,0])
st([Y,0])                       # Test if we can change and read back ok
st([0])                         # Preserve (inverted) memory value in [0]
xora([Y,0])
bne(pc())                       # Just hang here on apparent RAM failure
ld(255)
xora([Y,0])
st([Y,0])
xora([0])
beq('.countMem1')               # Wrapped and [0] changed as well
ld([memSize])
bra('.countMem0')
adda(AC)
label('.countMem1')

# Momentarily wait to allow for debouncing of the reset switch by spinning
# roughly 2^15 times at 2 clocks per loop: 6.5ms@10MHz to 10ms@6.3MHz
# Real-world switches normally bounce shorter than that.
# "[...] 16 switches exhibited an average 1557 usec of bouncing, with,
#  as I said, a max of 6200 usec" (From: http://www.ganssle.com/debouncing.htm)
# Relevant for the breadboard version, as the kit doesn't have a reset switch.

ld(255);                        C('Debounce reset button')
label('.debounce')
st([0])
bne(pc())
suba(1)
ld([0])
bne('.debounce')
suba(1)

# Update LEDs (memory is present and counted, reset is stable)
ld(0b0001);                     C('LEDs |*OOO|')
ld(syncBits^hSync, OUT)
ld(syncBits, OUT)

# Scan the entire RAM space to collect entropy for a random number generator.
# The 16-bit address space is scanned, even if less RAM was detected.
ld(0);                          C('Collect entropy from RAM')
st([vAC+0], X)
st([vAC+1], Y)
label('.initEnt0')
ld([entropy+0])
bpl('.initEnt1')
adda([Y,X])
xora(191)
label('.initEnt1')
st([entropy+0])
ld([entropy+1])
bpl('.initEnt2')
adda([entropy+0])
xora(193)
label('.initEnt2')
st([entropy+1])
adda([entropy+2])
st([entropy+2])
ld([vAC+0])
adda(1)
bne('.initEnt0')
st([vAC+0], X)
ld([vAC+1])
adda(1)
bne('.initEnt0')
st([vAC+1], Y)

# Update LEDs
ld(0b0011);                     C('LEDs |**OO|')
ld(syncBits^hSync, OUT)
ld(syncBits, OUT)

# Determine if this is a cold or a warm start. We do this by checking the
# boot counter and comparing it to a simplistic checksum. The assumption
# is that after a cold start the checksum is invalid.

ld([bootCount]);                C('Cold or warm boot?')
adda([bootCheck])
adda(0x5a)
bne('cold')
ld(0)
label('warm')
ld([bootCount])                 # if warm start: bootCount += 1
adda(1)
label('cold')
st([bootCount])                 # if cold start: bootCount = 0
xora(255)
suba(0x5a-1)
st([bootCheck])

# vCPU reset handler
vReset = videoTable + 240 # we have 10 unused bytes behind the video table
ld((vReset&255)-2);             C('Setup vCPU reset handler')
st([vPC])
adda(2, X)
ld(vReset>>8)
st([vPC+1], Y)
st('LDI',             [Y,Xpp])
st('SYS_Reset_36',    [Y,Xpp])
st('STW',             [Y,Xpp])
st(sysFn,             [Y,Xpp])
st('SYS',             [Y,Xpp])
st(256-36/2+maxTicks, [Y,Xpp])
st('SYS',             [Y,Xpp])  # SYS_Exec_88
st(256-88/2+maxTicks, [Y,Xpp])

ld(255);                        C('Setup serial input')
st([frameCount])
st([serialRaw])
st([serialLast])
st([buttonState])
st([resetTimer])

ld(0b0111);                     C('LEDs |***O|')
ld(syncBits^hSync, OUT)
ld(syncBits, OUT)

# XXX Everything below should at one point migrate to Reset.gcl

# Init sound tables
ld(soundTable>>8, Y);           C('Setup sound tables')
ld(0)
st([channel])
ld(0, X)
label('.loop0')
st([vTmp]);                     C('Noise: T[4x+0] = x (permutate below)')
st([Y,Xpp])
anda(0x20);                     C('Triangle: T[4x+1] = 2x if x<32 else 127-2x')
bne('.initTri0')
ld([vTmp])
bra('.initTri1')
label('.initTri0')
adda([vTmp])
xora(127)
label('.initTri1')
st([Y,Xpp])
ld([vTmp]);                     C('Pulse: T[4x+2] = 0 if x<32 else 63')
anda(0x20)
beq('.initPul')
ld(0)
ld(63)
label('.initPul')
st([Y,Xpp])
ld([vTmp]);                     C('Sawtooth: T[4x+3] = x')
st([Y,Xpp])
adda(1)
xora(0x40)
bne('.loop0')
xora(0x40)

ld(0);                          C('Permutate noise table T[4i]')
st([vAC+0]);                    C('x')
st([vAC+1]);                    C('4y')
label('.loop1')
ld([vAC+1], X);                 C('tmp = T[4y]')
ld([Y,X])
st([vTmp])
ld([vAC+0]);                    C('T[4y] = T[4x]')
adda(AC)
adda(AC, X)
ld([Y,X])
ld([vAC+1], X)
st([Y,X])
adda(AC);                       C('y += T[4x]')
adda(AC)
adda([vAC+1])
st([vAC+1])
ld([vAC+0]);                    C('T[x] = tmp')
adda(AC)
adda(AC, X)
ld([vTmp])
st([Y,X])
ld([vAC+0]);                    C('while(++x)')
adda(1)
bne('.loop1')
st([vAC+0])

# Init LED sequencer
ld(120);                        C('Setup LED sequencer')
st([ledTimer])
ld(60/6)
st([ledTempo])
ld(0)
st([ledState])

ld(0b1111);                     C('LEDs |****|')
ld(syncBits^hSync, OUT)
ld(syncBits, OUT)
st([xout])                      # Setup for control by video loop
st([xoutMask])

ld(hi('vBlankStart'), Y);       C('Enter video loop')
jmpy('vBlankStart')
ld(syncBits)

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
assert pc()>>8 == 0
romTypeValue = getenv('romType')
romTypeValue = int(romTypeValue, base=0) if romTypeValue else 0
ld(romTypeValue);               C('Set ROM type/version')#15
st([romType])                   #16
ld(0)                           #17
st([vSP])                       #18 Reset stack pointer
assert userCode&255 == 0
st([vLR])                       #19
st([soundTimer])                #20
ld(userCode>>8)                #21
st([vLR+1])                     #22
ld('videoF')                    #23 Do this before first visible pixels
st([videoDorF])                 #24
ld('SYS_Exec_88')               #25
st([sysFn])                     #26 High byte (remains) 0
ld('Reset')                     #27
st([sysArgs+0])                 #28
ld(hi('Reset'))                 #29
st([sysArgs+1])                 #30
# Return to interpreter
ld(hi('REENTER'), Y)            #31
jmpy('REENTER')                 #32
ld(-36/2)                       #33

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
assert pc()>>8 == 0
ld(0)                           #15 Address of loader on zero page
st([vPC+1], Y)                  #16
ld([vSP])                       #17 Below the current stack pointer
suba(53+2)                      #18 (AC -> *+0)
st([vTmp], X)                   #19
adda(-2)                        #20 (AC -> *-2)
st([vPC])                       #21
# Start of manually compiled vCPU section
st('PUSH',    [Y,Xpp])          #22 *+0
st('BRA',     [Y,Xpp])          #23 *+1
adda(26)                        #24 (AC -> *+24)
st(           [Y,Xpp])          #25 *+2
st('ST',      [Y,Xpp])          #26 *+3 Chunk copy loop
st(sysArgs+3, [Y,Xpp])          #27 *+4 High-address came first
st('CALL',    [Y,Xpp])          #28 *+5
adda(33-24)                     #29 (AC -> *+33)
st(           [Y,Xpp])          #30 *+6
st('ST',      [Y,Xpp])          #31 *+7
st(sysArgs+2, [Y,Xpp])          #32 *+8 Then the low address
st('CALL',    [Y,Xpp])          #33 *+9
st(           [Y,Xpp])          #34 *+10
st('ST',      [Y,Xpp])          #35 *+11 Byte copy loop
st(sysArgs+4, [Y,Xpp])          #36 *+12 Byte count (0 means 256)
st('CALL',    [Y,Xpp])          #37 *+13
st(           [Y,Xpp])          #38 *+14
st('POKE',    [Y,Xpp])          #39 *+15
st(sysArgs+2, [Y,Xpp])          #40 *+16
st('INC',     [Y,Xpp])          #41 *+17
st(sysArgs+2, [Y,Xpp])          #42 *+18
st('LD',      [Y,Xpp])          #43 *+19
st(sysArgs+4, [Y,Xpp])          #44 *+20
st('SUBI',    [Y,Xpp])          #45 *+21
st(1,         [Y,Xpp])          #46 *+22
st('BCC',     [Y,Xpp])          #47 *+23
st('NE',      [Y,Xpp])          #48 *+24
adda(11-2-33)                   #49 (AC -> *+9)
st(           [Y,Xpp])          #50 *+25
st('CALL',    [Y,Xpp])          #51 *+26 Go to next block
adda(33-9)                      #52 (AC -> *+33)
st(           [Y,Xpp])          #53 *+27
st('BCC',     [Y,Xpp])          #54 *+28
st('NE',      [Y,Xpp])          #55 *+29
adda(3-2-33)                    #56 (AC -> *+1)
st(           [Y,Xpp])          #57 *+30
st('POP',     [Y,Xpp])          #58 *+31 End
st('RET',     [Y,Xpp])          #59 *+32
# Pointer constant pointing to the routine below (for use by CALL)
adda(35-1)                      #60 (AC -> *+35)
st(           [Y,Xpp])          #61 *+33
st(0,         [Y,Xpp])          #62 *+34
# Routine to read next byte from ROM and advance read pointer
st('LD',      [Y,Xpp])          #63 *+35 Test for end of ROM table
st(sysArgs+0, [Y,Xpp])          #64 *+36
st('XORI',    [Y,Xpp])          #65 *+37
st(251,       [Y,Xpp])          #66 *+38
st('BCC',     [Y,Xpp])          #67 *+39
st('NE',      [Y,Xpp])          #68 *+40
adda(46-2-35)                   #69 (AC -> *+44)
st(           [Y,Xpp])          #70 *+41
st('ST',      [Y,Xpp])          #71 *+42 Wrap to next ROM page
st(sysArgs+0, [Y,Xpp])          #72 *+43
st('INC',     [Y,Xpp])          #73 *+44
st(sysArgs+1, [Y,Xpp])          #74 *+45
st('LDW',     [Y,Xpp])          #75 *+46 Read next byte from ROM table
st(sysArgs+0, [Y,Xpp])          #76 *+47
st('LUP',     [Y,Xpp])          #77 *+48
st(0,         [Y,Xpp])          #78 *+49
st('INC',     [Y,Xpp])          #79 *+50 Increment read pointer
st(sysArgs+0, [Y,Xpp])          #80 *+51
st('RET',     [Y,Xpp])          #81 *+52 Return
# Return to interpreter
nop()                           #82
ld(hi('REENTER'), Y)            #83
jmpy('REENTER')                 #84
ld(-88/2)                       #85

#-----------------------------------------------------------------------
# Extension SYS_Out_22: Send byte to output port
#-----------------------------------------------------------------------

label('SYS_Out_22')
ld([sysArgs+0], OUT)            #15
nop()                           #16
ld(hi('REENTER'), Y)            #17
jmpy('REENTER')                 #18
ld(-22/2)                       #19

#-----------------------------------------------------------------------
# Extension SYS_In_24: Read a byte from the input port
#-----------------------------------------------------------------------

label('SYS_In_24')
st(IN, [vAC])                   #15
ld(0)                           #16
st([vAC+1])                     #17
nop()                           #18
ld(hi('REENTER'), Y)            #19
jmpy('REENTER')                 #20
ld(-24/2)                       #21

assert pc()&255 == 0

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
assert lo('videoA') == 0        # videoA starts at the page boundary
ld('videoB')                    #29
st([nextVideo])                 #30
ld(videoTable>>8, Y)            #31
ld([videoY], X)                 #32
ld([Y,X])                       #33
st([Y,Xpp])                     #34 Just to increment X
st([frameY])                    #35
ld([Y,X])                       #36
adda([frameX], X)               #37
ld([frameY], Y)                 #38
ld(syncBits)                    #39

# Stream 160 pixels from memory location <Yi,Xi> onwards
# Superimpose the sync signal bits to be robust against misprogramming
label('pixels')
for i in range(160):
  ora([Y,Xpp], OUT)             #40-199
  if i==0: C('Pixel burst')
ld(syncBits, OUT);              C('<New scan line start>')#0 Back to black

# Front porch
ld([channel]);                  C('Advance to next sound channel')#1
label('soundF')
anda(3)                         #2
adda(1)                         #3
ld(syncBits^hSync, OUT);        C('Start horizontal pulse')#4

# Horizontal sync
label('sound2')
st([channel], Y)                #5
ld(0x7f)                        #6
anda([Y,oscL])                  #7
adda([Y,keyL])                  #8
st([Y,oscL])                    #9
anda(0x80, X)                   #10
ld([X])                         #11
adda([Y,oscH])                  #12
adda([Y,keyH])                  #13
st([Y,oscH] )                   #14
anda(0xfc)                      #15
xora([Y,wavX])                  #16
ld(AC, X)                       #17
ld([Y,wavA])                    #18
ld(soundTable>>8, Y)            #19
adda([Y,X])                     #20
bmi('.sound2a')                 #21
bra('.sound2b')                 #22
anda(63)                        #23
label('.sound2a')
ld(63)                          #23
label('.sound2b')
adda([sample])                  #24
st([sample])                    #25

ld([xout]);                     C('Gets copied to XOUT')#26
bra([nextVideo])                #27
ld(syncBits, OUT);              C('End horizontal pulse')#28

# Back porch B: second of 4 repeated scan lines
# - Recompute Xi from dXi and store for retrieval in the next scan lines
label('videoB')
ld('videoC')                    #29
st([nextVideo])                 #30
ld(videoTable>>8, Y)            #31
ld([videoY])                    #32
adda(1, X)                      #33
ld([frameX])                    #34
adda([Y,X])                     #35
st([frameX], X)                 #36 Undocumented opcode "store in RAM and X"!
ld([frameY], Y)                 #37
bra('pixels')                   #38
ld(syncBits)                    #39

# Back porch C: third of 4 repeated scan lines
# - Nothing new to do, Yi and Xi are known
label('videoC')
ld([sample]);                   C('New sound sample is ready')#29 First something that didn't fit in the audio loop
ora(0x0f)                       #30
anda([xoutMask])                #31
st([xout])                      #32 Update [xout] with new sample (4 channels just updated)
st(sample, [sample]);           C('Reset for next sample')#33 Reset for next sample
ld([videoDorF]);                C('Mode for scan line 4')#34 Now back to video business
st([nextVideo])                 #35
ld([frameX], X)                 #36
ld([frameY], Y)                 #37
bra('pixels')                   #38
ld(syncBits)                    #39

# Back porch D: last of 4 repeated scan lines
# - Calculate the next frame index
# - Decide if this is the last line or not
label('videoD')                 # Default video mode
ld([frameX], X)                 #29
ld([videoY])                    #30
suba((120-1)*2)                 #31
beq('.last')                    #32
ld([frameY], Y)                 #33
adda(120*2)                     #34 More pixel lines to go
st([videoY])                    #35
ld('videoA')                    #36
st([nextVideo])                 #37
bra('pixels')                   #38
ld(syncBits)                    #39
label('.last')
wait(36-34)                     #34 No more pixel lines
ld('videoE')                    #36
st([nextVideo])                 #37
bra('pixels')                   #38
ld(syncBits)                    #39

# Back porch "E": after the last line
# - Go back to program page 0 and enter vertical blank
label('videoE') # Exit visible area
ld(hi('vBlankStart'), Y)        #29
jmpy('vBlankStart')             #30
ld(syncBits)                    #31

# Back porch "F": scan lines and fast mode
label('videoF')                 # Fast video mode
ld([videoY])                    #29
suba((120-1)*2)                 #30
bne('.notlast')                 #31
adda(120*2)                     #32
bra('.join')                    #33
ld('videoE')                    #34 No more visible lines
label('.notlast')
st([videoY])                    #33 More visible lines
ld('videoA')                    #34
label('.join')
st([nextVideo])                 #35
runVcpu(199-36, 'line41-521 typeF')#36 Application (every 4th of scan lines 41-521)
ld(hi('soundF'), Y)             #199 XXX This is on the current page
jmpy('soundF');                 C('<New scan line start>')#0
ld([channel])                   #1 Advance to next sound channel

# Vertical blank part of video loop
label('vBlankStart')            # Start of vertical blank interval
assert pc()&255 < 16            # Assure that we are in the beginning of the next page

st([videoSync0]);               C('Start of vertical blank interval')#32
ld(syncBits^hSync)              #33
st([videoSync1])                #34

# (Re)initialize carry table for robustness
st(0, [0]);                     C('Carry table')#35
ld(1)                           #36
st([0x80])                      #37

# It is nice to set counter before vCPU starts
ld(1-2*(vFront+vPulse+vBack-2)) #38 -2 because first and last are different
st([videoY])                    #39

# Uptime frame count (3 cycles)
ld([frameCount]);               C('Frame counter')#40
adda(1)                         #41
st([frameCount])                #42

# Mix entropy (11 cycles)
xora([entropy+1]);              C('Mix entropy')#43
xora([serialRaw])               #44 Mix in serial input
adda([entropy+0])               #45
st([entropy+0])                 #46
adda([entropy+2])               #47 Some hidden state
st([entropy+2])                 #48
bmi('.rnd0')                    #49
bra('.rnd1')                    #50
xora(64+16+2+1)                 #51
label('.rnd0')
xora(64+32+8+4)                 #51
label('.rnd1')
adda([entropy+1])               #52
st([entropy+1])                 #53

# LED sequencer (19 cycles)
ld([ledTimer]);                 C('Blinkenlight sequencer')#54
bne('.leds4')                   #55
ld('.leds0')                    #56
adda([ledState])                #57
bra(AC)                         #58
bra('.leds1')                   #59
label('.leds0')
ld(0b1111);C('LEDs |****|')     #60
ld(0b0111);C('LEDs |***O|')     #60
ld(0b0011);C('LEDs |**OO|')     #60
ld(0b0001);C('LEDs |*OOO|')     #60
ld(0b0010);C('LEDs |O*OO|')     #60
ld(0b0100);C('LEDs |OO*O|')     #60
ld(0b1000);C('LEDs |OOO*|')     #60
ld(0b0100);C('LEDs |OO*O|')     #60
ld(0b0010);C('LEDs |O*OO|')     #60
ld(0b0001);C('LEDs |*OOO|')     #60
ld(0b0011);C('LEDs |**OO|')     #60
ld(0b0111);C('LEDs |***O|')     #60
ld(0b1111);C('LEDs |****|')     #60
ld(0b1110);C('LEDs |O***|')     #60
ld(0b1100);C('LEDs |OO**|')     #60
ld(0b1000);C('LEDs |OOO*|')     #60
ld(0b0100);C('LEDs |OO*O|')     #60
ld(0b0010);C('LEDs |O*OO|')     #60
ld(0b0001);C('LEDs |*OOO|')     #60
ld(0b0010);C('LEDs |O*OO|')     #60
ld(0b0100);C('LEDs |OO*O|')     #60
ld(0b1000);C('LEDs |OOO*|')     #60
ld(0b1100);C('LEDs |OO**|')     #60
ld(0b1110+128)                  #60
C('LEDs |O***|')
label('.leds1')
st([xoutMask])                  #61 Temporarily park new state here
bmi('.leds2')                   #62
bra('.leds3')                   #63
ld([ledState])                  #64
label('.leds2')
ld(-1)                          #64
label('.leds3')
adda(1)                         #65
st([ledState])                  #66
bra('.leds5')                   #67
ld([ledTempo])                  #68 Setup the LED timer for the next period
label('.leds4')
wait(67-57)                     #57
ld([ledTimer])                  #67
suba(1)                         #68
label('.leds5')
st([ledTimer])                  #69
ld([xoutMask])                  #70 Low 4 bits are the LED output
anda(0b00001111)                #71 High bits will be restored below
st([xoutMask])                  #72

# When the total number of scan lines per frame is not an exact multiple of the
# (4) channels, there will be an audible discontinuity if no measure is taken.
# This static noise can be suppressed by swallowing the first `lines mod 4'
# partial samples after transitioning into vertical blank. This is easiest if
# the modulo is 0 (do nothing) or 1 (reset sample while in the first blank scan
# line). For the two other cases there is no solution yet: give a warning.
soundDiscontinuity = (vFront+vPulse+vBack) % 4
extra = 0
if soundDiscontinuity == 1:
  st(sample, [sample])          # XXX We're swallowing _2_ samples here!
  C('Sound continuity')
  extra += 1
if soundDiscontinuity > 1:
  print "Warning: sound discontinuity not supressed"

runVcpu(189-73-extra, 'line0')  #73 Application cycles (scan line 0)

# Sound on/off (6 cycles)
ld([soundTimer]);               C('Sound on/off')#189
bne('.snd0')                    #190
bra('.snd1')                    #191
ld(0)                           #192 Sound off
label('.snd0')
ld(0xf0)                        #192 Sound on
label('.snd1')
ora([xoutMask])                 #193
st([xoutMask])                  #194

# Sound timer count down (5 cycles)
ld([soundTimer]);               C('Sound timer')#195
beq('.snd2')                    #196
bra('.snd3')                    #197
suba(1)                         #198
label('.snd2')
ld(0)                           #198
label('.snd3')
st([soundTimer])                #199

ld([videoSync0], OUT);          C('<New scan line start>')#0

label('sound1')
ld([channel]);                  C('Advance to next sound channel')#1
anda(3)                         #2
adda(1)                         #3
ld([videoSync1], OUT);          C('Start horizontal pulse')#4
st([channel], Y)                #5
ld(0x7f);                       C('Update sound channel')#6
anda([Y,oscL])                  #7
adda([Y,keyL])                  #8
st([Y,oscL])                    #9
anda(0x80, X)                   #10
ld([X])                         #11
adda([Y,oscH])                  #12
adda([Y,keyH])                  #13
st([Y,oscH])                    #14
anda(0xfc)                      #15
xora([Y,wavX])                  #16
ld(AC, X)                       #17
ld([Y,wavA])                    #18
ld(soundTable>>8, Y)            #19
adda([Y,X])                     #20
bmi('.sound1a')                 #21
bra('.sound1b')                 #22
anda(63)                        #23
label('.sound1a')
ld(63)                          #23
label('.sound1b')
adda([sample])                  #24
st([sample])                    #25

ld([xout]);                     C('Gets copied to XOUT')#26
nop()                           #27
ld([videoSync0], OUT);          C('End horizontal pulse')#28

# Count through the vertical blank interval until its last scan line
ld([videoY])                    #29
bpl('vBlankLast')               #30
adda(2)                         #31
st([videoY])                    #32

# Determine if we're in the vertical sync pulse
suba(1-2*(vBack-1))             #33
bne('vSync0')                   #34 Tests for end of vPulse
adda(2*vPulse)                  #35
ld(syncBits)                    #36 Entering vertical back porch
bra('vSync2')                   #37
st([videoSync0])                #38
label('vSync0')
bne('vSync1')                   #36 Tests for start of vPulse
ld(syncBits^vSync)              #37
bra('vSync3')                   #38 Entering vertical sync pulse
st([videoSync0])                #39
label('vSync1')
ld([videoSync0])                #38 Load current value
label('vSync2')
nop()                           #39
label('vSync3')
xora(hSync)                     #40 Precompute, as during the pulse there is no time
st([videoSync1])                #41

# Capture the serial input before the '595 shifts it out
ld([videoY]);                   C('Capture serial input')#42
xora(1-2*(vBack-1-1))           #43 Exactly when the 74HC595 has captured all 8 controller bits
bne('.ser0')                    #44
bra('.ser1')                    #45
st(IN, [serialRaw])             #46
label('.ser0')
nop()                           #46
label('.ser1')

# Update [xout] with the next sound sample every 4 scan lines.
# Keep doing this on 'videoC equivalent' scan lines in vertical blank.
ld([videoY])                    #47
anda(6)                         #48
bne('vBlankNormal')             #49
ld([sample])                    #50
label('vBlankSample')
ora(0x0f);                      C('New sound sample is ready')#51
anda([xoutMask])                #52
st([xout])                      #53
st(sample, [sample]);           C('Reset for next sample')#54

runVcpu(199-55, 'line1-39 typeC')#55 Appplication cycles (scan line 1-43 with sample update)
bra('sound1')                   #199
ld([videoSync0], OUT);          C('<New scan line start>')#0 Ends the vertical blank pulse at the right cycle

label('vBlankNormal')
runVcpu(199-51, 'line1-39 typeABD')#51 Application cycles (scan line 1-43 without sample update)
bra('sound1')                   #199
ld([videoSync0], OUT);          C('<New scan line start>')#0 Ends the vertical blank pulse at the right cycle

# Last blank line before transfering to visible area
label('vBlankLast')

# pChange = pNew & ~pOld
# nChange = nNew | ~nOld {DeMorgan}

# Filter raw serial input captured in last vblank (8 cycles)
ld(255);                        C('Filter controller input')#32
xora([serialLast])              #33
ora([serialRaw])                #34 Catch button-press events
anda([buttonState])             #35 Keep active button presses
ora([serialRaw])                #36 Auto-reset already-released buttons
st([buttonState])               #37
ld([serialRaw])                 #38
st([serialLast])                #39

# Respond to reset button (11 cycles)
xora(~buttonStart);             C('Check for soft reset')#40
bne('.restart0')                #41
ld([resetTimer])                #42 As long as button pressed
suba(1)                         #43 ... count down the timer
st([resetTimer])                #44
anda(127)                       #45
beq('.restart2')                #46
ld((vReset&255)-2)              #47 Start force reset when hitting 0
bra('.restart1')                #48 ... otherwise do nothing yet
bra('.restart3')                #49
label('.restart0')
ld(127)                         #43 Restore to ~2 seconds when not pressed
st([resetTimer])                #44
wait(49-45)                     #45
bra('.restart3')                #49
label('.restart1')
nop()                           #50
label('.restart2')
st([vPC])                       #48 Continue force reset
ld(vReset>>8)                   #49
st([vPC+1])                     #50
label('.restart3')

# --- Switch video mode when (only) select is pressed
ld([buttonState])               #51
xora(~buttonSelect)             #52
beq('.select0')                 #53
bra('.select1')                 #54
ld(0)                           #55
label('.select0')
ld(lo('videoD')^lo('videoF'))   #55 The XOR is actually done by the 2nd pass
label('.select1')
xora([videoDorF])               #56
st([videoDorF])                 #57
ld([buttonState])               #58
ora(buttonSelect)               #59
st([buttonState])               #60

runVcpu(196-61, 'line40')       #61 Application cycles (scan line 40)
# vAC==0 now
st([videoY])                    #196
st([frameX])                    #197
st([nextVideo])                 #198 videoA=0
ld([channel])                   #199 Advance to next sound channel
anda(3);                        C('<New scan line start>')#0
adda(1)                         #1
ld(hi('sound2'), Y)             #2
jmpy('sound2')                  #3
ld(syncBits^hSync, OUT)         #4 Start horizontal pulse

# Fillers
nop()
nop()
nop()
nop()
nop()

#-----------------------------------------------------------------------
# Extension SYS_NextByteIn_32
#-----------------------------------------------------------------------

# sysArgs[0:1] Current address
# sysArgs[2]   Checksum
# sysArgs[3]   Wait value (videoY)

label('SYS_NextByteIn_32')
ld([videoY])                    #15
xora([sysArgs+3])               #16
bne('.sysNbi')                  #17
ld([sysArgs+0], X)              #18
ld([sysArgs+1], Y)              #19
ld(IN)                          #20
st([Y,X])                       #21
adda([sysArgs+2])               #22
st([sysArgs+2])                 #23
ld([sysArgs+0])                 #24
adda(1)                         #25
st([sysArgs+0])                 #26
ld(hi('REENTER'), Y)            #27
jmpy('REENTER')                 #28
ld(-32/2)                       #29
# Restart instruction
label('.sysNbi')
ld([vPC])                       #19
suba(2)                         #20
st([vPC])                       #21
ld(-28/2)                       #22
ld(hi('REENTER'), Y)            #23
jmpy('REENTER')                 #24
nop()                           #25
assert pc()&255 == 255

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
bra('.next2')                   #0 Enter at '.next2' (so no startup overhead)
C('vCPU interpreter')

# --- Page boundary ---

align(0x100,0x100)
ld([vPC+1], Y)                  #1

# Fetch next instruction and execute it, but only if there are sufficient
# ticks left for the slowest instruction.
label('NEXT')
adda([vTicks]);                 C('Track elapsed ticks')#0 Actually counting down (AC<0)
blt('EXIT');                    C('Escape near time out')#1
label('.next2')
st([vTicks])                    #2
ld([vPC]);                      C('Advance vPC')#3
adda(2)                         #4
st([vPC], X)                    #5
ld([Y,X]);                      C('Fetch opcode')#6 Fetch opcode (actually a branch target)
st([Y,Xpp]);                    #7 Just X++
bra(AC);                        C('Dispatch')#8
ld([Y,X]);                      C('Prefetch operand')#9

# Resync with caller and return
label('EXIT')
adda(maxTicks)                  #3
bgt(pc()&255);                  C('Resync')#4
suba(1)                         #5
if fastRunVcpu:
  ld(2, Y)                      #6
else:
  ld([vReturn+1], Y)            #6
jmpy([vReturn+0]);              C('Return to caller')#7
ld(0)                           #8 AC should be 0 already. Still..
assert vOverheadInt ==          9

# Instruction LDWI: Load immediate constant (AC=$DDDD), 20 cycles
label('LDWI')
st([vAC])                       #10
st([Y,Xpp])                     #11 Just to increment X
ld([Y,X])                       #12 Fetch second operand
st([vAC+1])                     #13
ld([vPC])                       #14 Advance vPC one more
adda(1)                         #15
st([vPC])                       #16
ld(-20/2)                       #17
bra('NEXT')                     #18
#nop()                          #(19)
#
# Instruction LD: Load from zero page (AC=[D]), 18 cycles
label('LD')
ld(AC, X)                       #10,19 (overlap with LDWI)
ld([X])                         #11
st([vAC])                       #12
ld(0)                           #13
st([vAC+1])                     #14
ld(-18/2)                       #15
bra('NEXT')                     #16
#nop()                          #(17)
#
# Instruction LDW: Word load from zero page (AC=[D],[D+1]), 20 cycles
label('LDW')
ld(AC, X)                       #10,17 (overlap with LD)
adda(1)                         #11
st([vTmp])                      #12 Address of high byte
ld([X])                         #13
st([vAC])                       #14
ld([vTmp], X)                   #15
ld([X])                         #16
st([vAC+1])                     #17
bra('NEXT')                     #18
ld(-20/2)                       #19
#nop()                          #(20)
#
# Instruction STW: Word load from zero page (AC=[D],[D+1]), 20 cycles
label('STW')
ld(AC, X)                       #10,20 (overlap with LDW)
adda(1)                         #11
st([vTmp])                      #12 Address of high byte
ld([vAC])                       #13
st([X])                         #14
ld([vTmp], X)                   #15
ld([vAC+1])                     #16
st([X])                         #17
bra('NEXT')                     #18
ld(-20/2)                       #19

# Instruction BCC: Test AC sign and branch conditionally, 28 cycles
label('BCC')
ld([vAC+1])                     #10 First inspect high byte of vAC
bne('.cond2')                   #11
st([vTmp])                      #12
ld([vAC])                       #13 Additionally inspect low byte of vAC
beq('.cond3')                   #14
ld(1)                           #15
st([vTmp])                      #16
ld([Y,X])                       #17 Operand is the conditional
label('.cond1')
bra(AC)                         #18
ld([vTmp])                      #19

# Conditional EQ: Branch if zero (if(ALC==0)PCL=D)
label('EQ')
bne('.cond4')                   #20
label('.cond2')
beq('.cond5');                  C('AC=0 in EQ, AC!=0 from BCC...')#21,13 (overlap with BCC)
ld([Y,X])                       #22,14 (overlap with BCC)
#
# (continue BCC)
#label('.cond2')
#nop()                          #13
#nop()                          #14
nop()                           #15
label('.cond3')
bra('.cond1')                   #16
ld([Y,X])                       #17 Operand is the conditional
label('.cond4')
ld([vPC]);                      C('False condition')#22
bra('.cond6')                   #23
adda(1)                         #24
label('.cond5')
st([Y,Xpp]);                    C('True condition')#23 Just X++
ld([Y,X])                       #24
label('.cond6')
st([vPC])                       #25
bra('NEXT')                     #26
ld(-28/2)                       #27

# Conditional GT: Branch if positive (if(ALC>0)PCL=D)
label('GT')
ble('.cond4')                   #20
bgt('.cond5')                   #21
ld([Y,X])                       #22

# Conditional LT: Branch if negative (if(ALC<0)PCL=D), 16 cycles
label('LT')
bge('.cond4')                   #20
blt('.cond5')                   #21
ld([Y,X])                       #22

# Conditional GE: Branch if positive or zero (if(ALC>=0)PCL=D)
label('GE')
blt('.cond4')                   #20
bge('.cond5')                   #21
ld([Y,X])                       #22

# Conditional LE: Branch if negative or zero (if(ALC<=0)PCL=D)
label('LE')
bgt('.cond4')                   #20
ble('.cond5')                   #21
ld([Y,X])                       #22

# Instruction LDI: Load immediate constant (AC=$DD), 16 cycles
label('LDI')
st([vAC])                       #10
ld(0)                           #11
st([vAC+1])                     #12
ld(-16/2)                       #13
bra('NEXT')                     #14
#nop()                          #(15)
#
# Instruction ST: Store in zero page ([D]=ACL), 16 cycles
label('ST')
ld(AC, X)                       #10,15 (overlap with LDI)
ld([vAC])                       #11
lo('vAC')                       # XXX Preserve original artifact (remove in next version)
st([X])                         #12
ld(-16/2)                       #13
bra('NEXT')                     #14
#nop()                          #(15)
#
# Instruction POP: (LR=[SP++]), 26 cycles
label('POP')
ld([vSP], X)                    #10,15 (overlap with ST)
ld([X])                         #11
st([vLR])                       #12
ld([vSP])                       #13
adda(1, X)                      #14
ld([X])                         #15
st([vLR+1])                     #16
ld([vSP])                       #17
adda(2)                         #18
st([vSP])                       #19
label('next1')
ld([vPC])                       #20
suba(1)                         #21
st([vPC])                       #22
ld(-26/2)                       #23
bra('NEXT')                     #24
#nop()                          #(25)
#
# Conditional NE: Branch if not zero (if(ALC!=0)PCL=D)
label('NE')
beq('.cond4')                   #20,25 (overlap with POP)
bne('.cond5')                   #21
ld([Y,X])                       #22

# Instruction PUSH: ([--SP]=LR), 26 cycles
label('PUSH')
ld([vSP])                       #10
suba(1, X)                      #11
ld([vLR+1])                     #12
st([X])                         #13
ld([vSP])                       #14
suba(2)                         #15
st([vSP], X)                    #16
ld([vLR])                       #17
bra('next1')                    #18
st([X])                         #19

# Instruction LUP: ROM lookup (AC=ROM[AC+256*D]), 26 cycles
label('LUP')
ld([vAC+1], Y)                  #10
jmpy(251);                      C('Trampoline offset')#11
adda([vAC])                     #12

# Instruction ANDI: Logical-AND with constant (AC&=D), 16 cycles
label('ANDI')
anda([vAC])                     #10
st([vAC])                       #11
ld(0)                           #12 Clear high byte
st([vAC+1])                     #13
bra('NEXT')                     #14
ld(-16/2)                       #15

# Instruction ORI: Logical-OR with constant (AC|=D), 14 cycles
label('ORI')
ora([vAC])                      #10
st([vAC])                       #11
bra('NEXT')                     #12
ld(-14/2)                       #13

# Instruction XORI: Logical-XOR with constant (AC^=D), 14 cycles
label('XORI')
xora([vAC])                     #10
st([vAC])                       #11
bra('NEXT')                     #12
ld(-14/2)                       #13

# Instruction BRA: Branch unconditionally (PCL=D), 14 cycles
label('BRA')
st([vPC])                       #10
ld(-14/2)                       #11
bra('NEXT')                     #12
#nop()                          #(13)
#
# Instruction INC: Increment zero page byte ([D]++), 16 cycles
label('INC')
ld(AC, X)                       #10,13 (overlap with BRA)
ld([X])                         #11
adda(1)                         #12
st([X])                         #13
bra('NEXT')                     #14
ld(-16/2)                       #15

# Instruction ADDW: Word addition with zero page (AC+=[D]+256*[D+1]), 28 cycles
label('ADDW')
# The non-carry paths could be 26 cycles at the expense of (much) more code.
# But a smaller size is better so more instructions fit in this code page.
# 28 cycles is still 4.5 usec. The 6502 equivalent takes 20 cycles or 20 usec.
ld(AC, X)                       #10 Address of low byte to be added
adda(1)                         #11
st([vTmp])                      #12 Address of high byte to be added
ld([vAC])                       #13 Add the low bytes
adda([X])                       #14
st([vAC])                       #15 Store low result
bmi('.addw0')                   #16 Now figure out if there was a carry
suba([X])                       #17 Gets back the initial value of vAC
bra('.addw1')                   #18
ora([X])                        #19 Bit 7 is our lost carry
label('.addw0')
anda([X])                       #18 Bit 7 is our lost carry
nop()                           #19
label('.addw1')
anda(0x80, X)                   #20 Move the carry to bit 0 (0 or +1)
ld([X])                         #21
adda([vAC+1])                   #22 Add the high bytes with carry
ld([vTmp], X)                   #23
adda([X])                       #24
st([vAC+1])                     #25 Store high result
bra('NEXT')                     #26
ld(-28/2)                       #27

# Instruction PEEK: (AC=[AC]), 26 cycles
label('PEEK')
ld(hi('peek'), Y)               #10
jmpy('peek')                    #11
#ld([vPC])                      #12
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
ld([vPC]);                      C('Retry until sufficient time')#13,12 (overlap with PEEK)
suba(2)                         #14
st([vPC])                       #15
bra('REENTER')                  #16
ld(-20/2)                       #17
label('SYS')
adda([vTicks])                  #10
blt('retry')                    #11
ld([sysFn+1], Y)                #12
jmpy([sysFn])                   #13
#nop()                          #(14)
#
# Instruction SUBW: Word subtract with zero page (AC-=[D]+256*[D+1]), 28 cycles
# All cases can be done in 26 cycles, but the code will become much larger
label('SUBW')
ld(AC, X)                       #10,14 (overlap with SYS) Address of low byte to be subtracted
adda(1)                         #11
st([vTmp])                      #12 Address of high byte to be subtracted
ld([vAC])                       #13
bmi('.subw0')                   #14
suba([X])                       #15
st([vAC])                       #16 Store low result
bra('.subw1')                   #17
ora([X])                        #18 Bit 7 is our lost carry
label('.subw0')
st([vAC])                       #16 Store low result
anda([X])                       #17 Bit 7 is our lost carry
nop()                           #18
label('.subw1')
anda(0x80, X)                   #19 Move the carry to bit 0
ld([vAC+1])                     #20
suba([X])                       #21
ld([vTmp], X)                   #22
suba([X])                       #23
st([vAC+1])                     #24
ld(-28/2)                       #25
label('REENTER')
bra('NEXT');                    C('Return from SYS calls')#26
ld([vPC+1], Y)                  #27

# Instruction DEF: Define data or code (AC,PCL=PC+2,D), 18 cycles
label('DEF')
ld(hi('def'), Y)                #10
jmpy('def')                     #11
#st([vTmp])                     #12
#
# Instruction CALL: (LR=PC+2,PC=[D]-2), 26 cycles
label('CALL')
st([vTmp])                      #10,12 (overlap with DEF)
ld([vPC])                       #11
adda(2);                        C('Point to instruction after CALL')#12
st([vLR])                       #13
ld([vPC+1])                     #14
st([vLR+1])                     #15
ld([vTmp], X)                   #16
ld([X])                         #17
suba(2);                        C('Because NEXT will add 2')#18
st([vPC])                       #19
ld([vTmp])                      #20
adda(1, X)                      #21
ld([X])                         #22
st([vPC+1], Y)                  #23
bra('NEXT')                     #24
ld(-26/2)                       #25

# ALLOCA implementation
# Instruction ALLOCA: (SP+=D), 14 cycles
label('ALLOC')
adda([vSP])                     #10
st([vSP])                       #11
bra('NEXT')                     #12
ld(-14/2)                       #13

# The instructions below are all implemented in the second code page. Jumping
# back and forth makes each 6 cycles slower, but it also saves space in the
# primary page for the instructions above. Most of them are in fact not very
# critical, as evidenced by the fact that they weren't needed for the first
# Gigatron applications (Snake, Racer, Mandelbrot, Loader). By providing them
# in this way, at least they don't need to be implemented as a SYS extension.

# Instruction ADDI: Add small positive constant (AC+=D), 28 cycles
label('ADDI')
ld(hi('addi'), Y)               #10
jmpy('addi')                    #11
st([vTmp])                      #12

# Instruction SUBI: Subtract small positive constant (AC+=D), 28 cycles
label('SUBI')
ld(hi('subi'), Y)               #10
jmpy('subi')                    #11
st([vTmp])                      #12

# Instruction LSLW: Logical shift left (AC<<=1), 28 cycles
# Useful, because ADDW can't add vAC to itself. Also more compact.
label('LSLW')
ld(hi('lslw'), Y)               #10
jmpy('lslw')                    #11
ld([vAC])                       #12

# Instruction STLW: Store on stack (), 26 cycles
label('STLW')
ld(hi('stlw'), Y)               #10
jmpy('stlw')                    #11
#nop()                          #12
#
# Instruction LDLW: Load from stack (), 26 cycles
label('LDLW')
ld(hi('ldlw'), Y)               #10,12 (overlap with STLW)
jmpy('ldlw')                    #11
#nop()                          #12
#
# Instruction POKE: ([[D+1],[D]]=ACL), 28 cycles
label('POKE')
ld(hi('poke'), Y)               #10,12 (overlap with LDLW)
jmpy('poke')                    #11
st([vTmp])                      #12

# Instruction DOKE: (), 28 cycles
label('DOKE')
ld(hi('doke'), Y)               #10
jmpy('doke')                    #11
st([vTmp])                      #12

# Instruction DEEK: (), 28 cycles
label('DEEK')
ld(hi('deek'), Y)               #10
jmpy('deek')                    #11
#nop()                          #12
#
# Instruction ANDW: (AC&=[D]+256*[D+1]), 28 cycles
label('ANDW')
ld(hi('andw'), Y)               #10,12 (overlap with DEEK)
jmpy('andw')                    #11
#nop()                          #12
#
# Instruction ORW: (AC|=[D]+256*[D+1]), 28 cycles
label('ORW')
ld(hi('orw'), Y)                #10,12 (overlap with ANDW)
jmpy('orw')                     #11
#nop()                          #12
#
# Instruction XORW: (AC^=[D]+256*[D+1]), 26 cycles
label('XORW')
ld(hi('xorw'), Y)               #10,12 (overlap with ORW)
jmpy('xorw')                    #11
st([vTmp])                      #12
# We keep XORW 2 cycles faster than ANDW/ORW, because that
# can be useful for comparing numbers for equality a tiny
# bit faster than with SUBW

# Instruction RET: Function return (PC=LR-2), 16 cycles
label('RET')
ld([vLR])                       #10
assert pc()&255 == 0

#-----------------------------------------------------------------------
#
#  ROM page 4: Application interpreter extension
#
#-----------------------------------------------------------------------
align(0x100, 0x100)

# (Continue RET)
suba(2)                         #11
st([vPC])                       #12
ld([vLR+1])                     #13
st([vPC+1])                     #14
ld(hi('REENTER'), Y)            #15
jmpy('REENTER')                 #16
ld(-20/2)                       #17

# DEF implementation
label('def')
ld([vPC])                       #13
adda(2)                         #14
st([vAC])                       #15
ld([vPC+1])                     #16
st([vAC+1])                     #17
ld([vTmp])                      #18
st([vPC])                       #19
ld(hi('REENTER'), Y)            #20
ld(-26/2)                       #21
jmpy('REENTER')                 #22
nop()                           #23

# ADDI implementation
label('addi')
adda([vAC])                     #13
st([vAC])                       #14 Store low result
bmi('.addi0')                   #15 Now figure out if there was a carry
suba([vTmp])                    #16 Gets back the initial value of vAC
bra('.addi1')                   #17
ora([vTmp])                     #18 Bit 7 is our lost carry
label('.addi0')
anda([vTmp])                    #17 Bit 7 is our lost carry
nop()                           #18
label('.addi1')
anda(0x80, X)                   #19 Move the carry to bit 0 (0 or +1)
ld([X])                         #20
adda([vAC+1])                   #21 Add the high bytes with carry
st([vAC+1])                     #22 Store high result
ld(hi('REENTER'), Y)            #23
jmpy('REENTER')                 #24
ld(-28/2)                       #25

# SUBI implementation
label('subi')
ld([vAC])                       #13
bmi('.subi0')                   #14
suba([vTmp])                    #15
st([vAC])                       #16 Store low result
bra('.subi1')                   #17
ora([vTmp])                     #18 Bit 7 is our lost carry
label('.subi0')
st([vAC])                       #16 Store low result
anda([vTmp])                    #17 Bit 7 is our lost carry
nop()                           #18
label('.subi1')
anda(0x80, X)                   #19 Move the carry to bit 0
ld([vAC+1])                     #20
suba([X])                       #21
st([vAC+1])                     #22
ld(hi('REENTER'), Y)            #23
jmpy('REENTER')                 #24
ld(-28/2)                       #25

# LSLW implementation
label('lslw')
anda(128, X)                    #13
adda([vAC])                     #14
st([vAC])                       #15
ld([X])                         #16
adda([vAC+1])                   #17
adda([vAC+1])                   #18
st([vAC+1])                     #19
ld([vPC])                       #20
suba(1)                         #21
st([vPC])                       #22
ld(hi('REENTER'), Y)            #23
jmpy('REENTER')                 #24
ld(-28/2)                       #25

# STLW implementation
label('stlw')
adda([vSP])                     #13
st([vTmp])                      #14
adda(1, X)                      #15
ld([vAC+1])                     #16
st([X])                         #17
ld([vTmp], X)                   #18
ld([vAC])                       #19
st([X])                         #20
ld(hi('REENTER'), Y)            #21
jmpy('REENTER')                 #22
ld(-26/2)                       #23

# LDLW implementation
label('ldlw')
adda([vSP])                     #13
st([vTmp])                      #14
adda(1, X)                      #15
ld([X])                         #16
st([vAC+1])                     #17
ld([vTmp], X)                   #18
ld([X])                         #19
st([vAC])                       #20
ld(hi('REENTER'), Y)            #21
jmpy('REENTER')                 #22
ld(-26/2)                       #23

# POKE implementation
label('poke')
adda(1, X)                      #13
ld([X])                         #14
ld(AC, Y)                       #15
ld([vTmp], X)                   #16
ld([X])                         #17
ld(AC, X)                       #18
ld([vAC])                       #19
st([Y,X])                       #20
ld(hi('REENTER'), Y)            #21
jmpy('REENTER')                 #22
ld(-26/2)                       #23

# PEEK implementation
label('peek')
suba(1)                         #13
st([vPC])                       #14
ld([vAC], X)                    #15
ld([vAC+1], Y)                  #16
ld([Y,X])                       #17
st([vAC])                       #18
label('lupReturn')              #Nice coincidence that lupReturn can be here
ld(0)                           #19
st([vAC+1])                     #20
ld(hi('REENTER'), Y)            #21
jmpy('REENTER')                 #22
ld(-26/2)                       #23
#
# DOKE implementation
label('doke')
adda(1, X)                      #13,25 (overlap with peek)
ld([X])                         #14
ld(AC, Y)                       #15
ld([vTmp], X)                   #16
ld([X])                         #17
ld(AC, X)                       #18
ld([vAC])                       #19
st([Y,Xpp])                     #20
ld([vAC+1])                     #21
st([Y,X])                       #22
ld(hi('REENTER'), Y)            #23
jmpy('REENTER')                 #24
ld(-28/2)                       #25

# DEEK implementation
label('deek')
ld([vPC])                       #13
suba(1)                         #14
st([vPC])                       #15
ld([vAC], X)                    #16
ld([vAC+1], Y)                  #17
ld([Y,X])                       #18
st([Y,Xpp])                     #19
st([vAC])                       #20
ld([Y,X])                       #21
st([vAC+1])                     #22
ld(hi('REENTER'), Y)            #23
jmpy('REENTER')                 #24
ld(-28/2)                       #25

# ANDW implementation
label('andw')
st([vTmp])                      #13
adda(1,X)                       #14
ld([X])                         #15
anda([vAC+1])                   #16
st([vAC+1])                     #17
ld([vTmp], X)                   #18
ld([X])                         #19
anda([vAC])                     #20
st([vAC])                       #21
ld(-28/2)                       #22
ld(hi('REENTER'), Y)            #23
jmpy('REENTER')                 #24
#nop()                          #(25)

# ORW implementation
label('orw')
st([vTmp])                      #13,25 (overlap with andw)
adda(1, X)                      #14
ld([X])                         #15
ora([vAC+1])                    #16
st([vAC+1])                     #17
ld([vTmp], X)                   #18
ld([X])                         #19
ora([vAC])                      #20
st([vAC])                       #21
ld(-28/2)                       #22
ld(hi('REENTER'), Y)            #23
jmpy('REENTER')                 #24
#nop()                          #(25)

# XORW implementation
label('xorw')
adda(1, X)                      #13,25 (overlap with orw)
ld([X])                         #14
xora([vAC+1])                   #15
st([vAC+1])                     #16
ld([vTmp], X)                   #17
ld([X])                         #18
xora([vAC])                     #19
st([vAC])                       #20
ld(hi('REENTER'), Y)            #21
jmpy('REENTER')                 #22
ld(-26/2)                       #23

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
# Extension SYS_Random_34: Update entropy and copy to vAC
#-----------------------------------------------------------------------

# This same algorithm runs automatically once per vertical blank.
# Use this function to get numbers at a higher rate.
label('SYS_Random_34')
ld([frameCount])                #15
xora([entropy+1])               #16
xora([serialRaw])               #17
adda([entropy+0])               #18
st([entropy+0])                 #19
st([vAC+0])                     #20
adda([entropy+2])               #21
st([entropy+2])                 #22
bmi('.sysRnd0')                 #23
bra('.sysRnd1')                 #24
xora(64+16+2+1)                 #25
label('.sysRnd0')
xora(64+32+8+4)                 #25
label('.sysRnd1')
adda([entropy+1])               #26
st([entropy+1])                 #27
st([vAC+1])                     #28
ld(hi('REENTER'), Y)            #29
jmpy('REENTER')                 #30
ld(-34/2)                       #31

label('SYS_LSRW7_30')
ld([vAC])                       #15
anda(128, X)                    #16
ld([vAC+1])                     #17
adda(AC)                        #18
ora([X])                        #19
st([vAC])                       #20
ld([vAC+1])                     #21
anda(128, X)                    #22
ld([X])                         #23
st([vAC+1])                     #24
ld(hi('REENTER'), Y)            #25
jmpy('REENTER')                 #26
ld(-30/2)                       #27

label('SYS_LSRW8_24')
ld([vAC+1])                     #15
st([vAC])                       #16
ld(0)                           #17
st([vAC+1])                     #18
ld(hi('REENTER'), Y)            #19
jmpy('REENTER')                 #20
ld(-24/2)                       #21

label('SYS_LSLW8_24')
ld([vAC])                       #15
st([vAC+1])                     #16
ld(0)                           #17
st([vAC])                       #18
ld(hi('REENTER'), Y)            #19
jmpy('REENTER')                 #20
ld(-24/2)                       #21

#-----------------------------------------------------------------------
# Extension SYS_Draw4_30:
#-----------------------------------------------------------------------

# sysArgs[0:3]  Pixels
# sysArgs[4:5]  Position on screen

label('SYS_Draw4_30')
ld([sysArgs+4], X)              #15
ld([sysArgs+5], Y)              #16
ld([sysArgs+0])                 #17
st([Y,Xpp])                     #18
ld([sysArgs+1])                 #19
st([Y,Xpp])                     #20
ld([sysArgs+2])                 #21
st([Y,Xpp])                     #22
ld([sysArgs+3])                 #23
st([Y,Xpp])                     #24
ld(hi('REENTER'), Y)            #25
jmpy('REENTER')                 #26
ld(-30/2)                       #27

#-----------------------------------------------------------------------
# Extension SYS_VDrawBits_134:
#-----------------------------------------------------------------------

# Draw slice of a character
# sysArgs[0]    Color 0 (background)
# sysArgs[1]    Color 1 (pen)
# sysArgs[2]    8 bits, highest bit first (destructive)
# sysArgs[4:5]  Position on screen

label('SYS_VDrawBits_134')
ld([sysArgs+4], X)              #15
ld(0)                           #16
label('.vdb0')
st([vTmp])                      #17+i*14
adda([sysArgs+5], Y)            #18+i*14 Y=[sysPos+1]+vTmp
ld([sysArgs+2])                 #19+i*14 Select color
bmi('.vdb1')                    #20+i*14
bra('.vdb2')                    #21+i*14
ld([sysArgs+0])                 #22+i*14
label('.vdb1')
ld([sysArgs+1])                 #22+i*14
label('.vdb2')
st([Y,X])                       #23+i*14 Draw pixel
ld([sysArgs+2])                 #24+i*14 Shift byte left
adda(AC)                        #25+i*14
st([sysArgs+2])                 #26+i*14
ld([vTmp])                      #27+i*14 Loop counter
suba(7)                         #28+i*14
bne('.vdb0')                    #29+i*14
adda(8)                         #30+i*14
ld(hi('REENTER'), Y)            #129
jmpy('REENTER')                 #130
ld(-134/2)                      #131

#-----------------------------------------------------------------------
#  ROM page 5-6: Shift table and code
#-----------------------------------------------------------------------

# Lookup table for i>>n, with n in 1..6
# Indexing ix = i & ~b | (b-1), where b = 1<<(n-1)
#       ...
#       ld   <.ret
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
  ld(ix>>n); C('0b%s >> %d' % (''.join(reversed(pattern)), n))

assert pc()&255 == 255
bra([vTmp]);                    C('Jumps back into next page')

label('SYS_LSRW1_48')
assert pc()&255 == 0            #First instruction on this page must be a nop
nop()                           #15
ld(hi('shiftTable'), Y);        C('Logical shift right 1 bit (X >> 1)')#16
ld('.sysLsrw1a');               C('Shift low byte')#17
st([vTmp])                      #18
ld([vAC])                       #19
anda(0b11111110)                #20
jmpy(AC)                        #21
bra(255);                       C('Actually: bra $%04x' % (shiftTable+255))#22
label('.sysLsrw1a')
st([vAC])                       #26
ld([vAC+1]);                    C('Transfer bit 8')#27
anda(1)                         #28
adda(127)                       #29
anda(128)                       #30
ora([vAC])                      #31
st([vAC])                       #32
ld('.sysLsrw1b');               C('Shift high byte')#33
st([vTmp])                      #34
ld([vAC+1])                     #35
anda(0b11111110)                #36
jmpy(AC)                        #37
bra(255);                       C('Actually: bra $%04x' % (shiftTable+255))#38
label('.sysLsrw1b')
st([vAC+1])                     #42
ld(hi('REENTER'), Y)            #43
jmpy('REENTER')                 #44
ld(-48/2)                       #45

label('SYS_LSRW2_52')
ld(hi('shiftTable'), Y);        C('Logical shift right 2 bit (X >> 2)')#15
ld('.sysLsrw2a');               C('Shift low byte')#16
st([vTmp])                      #17
ld([vAC])                       #18
anda(0b11111100)                #19
ora( 0b00000001)                #20
jmpy(AC)                        #21
bra(255);                       C('Actually: bra $%04x' % (shiftTable+255))#22
label('.sysLsrw2a')
st([vAC])                       #26
ld([vAC+1]);                    C('Transfer bit 8:9')#27
adda(AC)                        #28
adda(AC)                        #29
adda(AC)                        #30
adda(AC)                        #31
adda(AC)                        #32
adda(AC)                        #33
ora([vAC])                      #34
st([vAC])                       #35
ld('.sysLsrw2b');               C('Shift high byte')#36
st([vTmp])                      #37
ld([vAC+1])                     #38
anda(0b11111100)                #39
ora( 0b00000001)                #40
jmpy(AC)                        #41
bra(255);                       C('Actually: bra $%04x' % (shiftTable+255))#42
label('.sysLsrw2b')
st([vAC+1])                     #46
ld(hi('REENTER'), Y)            #47
jmpy('REENTER')                 #48
ld(-52/2)                       #49

label('SYS_LSRW3_52')
ld(hi('shiftTable'), Y);        C('Logical shift right 3 bit (X >> 3)')#15
ld('.sysLsrw3a');               C('Shift low byte')#16
st([vTmp])                      #17
ld([vAC])                       #18
anda(0b11111000)                #19
ora( 0b00000011)                #20
jmpy(AC)                        #21
bra(255);                       C('Actually: bra $%04x' % (shiftTable+255))#22
label('.sysLsrw3a')
st([vAC])                       #26
ld([vAC+1]);                    C('Transfer bit 8:10')#27
adda(AC)                        #28
adda(AC)                        #29
adda(AC)                        #30
adda(AC)                        #31
adda(AC)                        #32
ora([vAC])                      #33
st([vAC])                       #34
ld('.sysLsrw3b');               C('Shift high byte')#35
st([vTmp])                      #36
ld([vAC+1])                     #37
anda(0b11111000)                #38
ora( 0b00000011)                #39
jmpy(AC)                        #40
bra(255);                       C('Actually: bra $%04x' % (shiftTable+255))#41
label('.sysLsrw3b')
st([vAC+1])                     #45
ld(-52/2)                       #46
ld(hi('REENTER'), Y)            #47
jmpy('REENTER')                 #48
#nop()                          #49

label('SYS_LSRW4_50')
ld(hi('shiftTable'), Y);        C('Logical shift right 4 bit (X >> 4)')#15,49
ld('.sysLsrw4a');               C('Shift low byte')#16
st([vTmp])                      #17
ld([vAC])                       #18
anda(0b11110000)                #19
ora( 0b00000111)                #20
jmpy(AC)                        #21
bra(255);                       C('Actually: bra $%04x' % (shiftTable+255))#22
label('.sysLsrw4a')
st([vAC])                       #26
ld([vAC+1]);                    C('Transfer bit 8:11')#27
adda(AC)                        #28
adda(AC)                        #29
adda(AC)                        #30
adda(AC)                        #31
ora([vAC])                      #32
st([vAC])                       #33
ld('.sysLsrw4b');               C('Shift high byte')#34
st([vTmp])                      #35
ld([vAC+1])                     #36
anda(0b11110000)                #37
ora( 0b00000111)                #38
jmpy(AC)                        #39
bra(255);                       C('Actually: bra $%04x' % (shiftTable+255))#40
label('.sysLsrw4b')
st([vAC+1])                     #44
ld(hi('REENTER'), Y)            #45
jmpy('REENTER')                 #46
ld(-50/2)                       #47

label('SYS_LSRW5_50')
ld(hi('shiftTable'), Y);        C('Logical shift right 5 bit (X >> 5)')#15
ld('.sysLsrw5a');               C('Shift low byte')#16
st([vTmp])                      #17
ld([vAC])                       #18
anda(0b11100000)                #19
ora( 0b00001111)                #20
jmpy(AC)                        #21
bra(255);                       C('Actually: bra $%04x' % (shiftTable+255))#22
label('.sysLsrw5a')
st([vAC])                       #26
ld([vAC+1]);                    C('Transfer bit 8:13')#27
adda(AC)                        #28
adda(AC)                        #29
adda(AC)                        #30
ora([vAC])                      #31
st([vAC])                       #32
ld('.sysLsrw5b');               C('Shift high byte')#33
st([vTmp])                      #34
ld([vAC+1])                     #35
anda(0b11100000)                #36
ora( 0b00001111)                #37
jmpy(AC)                        #38
bra(255);                       C('Actually: bra $%04x' % (shiftTable+255))#39
label('.sysLsrw5b')
st([vAC+1])                     #44
ld(-50/2)                       #45
ld(hi('REENTER'), Y)            #46
jmpy('REENTER')                 #47
#nop()                          #48

label('SYS_LSRW6_48')
ld(hi('shiftTable'), Y);        C('Logical shift right 6 bit (X >> 6)')#15,44
ld('.sysLsrw6a');               C('Shift low byte')#16
st([vTmp])                      #17
ld([vAC])                       #18
anda(0b11000000)                #19
ora( 0b00011111)                #20
jmpy(AC)                        #21
bra(255);                       C('Actually: bra $%04x' % (shiftTable+255))#22
label('.sysLsrw6a')
st([vAC])                       #26
ld([vAC+1]);                    C('Transfer bit 8:13')#27
adda(AC)                        #28
adda(AC)                        #29
ora([vAC])                      #30
st([vAC])                       #31
ld('.sysLsrw6b');               C('Shift high byte')#32
st([vTmp])                      #33
ld([vAC+1])                     #34
anda(0b11000000)                #35
ora( 0b00011111)                #36
jmpy(AC)                        #37
bra(255);                       C('Actually: bra $%04x' % (shiftTable+255))#38
label('.sysLsrw6b')
st([vAC+1])                     #42
ld(hi('REENTER'), Y)            #43
jmpy('REENTER')                 #44
ld(-48/2)                       #45

label('SYS_LSLW4_46')
ld(hi('shiftTable'), Y);        C('Logical shift left 4 bit (X << 4)')#15
ld('.sysLsrl4')                 #16
st([vTmp])                      #17
ld([vAC+1])                     #18
adda(AC)                        #19
adda(AC)                        #20
adda(AC)                        #21
adda(AC)                        #22
st([vAC+1])                     #23
ld([vAC])                       #24
anda(0b11110000)                #25
ora( 0b00000111)                #26
jmpy(AC)                        #27
bra(255);                       C('Actually: bra $%04x' % (shiftTable+255))#28
label('.sysLsrl4')
ora([vAC+1])                    #32
st([vAC+1])                     #33
ld([vAC])                       #34
adda(AC)                        #35
adda(AC)                        #36
adda(AC)                        #37
adda(AC)                        #38
st([vAC])                       #39
ld(-46/2)                       #40
ld(hi('REENTER'), Y)            #41
jmpy('REENTER')                 #42
#nop()                          #43

#-----------------------------------------------------------------------
# Extension SYS_Read3_40: Read 3 consecutive bytes from ROM
#-----------------------------------------------------------------------

# sysArgs[0:2]  Bytes (output)
# sysArgs[6:7]  ROM pointer (input)

label('SYS_Read3_40')
ld([sysArgs+7], Y)              #15,32
jmpy(128-7)                     #16 trampoline3a
ld([sysArgs+6])                 #17
label('txReturn')
st([sysArgs+2])                 #34
ld(hi('REENTER'), Y)            #35
jmpy('REENTER')                 #36
ld(-40/2)                       #37

def trampoline3a():
  """Read 3 bytes from ROM page"""
  while pc()&255 < 128-7:
    nop()
  bra(AC)                       #18
  C('Trampoline for page $%02x00 reading (entry)' % (pc()>>8))
  bra(123)                      #19
  st([sysArgs+0])               #21
  ld([sysArgs+6])               #22
  adda(1)                       #23
  bra(AC)                       #24
  bra(250)                      #25 trampoline3b

def trampoline3b():
  """Read 3 bytes from ROM page (continue)"""
  while pc()&255 < 256-6:
    nop()
  st([sysArgs+1])               #27
  C('Trampoline for page $%02x00 reading (continue)' % (pc()>>8))
  ld([sysArgs+6])               #28
  adda(2)                       #29
  ld(hi('txReturn'), Y)         #30
  bra(AC)                       #31
  jmpy('txReturn')              #32

#-----------------------------------------------------------------------
# Extension SYS_Unpack_56: Unpack 3 bytes into 4 pixels
#-----------------------------------------------------------------------

# sysArgs[0:2]  Packed bytes (input)
# sysArgs[0:3]  Pixels (output)

label('SYS_Unpack_56')
ld(soundTable>>8, Y)            #15
ld([sysArgs+2])                 #16 a[2]>>2
ora(0x03, X)                    #17
ld([Y,X])                       #18
st([sysArgs+3]);                C('-> Pixel 3')#19

ld([sysArgs+2])                 #20 (a[2]&3)<<4
anda(0x03)                      #21
adda(AC)                        #22
adda(AC)                        #23
adda(AC)                        #24
adda(AC)                        #25
st([sysArgs+2])                 #26
ld([sysArgs+1])                 #27 | a[1]>>4
ora(0x03, X)                    #28
ld([Y,X])                       #29
ora(0x03, X)                    #30
ld([Y,X])                       #31
ora([sysArgs+2])                #32
st([sysArgs+2]);                C('-> Pixel 2')#33

ld([sysArgs+1])                 #34 (a[1]&15)<<2
anda(0x0f)                      #35
adda(AC)                        #36
adda(AC)                        #37
st([sysArgs+1])                 #38

ld([sysArgs+0])                 #39 | a[0]>>6
ora(0x03, X)                    #40
ld([Y,X])                       #41
ora(0x03, X)                    #42
ld([Y,X])                       #43
ora(0x03, X)                    #44
ld([Y,X])                       #45
ora([sysArgs+1])                #46
st([sysArgs+1]);                C('-> Pixel 1')#47

ld([sysArgs+0])                 #48 a[1]&63
anda(0x3f)                      #49
st([sysArgs+0]);                C('-> Pixel 0')#50

ld(hi('REENTER'), Y)            #51
jmpy('REENTER')                 #52
ld(-56/2)                       #53

#-----------------------------------------------------------------------
# Extension SYS_PayloadCopy_34
#-----------------------------------------------------------------------

# sysArgs[0:1] Source address
# sysArgs[4]   Copy count
# sysArgs[5:6] Destination address

label('SYS_PayloadCopy_34')
ld([sysArgs+4])                 #15 Copy count
beq('.sysCc0')                  #16
suba(1)                         #17
st([sysArgs+4])                 #18
ld([sysArgs+0], X)              #19 Current pointer
ld([sysArgs+1], Y)              #20
ld([Y,X])                       #21
ld([sysArgs+5], X)              #22 Target pointer
ld([sysArgs+6], Y)              #23
st([Y,X])                       #24
ld([sysArgs+5])                 #25 Increment target
adda(1)                         #26
st([sysArgs+5])                 #27
bra('.sysCc1')                  #28
label('.sysCc0')
ld(hi('REENTER'), Y)            #18,29
wait(30-19)                     #19
label('.sysCc1')
jmpy('REENTER')                 #30
ld(-34/2)                       #31

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
    ld(byte)
    comment = C(comment)

trampoline()

#-----------------------------------------------------------------------

align(0x100, 0x100)

label('font82up')
for ch in range(32+50, 128):
  comment = 'Char %s' % repr(chr(ch))
  for byte in font.font[ch-32]:
    ld(byte)
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
  ld(key&127); C(comment)
  ld(key>>7)
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
  ld(4096/(i+16)-1)

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
    ld(ord(raw[i]))
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
        ld( ((bytes[0]&0b111111)>>0) + ((bytes[1]&0b000011)<<6) ); comment = C(comment)
        ld( ((bytes[1]&0b111100)>>2) + ((bytes[2]&0b001111)<<4) )
        ld( ((bytes[2]&0b110000)>>4) + ((bytes[3]&0b111111)<<2) )
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
ld([sysArgs+2], X)              #15 q,
ld([sysArgs+3], Y)              #16
ld([Y,X])                       #17
st([vTmp])                      #18
suba([sysArgs+4])               #19 X-
ld([sysArgs+0], X)              #20 p.
ld([sysArgs+1], Y)              #21
st([Y,X])                       #22
ld([sysArgs+0])                 #23 p 4- p=
suba(4)                         #24
st([sysArgs+0])                 #25
ld([vTmp])                      #26 q,
st([sysArgs+4])                 #27 X=
ld([sysArgs+2])                 #28 q<++
adda(1)                         #29
st([sysArgs+2])                 #30
bne('.sysRacer0')               #31 Self-repeat by adjusting vPC
ld([vPC])                       #32
bra('.sysRacer1')               #33
nop()                           #34
label('.sysRacer0')
suba(2)                         #33
st([vPC])                       #34
label('.sysRacer1')
ld(hi('REENTER'), Y)            #35
jmpy('REENTER')                 #36
ld(-40/2)                       #37

label('SYS_RacerUpdateVideoY_40')
ld([sysArgs+3])                 #15 8&
anda(8)                         #16
bne('.sysRacer2')               #17 [if<>0 1]
bra('.sysRacer3')               #18
ld(0)                           #19
label('.sysRacer2')
ld(1)                           #19
label('.sysRacer3')
st([vTmp])                      #20 tmp=
ld([sysArgs+1], Y  )            #21
ld([sysArgs+0])                 #22 p<++ p<++
adda(2)                         #23
st([sysArgs+0], X)              #24
xora(238)                       #25 238^
st([vAC])                       #26
st([vAC+1])                     #27
ld([sysArgs+2])                 #28 SegmentY
anda(254)                       #29 254&
adda([vTmp])                    #30 tmp+
st([Y,X])                       #31
ld([sysArgs+2])                 #32 SegmentY<++
adda(1)                         #33
st([sysArgs+2])                 #34
ld(hi('REENTER'), Y)            #35
jmpy('REENTER')                 #36
ld(-40/2)                       #37

#-----------------------------------------------------------------------
# Extension SYS_LoaderProcessInput_48
#-----------------------------------------------------------------------

# sysArgs[0:1] Source address
# sysArgs[2]   Checksum
# sysArgs[4]   Copy count
# sysArgs[5:6] Destination address

label('SYS_LoaderProcessInput_48')
ld([sysArgs+1], Y)              #15
ld([sysArgs+2])                 #16
bne('.sysPi0')                  #17
ld([sysArgs+0])                 #18
suba(65, X)                     #19 Point at first byte of buffer
ld([Y,X])                       #20 Command byte
st([Y,Xpp])                     #21 X++
xora(ord('L'))                  #22 This loader lumps everything under 'L'
bne('.sysPi1')                  #23
ld([Y,X]);                      C('Valid command')#24 Length byte
st([Y,Xpp])                     #25 X++
anda(63)                        #26 Bit 6:7 are garbage
st([sysArgs+4])                 #27 Copy count
ld([Y,X])                       #28 Low copy address
st([Y,Xpp])                     #29 X++
st([sysArgs+5])                 #30
ld([Y,X])                       #31 High copy address
st([Y,Xpp])                     #32 X++
st([sysArgs+6])                 #33
ld([sysArgs+4])                 #34
bne('.sysPi2')                  #35
# Execute code (don't care about checksum anymore)
ld([sysArgs+5]);                C('Execute')#36 Low run address
suba(2)                         #37
st([vPC])                       #38
st([vLR])                       #39
ld([sysArgs+6])                 #40 High run address
st([vPC+1])                     #41
st([vLR+1])                     #42
ld(hi('REENTER'), Y)            #43
jmpy('REENTER')                 #44
ld(-48/2)                       #45
# Invalid checksum
label('.sysPi0')
wait(25-19);                    C('Invalid checksum')#19 Reset checksum
# Unknown command
label('.sysPi1')
ld(ord('g'));                   C('Unknown command')#25 Reset checksum
st([sysArgs+2])                 #26
ld(hi('REENTER'), Y)            #27
jmpy('REENTER')                 #28
ld(-32/2)                       #29
# Loading data
label('.sysPi2')
ld([sysArgs+0]);                C('Loading data')#37 Continue checksum
suba(1, X)                      #38 Point at last byte
ld([Y,X])                       #39
st([sysArgs+2])                 #40
ld(hi('REENTER'), Y)            #41
jmpy('REENTER')                 #42
ld(-46/2)                       #43

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
  ld(packed[i])
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
define('romTypeValue', romTypeValue)
define('memSize',    memSize)
define('bootCount',  bootCount)
for i in range(3):
  define('entropy%d' % i, entropy+i)
define('videoY',     videoY)
define('frameCount', frameCount)
define('serialRaw',  serialRaw)
define('buttonState', buttonState)
define('xoutMask',   xoutMask)
define('vPC',        vPC)
define('vAC',        vAC)
define('vACH',       vAC+1)
define('vLR',        vLR)
define('vSP',        vSP)
define('romType',    romType)
define('sysFn',      sysFn)
for i in range(8):
  define('sysArgs%d' % i, sysArgs+i)
define('soundTimer', soundTimer)
define('ledTimer',   ledTimer)
define('ledTempo',   ledTempo)
define('userVars',   userVars)
define('videoTable', videoTable)
define('userCode',   userCode)
define('soundTable', soundTable)
define('screenMemory',screenMemory)
define('wavA',       wavA)
define('wavX',       wavX)
define('keyL',       keyL)
define('keyH',       keyH)
define('oscL',       oscL)
define('oscH',       oscH)
define('maxTicks',   maxTicks)
# XXX This is a hack (trampoline() is probably in the wrong module):
define('vPC+1',      vPC+1)

# Load pre-compiled GT1 file
#
# !!! This is a particularly bad example, because this program loads on top of the
# !!! ROM loader and SYS_Exec_88 must be modified. Checked-in as a reminder only!
#
#gt1File = 'tetris.gt1'
#name = 'Tetris'
#f = open(gt1File, 'rb')
#raw = f.read()
#f.close()
#print
#print 'Include file %s label %s ROM %04x' % (gt1File, name, pc())
#label(name)
#raw = chr(ord(raw[0]) + 0x80) + raw[1:] # Patch zero page loading (only for 32KB system)
#raw = raw[:-2] # Drop start address
#program = gcl.Program(userCode, name, forGt1=True)
#zpReset(userVars)
#for byte in raw:
  #program.putInRomTable(ord(byte))
#program.end()

# Compile built-in GCL programs
for gclSource in argv[1:]:
  name = gclSource.rsplit('.', 1)[0]
  name = name.rsplit('/', 1)[-1]
  print
  print 'Compile file %s label %s ROM %04x' % (gclSource, name, pc())
  label(name)
  program = gcl.Program(userCode, name)
  zpReset(userVars)
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
writeRomFiles(argv[0])
