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
#
#  TODO:
#
#  XXX: Input handling update
#  XXX: Rising planet
#         8x8 pixel block in 48 bytes. Maybe 2 bytes overhead: 50
#         5 pixel blocks in one page
#         3 pages encode 1 column of 15 blocks
#         3*20 = 60 pages to encode an image: 15 kB. Good for Parrot, Jupiter, Baboon
#         SYS Routine to unpack 6 bytes would be nice
#  XXX: Image packed in memory
#  XXX: Serial read from ROM tables, ignoring page boundraries
#  XXX: ROM load of code / bootstrapping
#  XXX: Main menu
#  XXX: Logo drawing
#  XXX: DrawDecimal routine
#  XXX: Key/pitch table
#  XXX: Music sequencer
#  XXX: More waveforms
#  XXX: Serial loading of programs with Arduino/Trinket
#  XXX: Multitasking/threading/sleeping (start with date/time clock in GCL)
#  XXX: Loading and starting of programs
#  XXX: Better shift-table (use ROM?)
#  XXX: Better notation for 'call'. () or []
#  XXX: Prefix notation for high/low byte >X++ instead of X>++
#  XXX: Readability of asm.py instructions
#  XXX: Sprites by scanline 4 reset method? ("videoG"=graphics)
#  XXX: Decay, using Karplus-Strong
#  XXX: Scoping for variables or some form of local variables?
#  XXX: Simple GCL programs might be compiled by the host instead of offline?
#  XXX: Dynamic memory allocation
#  XXX: Macros
#-----------------------------------------------------------------------

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
vBack = 33      # Vertical back porch
vgaLines = vFront + vPulse + vBack + 480
vgaClock = 25.175e+06

# Video adjustments for Gigatron
# 1. Our clock is (slighty) slower than 1/4th VGA clock. Not all monitors will accept
#    the decreased frame rate, so we restore the frame rate to above minimum 59.94 Hz
#    by cutting some lines from the vertical front porch.
vFrontAdjust = vgaLines - int(4 * cpuClock / vgaClock * vgaLines)
vFront -= vFrontAdjust
# 2. Extend vertical sync pulse so we can feed the game controller the same signal.
#    This is needed for controllers based on the 4021 instead of 74165
vPulseExtension = max(0, 8-vPulse)
vPulse += vPulseExtension
# 3. Borrow these lines from the back porch so the refresh rate remains unaffected
vBack -= vPulseExtension

# Game controller bits
buttonRight     = 1
buttonLeft      = 2
buttonDown      = 4
buttonUp        = 8
buttonStart     = 16
buttonSelect    = 32
buttonA         = 64
buttonB         = 128

# Compile option: True restricts the calling of interpreter to calls from
# page 2, for 2 cycles less interpreter ENTER/EXIT overhead
fastRunVcpu = True

#-----------------------------------------------------------------------
#
#  RAM page 0: variables
#
#-----------------------------------------------------------------------

# Memory size in pages from auto-detect
memSize = zpByte()

# The current channel number for sound generation. Advanced every scanline
# and independent of the vertical refresh to maintain constant oscillation.
channel = zpByte()

# Next sound sample being synthesized
sample = zpByte()
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
bootCount = zpByte() # 0 for cold boot
bootCheck = zpByte() # Checksum

# Extended output (blinkenlights in bit 0:3 and audio in but 4:7). This
# value must be present in AC during a rising hSync edge. It then gets
# copied to the XOUT register by the hardware. The XOUT register is only
# accessible in this indirect manner because it isn't part of the core
# CPU architecture.
xout      = zpByte()
xoutMask  = zpByte() # The blinkenlights and sound on/off state

# Visible video
screenY   = zpByte() # Counts up from 0 to 238 in steps of 2
frameX    = zpByte() # Starting byte within page
frameY    = zpByte() # Page number of current pixel row (updated by videoA)
nextVideo = zpByte()
videoDorF = zpByte() # Scanline mode ('D' or 'F')

# Vertical blank, reuse some variables
blankY     = screenY # Counts down during vertical blank (44 to 0)
videoSync0 = frameX  # Vertical sync type on current line (0xc0 or 0x40)
videoSync1 = frameY  # Same during horizontal pulse

# System clock
frameCount = zpByte(1)

# Generic function return address
returnTo   = zpByte(2)

# Return from EEPROM lookups
lookupReturn = zpByte(2)

# Two bytes of havested entropy
entropy    = zpByte(3)

# Play sound if non-zero, count down and stop sound when zero
soundTimer = zpByte()

ledTimer        = zpByte() # Number of ticks until next LED change
ledState        = zpByte() # Current LED state
ledTempo        = zpByte() # Next value for ledTimer after LED state change
# Fow now the LED state machine itself is hard-coded in the program ROM

# Serial input (game controller)
serialInput     = zpByte()
buttonState     = zpByte() # Filtered button state

# vCPU interpreter
vPC     = zpByte(2)             # Interpreter program counter, points into RAM
vAC     = zpByte(2)             # Interpreter accumulator, 16-bits
vRT     = zpByte(2)             # Return address, for returning after CALL
define('vRT', vRT)
vSP     = zpByte(1)             # Stack pointer
vTicks  = zpByte()              # Interpreter ticks are units of 2 clocks
vTmp    = zpByte()

args    = zpByte(2)             # Space to pass arguments to SYS calls

# All bytes above, except 0x80, are free for temporary/scratch/stacks etc
zpFree  = zpByte(0)

# Export some zero page variables to GCL
define('entropy',    entropy)
define('soundTimer', soundTimer)
define('screenY',    screenY)
define('frameCount', frameCount)
define('args',       args)
define('arg0',       args+0)

#-----------------------------------------------------------------------
#
#  RAM page 1: video line table
#
#-----------------------------------------------------------------------

# Byte 0-239 define the video lines
scanTablePage = 0x01       # Indirection table: Y[0] dX[0]  ..., Y[119] dX[119]

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

shiftTablePage = 0x02

#-----------------------------------------------------------------------
#
#  RAM page 3-7: application code GCL
#
#-----------------------------------------------------------------------

bStart = 0x0300
bTop   = 0x07ff

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
  print '%04x runVcpu %s cycles (%s)' % (pc(), n, ref)
  comment = 'Run vCPU for %s cycles' % n
  if ref:
    comment += ' (%s)' % ref
  if n % 2 != (vOverheadExt + vOverheadInt) % 2:
    nop()
    comment = C(comment)
    n -= 1
  n -= vOverheadExt + vOverheadInt + 2*maxTicks

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
  st(d(returnTo))               #1
  if fastRunVcpu:
    # In this mode [returnTo+1] will not be used
    assert returnPc>>8 == 2
  else:
    # Allow interpreter to be called from anywhere
    ld(val(returnPc>>8))        #2
    st(d(returnTo+1))           #3
  ld(val(hi('ENTER')),regY)     #4
  jmpy(d(lo('ENTER')))          #5
  ld(val(n))                    #6

def trampoline():
  while pc()&255 < 256-5:
    nop()

  bra(busAC);                   #13
  C('Trampoline for page $%02x00 lookups' % (pc()>>8))
  bra(val(253))                 #14
  ld(d(hi('rLookup')),regY)     #16
  jmpy(d(lo('rLookup')))        #17
  ld(d(vPC+1),busRAM|regY)      #18

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
st(d(zpFree+0),busAC|ea0DregX)
st(d(zpFree+1),busAC|ea0DregY)
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
ldzp(d(zpFree+0))
adda(val(1))
bne(d(lo('.initEnt0')))
st(d(zpFree+0),busAC|ea0DregX)
ldzp(d(zpFree+1))
adda(val(1))
bne(d(lo('.initEnt0')))
st(d(zpFree+1),busAC|ea0DregY)

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

# Init system timer
ld(val(-1));                    C('Setup system timer')
st(d(frameCount))

# Initialize scan table for default video layout
ld(val(scanTablePage),regY);    C('Setup video scan table')
ld(val(0),regX)
ld(val(screenPages))
st(eaYXregOUTIX)         # Yi  = 0x08+i
label('.initVideo')
st(val(0),eaYXregOUTIX)  # dXi = 0
adda(val(1))
bge(d(lo('.initVideo'))) # stops at $80
st(eaYXregOUTIX)         # Yi  = 0x08+i
ld(d(lo('videoF')))
st(d(videoDorF))

# Init the shift2-right table for sound
ld(val(shiftTablePage),regY);   C('Setup shift2 table')
ld(val(0))
st(d(channel))
label('.loop')
st(d(zpFree+0))
adda(busAC)
adda(busAC, regX)
ldzp(d(zpFree+0))
st(eaYXregAC)
adda(val(1))
xora(val(0x40))
bne(d(lo('.loop')))
xora(val(0x40))

# Init LED sequencer
ld(val(120));                   C('Setup LED sequencer')
st(d(ledTimer))
ld(val(0))
st(d(ledState))
ld(val(60/6))
st(d(ledTempo))

# Setup a G-major chord to play
G3, G4, B4, D5 = 824, 1648, 2064, 2464

ld(val(1),regY);                C('Setup channel 1')
ld(val(keyL),regX)
st(d(G3 & 0x7f),eaYXregOUTIX)
st(d(G3 >> 7),eaYXregAC)

ld(val(2),regY);                C('Setup channel 2')
ld(val(keyL),regX)
st(d(G4&0x7f),eaYXregOUTIX)
st(d(G4>>7),eaYXregAC)

ld(val(3),regY);                C('Setup channel 3')
ld(val(keyL),regX)
st(d(B4&0x7f),eaYXregOUTIX)
st(d(B4>>7),eaYXregAC)

ld(val(4),regY);                C('Setup channel 4')
ld(val(keyL),regX)
st(d(D5&0x7f),eaYXregOUTIX)
st(d(D5>>7),eaYXregAC)

ld(val(0));                     C('Setup sound timer')
st(d(soundTimer))

ld(val(-1));                    C('Setup serial input')
st(d(serialInput))
st(d(buttonState))

ld(val(0b0111));                C('LEDs |***O|')
ld(val(syncBits^hSync),regOUT)
ld(val(syncBits),regOUT)

ld(val(lo('.retn')));           C('Bootstrap vCPU')
st(d(returnTo+0))
ld(val(hi('.retn')))
ld(d(hi('initVcpu')),regY)
jmpy(d(lo('initVcpu')))
st(d(returnTo+1))
label('.retn')

ld(val(0b1111));                C('LEDs |****|')
ld(val(syncBits^hSync),regOUT)
ld(val(syncBits),regOUT)
st(d(xoutMask)) # Setup for control by video loop
st(d(xout))

ld(d(hi('vBlank')),busD|ea0DregY);C('Enter video loop')
jmpy(d(lo('vBlank')))
ld(val(syncBits))

#-----------------------------------------------------------------------
#
#  ROM page 1-2: Video loop
#
#-----------------------------------------------------------------------
align(0x100, 0x200)

# Back porch A: first of 4 repeated scanlines
# - Fetch next Yi and store it for retrieval in the next scanlines
# - Calculate Xi from dXi, but there is no cycle time left to store it as well
label('videoA')
assert(lo('videoA') == 0)       # videoA starts at the page boundary
ld(d(lo('videoB')))             #29
st(d(nextVideo))                #30
ld(d(scanTablePage), regY)      #31
ld(d(screenY), busRAM|regX)     #32
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
ld(val(syncBits), regOUT);      C('<New scanline start>')#0 Back to black

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
ld(d(shiftTablePage),regY)      #18
ld(busRAM|eaYXregAC)            #19
adda(d(sample), busRAM|ea0DregAC)#20
st(d(sample))                   #21
wait(26-22)                     #22
ldzp(d(xout))                   #26
bra(d(nextVideo) | busRAM)      #27
ld(val(syncBits), regOUT);      C('End horizontal pulse')#28

# Back porch B: second of 4 repeated scanlines
# - Recompute Xi from dXi and store for retrieval in the next scanlines
label('videoB')
ld(d(lo('videoC')))             #29
st(d(nextVideo))                #30
ld(d(scanTablePage), regY)      #31
ldzp(d(screenY))                #32
adda(d(1), regX)                #33
ldzp(d(frameX))                 #34
adda(eaYXregAC, busRAM)         #35
st(d(frameX), busAC|ea0DregX)   #36 Undocumented opcode "store in RAM and X"!
ld(d(frameY), busRAM|regY)      #37
bra(d(lo('pixels')))            #38
ld(val(syncBits))               #39

# Back porch C: third of 4 repeated scanlines
# - Nothing new to do, Yi and Xi are known
label('videoC')
ldzp(d(sample));                C('New sound sample is ready')#29 First something that didn't fit in the audio loop
ora(d(0x0f))                    #30
anda(d(xoutMask),busRAM|ea0DregAC)#31
st(d(xout))                     #32 Update [xout] with new sample (4 channels just updated)
st(val(sample),ea0DregAC|busD); C('Reset for next sample')#33 Reset for next sample
ldzp(d(videoDorF));             C('Mode for scanline 4')#34 Now back to video business
st(d(nextVideo))                #35
ld(d(frameX),busRAM|regX)       #36
ld(d(frameY),busRAM|regY)       #37
bra(d(lo('pixels')))            #38
ld(val(syncBits))               #39

# Back porch D: last of 4 repeated scanlines
# - Calculate the next frame index
# - Decide if this is the last line or not
label('videoD')                 # Default video mode
ld(d(frameX), busRAM|regX)      #29
ldzp(d(screenY))                #30
suba(d((120-1)*2))              #31
beq(d(lo('last')))              #32
ld(d(frameY), busRAM|regY)      #33
adda(d(120*2))                  #34 More pixel lines to go
st(d(screenY))                  #35
ld(d(lo('videoA')))             #36
st(d(nextVideo))                #37
bra(d(lo('pixels')))            #38
ld(val(syncBits))               #39
label('last')
wait(36-34)                     #34 No more pixel lines
ld(d(lo('videoE')))             #36
st(d(nextVideo))                #37
bra(d(lo('pixels')))            #38
ld(val(syncBits))               #39

# Back porch "E": after the last line
# - Go back to program page 0 and enter vertical blank
label('videoE') # Exit visible area
ld(d(hi('vBlank')),ea0DregY)    #29
jmpy(d(lo('vBlank'))   )        #30
ld(val(syncBits))               #31

# Back porch "F": scanlines and fast mode
label('videoF')                 # Fast video mode
ldzp(d(screenY))                #29
suba(d((120-1)*2))              #30
bne(d(lo('notlast')))           #31
adda(d(120*2))                  #32
bra(d(lo('.join')))             #33
ld(d(lo('videoE')))             #34 No more visible lines
label('notlast')
st(d(screenY))                  #33 More visible lines
ld(d(lo('videoA')))             #34
label('.join')
st(d(nextVideo))                #35
runVcpu(199-36, 'line41-521 typeF')#36 Application (every 4th of scanlines 41-521)
ld(d(hi('soundF')), busD|ea0DregY)#199 XXX This is on the current page
jmpy(d(lo('soundF')));          C('<New scanline start>')#0
ldzp(d(channel))                #1 Advance to next sound channel

# Vertical blank part of video loop
label('vBlank')                 # Start of vertical blank interval
assert(pc()&255<16)             # Assure that we are in the beginning of the next page

st(d(videoSync0))               #32
C('Start vertical blank interval')
ld(val(syncBits^hSync))         #33
st(d(videoSync1))               #34

# (Re)initialize carry table for robustness
st(d(0x00), ea0DregAC|busD)     #35
ld(val(0x01))                   #36
st(d(0x80), ea0DregAC|busAC)    #37

# --- Uptime frame count

ldzp(d(frameCount))             #38
adda(val(1))                    #39
st(d(frameCount))               #40

# --- Mix entropy (11 cycles)
xora(d(entropy+1),busRAM)       #41
xora(d(serialInput),busRAM)     #42 Mix in serial input
adda(d(entropy+0),busRAM)       #43
st(d(entropy+0))                #44
adda(d(entropy+2),busRAM)       #45 Some hidden state
st(d(entropy+2))                #46
bmi(d(lo('.rnd0')))             #47
bra(d(lo('.rnd1')))             #48
xora(val(64+16+2+1))            #49
label('.rnd0')
xora(val(64+32+8+4))            #49
label('.rnd1')
adda(d(entropy+1),busRAM)       #50
st(d(entropy+1))                #51

# XXX TODO...

# When the total number of scanlines per frame is not an exact multiple of the (4) channels,
# there will be an audible discontinuity if no measure is taken. This static noise can be
# suppressed by swallowing the first `lines%4' partial samples after transitioning into
# vertical blank. This is easiest if the modulo is 0 (do nothing) or 1 (reset sample while in
# the first blank scanline). For the two other cases there is no solution yet: give a warning.
soundDiscontinuity = (vFront+vPulse+vBack) % 4
extra = 0
if soundDiscontinuity == 1:
  st(val(sample), ea0DregAC|busD)
  extra += 1
if soundDiscontinuity > 1:
  print "Warning: sound discontinuity not supressed"

extra+=11 # For sound on/off and sound timer hack below. XXX solve properly

runVcpu(179-52-extra, 'line0')  #52 Application cycles (scanline 0)

# --- LED sequencer (19 cycles)

ldzp(d(ledTimer))               #179
bne(d(lo('.leds4')))            #180

ld(d(lo('.leds0')))             #181
adda(d(ledState)|busRAM)        #182
bra(busAC)                      #183
bra(d(lo('.leds1')))            #184

label('.leds0')
ld(d(0b1111));C('LEDs |****|')  #185
ld(d(0b0111));C('LEDs |***O|')  #185
ld(d(0b0011));C('LEDs |**OO|')  #185
ld(d(0b0001));C('LEDs |*OOO|')  #185
ld(d(0b0010));C('LEDs |O*OO|')  #185
ld(d(0b0100));C('LEDs |OO*O|')  #185
ld(d(0b1000));C('LEDs |OOO*|')  #185
ld(d(0b0100));C('LEDs |OO*O|')  #185
ld(d(0b0010));C('LEDs |O*OO|')  #185
ld(d(0b0001));C('LEDs |*OOO|')  #185
ld(d(0b0011));C('LEDs |**OO|')  #185
ld(d(0b0111));C('LEDs |***O|')  #185
ld(d(0b1111));C('LEDs |****|')  #185
ld(d(0b1110));C('LEDs |O***|')  #185
ld(d(0b1100));C('LEDs |OO**|')  #185
ld(d(0b1000));C('LEDs |OOO*|')  #185
ld(d(0b0100));C('LEDs |OO*O|')  #185
ld(d(0b0010));C('LEDs |O*OO|')  #185
ld(d(0b0001));C('LEDs |*OOO|')  #185
ld(d(0b0010));C('LEDs |O*OO|')  #185
ld(d(0b0100));C('LEDs |OO*O|')  #185
ld(d(0b1000));C('LEDs |OOO*|')  #185
ld(d(0b1100));C('LEDs |OO**|')  #185
ld(d(0b1110+128))               #185
C('LEDs |O***|')

label('.leds1')
st(d(xoutMask))                 #186 Temporarily park new state here

bmi(d(lo('.leds2')))            #187
bra(d(lo('.leds3')))            #188
ldzp(d(ledState))               #189
label('.leds2')
ld(val(-1))                     #189
label('.leds3')
adda(val(1))                    #190
st(d(ledState))                 #191

bra(d(lo('.leds5')))            #192
ldzp(d(ledTempo))               #193 Setup the LED timer for the next period

label('.leds4')
wait(192-182)                   #182
ldzp(d(ledTimer))               #192
suba(d(1))                      #193

label('.leds5')
st(d(ledTimer))                 #194

ldzp(d(xoutMask))               #195 Low 4 bits are the LED output
anda(val(0b00001111))           #196
st(d(xoutMask))                 #197

# --- Sound on/off

ldzp(d(soundTimer))
bne(d(lo('.snd0')))
bra(d(lo('.snd1')))
ld(val(0))
label('.snd0')
ld(val(0xf0))
label('.snd1')
ora(d(xoutMask),busRAM)
st(d(xoutMask))

# --- Sound timer count down

ldzp(d(soundTimer))
beq(d(lo('.snd2')))
bra(d(lo('.snd3')))
suba(val(1))
label('.snd2')
ld(val(0))
label('.snd3')
st(d(soundTimer))

ld(val(vFront+vPulse+vBack-2))  #198 `-2' because first and last are different
st(d(blankY))                   #199
ld(d(videoSync0), busRAM|regOUT);C('<New scanline start>')#0

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
ld(d(shiftTablePage),regY)      #18
ld(busRAM|eaYXregAC)            #19
adda(d(sample), busRAM|ea0DregAC)#20
st(d(sample))                   #21
wait(26-22)                     #22
ldzp(d(xout))                   #26
nop()                           #27
ld(d(videoSync0), busRAM|regOUT);C('End horizontal pulse')#28

# Count down the vertical blank interval until its last scan line
ldzp(d(blankY))                 #29
beq(d(lo('vBlankLast0')))       #30
suba(d(1))                      #31
st(d(blankY))                   #32

# Determine if we're in the vertical sync pulse
suba(d(vBack-1))                #33
bne(d(lo('vSync0')))            #34 Tests for end of vPulse
suba(d(vPulse))                 #35
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

# Capture the serial input
ldzp(d(blankY));                C('Capture serial input')#42
xora(val(vBack-1-1))            #43 Exactly when the 74HC595 has captured all 8 controller bits
bne(d(lo('.ser0')))             #44
bra(d(lo('.ser1')))             #45
st(d(serialInput),busIN)        #46
label('.ser0')
nop()                           #46
label('.ser1')

# Update [xout] with the next sound sample every 4 scan lines.
# Keep doing this on 'videoC equivalent' scan lines in vertical blank.
ldzp(d(blankY))                 #47
anda(d(3))                      #48
bne(d(lo('vBlankNormal')))      #49
ldzp(d(sample))                 #50
label('vBlankSample')
ora(d(0x0f));                   C('New sound sample is ready')#51
anda(d(xoutMask),busRAM|ea0DregAC)#52
st(d(xout))                     #53
st(val(sample), ea0DregAC|busD); C('Reset for next sample')#54

runVcpu(199-55, 'line1-39 typeC')#55 Appplication cycles (scanline 1-43 with sample update)
bra(d(lo('sound1')))            #199
ld(d(videoSync0), busRAM|regOUT);C('<New scanline start>')#0 Ends the vertical blank pulse at the right cycle

label('vBlankNormal')
runVcpu(199-51, 'line1-39 typeABD')#51 Application cycles (scanline 1-43 without sample update)
bra(d(lo('sound1')))            #199
ld(d(videoSync0), busRAM|regOUT);C('<New scanline start>')#0 Ends the vertical blank pulse at the right cycle

# Last blank line before transfering to visible area
label('vBlankLast0')
ld(val(0))                      #32
st(d(frameX))                   #33
st(d(nextVideo))                #34

label('vBlankLast1')

# The serial game controller freaks out when two buttons are pressed: it just sends zeroes.
# When we see this, preserve the old value and assume buttonA was added.
ldzp(d(serialInput))            #35
beq(lo('.multi0'))              #36
bra(lo('.multi1'))              #37
st(d(buttonState),busAC)        #38
label('.multi0')
ld(val(buttonA))                #38
label('.multi1')
ora(d(buttonState),busRAM)      #39
st(d(buttonState),busAC)        #40

# --- Switch video mode when (only) select is pressed
ldzp(d(buttonState))            #41
xora(val(~buttonSelect))        #42
beq(d(lo('.sel0')))             #43
bra(d(lo('.sel1')))             #44
ld(val(0))                      #45
label('.sel0')
ld(val(lo('videoD')^lo('videoF')))#45
label('.sel1')
xora(d(videoDorF),busRAM)       #46
st(d(videoDorF))                #47

runVcpu(199-48, 'line40')       #48 Application cycles (scanline 40)
ldzp(d(channel))                #199 Advance to next sound channel
anda(val(3));                   C('<New scanline start>')#0
adda(val(1))                    #1
ld(d(hi('sound2')), busD|ea0DregY)#2
jmpy(d(lo('sound2')))           #3
ld(val(syncBits^hSync), regOUT) #4 Start horizontal pulse


#-----------------------------------------------------------------------
#
#  ROM page 3: Application interpreter
#
#-----------------------------------------------------------------------

align(0x100,0x100)

# Enter the timing-aware application interpreter (aka virtual CPU, vCPU)
#
# This routine will execute as many as possible instructions in the
# alotted time. When time runs out, it synchronizes such that the total
# duration matches the caller's request. Durations are counted in `ticks',
# which are multiples of 2 clock cycles.
#
# Use the runVcpu() macro as entry point
#
label('ENTER')
bra(d(lo('.next2')))            #0 Enter at '.next2' (so no startup overhead)
C('vCPU interpreter')
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
  ld(d(returnTo+1),busRAM|regY) #6
jmpy(d(returnTo+0)|busRAM);     C('Return to caller')#7
nop()                           #8
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
ld(busAC,regX)                  #10 (overlap with LDWI)
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
ld(busAC,regX)                  #10 (overlap with LD)
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
ld(busAC,regX)                  #10 (overlap with LDW)
adda(val(1))                    #11
st(d(vTmp))                     #12 Address of high byte
ldzp(d(vAC))                    #13
st(ea0XregAC)                   #14
ld(d(vTmp),busRAM|regX)         #15
ldzp(d(vAC+1))                  #16
st(ea0XregAC)                   #17
bra(d(lo('NEXT')))              #18
ld(val(-20/2))                  #19

# Instruction COND: Test AC sign and branch conditionally, 28 cycles
label('COND')
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
label('.cond2')
nop()                           #13
nop()                           #14
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

# Conditional EQ: Branch if zero (if(ALC==0)PCL=D)
label('EQ')
bne(d(lo('.cond4')))            #20
beq(d(lo('.cond5')))            #21
ld(busRAM|eaYXregAC)            #22

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
ld(busAC,regX)                  #10 (overlap with LDI)
ldzp(d(vAC))                    #11
st(d(vAC),busAC|ea0XregAC)      #12
ld(val(-16/2))                  #13
bra(d(lo('NEXT')))              #14
#nop()                          #(15)
#
# Instruction POP, (AC=[SP++]), 22 cycles
label('POP')
ld(d(vSP),busRAM|regX)          #10 (overlap with ST)
ld(busRAM,ea0XregAC)            #11
st(d(vAC))                      #12
ldzp(d(vSP))                    #13
adda(val(1),regX)               #14
ld(busRAM,ea0XregAC)            #15
st(d(vAC+1))                    #16
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
beq(d(lo('.cond4')))            #20 (overlap with POP)
bne(d(lo('.cond5')))            #21
ld(busRAM|eaYXregAC)            #22

# Instruction LOOKUP, (AC=ROM[AC+256*D]), 24 cycles
label('LOOKUP')
adda(d(vAC+1),busRAM|regY)      #10
jmpy(d(251));                   C('Trampoline offset')#11
ldzp(d(vAC))                    #12
label('rLookup')
st(d(vAC))                      #19
ld(val(0))                      #20
st(d(vAC+1))                    #21
bra(d(lo('NEXT')))              #22
ld(val(-24/2))                  #23

# Instruction PUSH, ([--SP]=RT), 14 cycles
label('PUSH')
ldzp(d(vSP))                    #10
suba(d(1),regX)                 #11
ldzp(d(vRT+1))                  #12
st(ea0XregAC)                   #13
ldzp(d(vSP))                    #14
suba(val(2))                    #15
st(d(vSP),busAC|regX)           #16
ldzp(d(vRT))                    #17
bra(d(lo('next1')))             #18
st(ea0XregAC)                   #19

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
# Instruction POKE ([[D+1],[D]]=ACL), 22 cycles
label('POKE')
st(d(vTmp))                     #10 (overlap with BRA)
adda(val(1),regX)               #11
ld(busRAM,ea0XregAC)            #12
ld(busAC,regY)                  #13
ld(d(vTmp),busRAM|regX)         #14
ld(busRAM,ea0XregAC)            #15
ld(busAC,regX)                  #16
ldzp(d(vAC))                    #17
st(eaYXregAC)                   #18
ld(d(vPC+1),busRAM|regY)        #19
bra(d(lo('NEXT')))              #20
ld(val(-22/2))                  #21

# Instruction SYS, Native function call, <=256 cycles (<=128 ticks, in reality less)
#
# The 'SYS' vCPU instruction first checks the number of desired ticks given by
# the operand. As long as there are insufficient ticks available in the current
# time slice, the instruction will be retried. This will effectively wait for the
# next scanline if the current slice is almost out of time. Then a jump to native
# code is made. This code can do whatever it wants, but it must return to the
# 'REENTER' label when done. When returning, AC must hold (the negative of) the
# actual consumed number of whole ticks for the entire virtual instruction cycle 
# (from NEXT to NEXT). This duration may not exceed the prior declared duration
# in the operand + 28 (or maxTicks). The operand specifies the (negative) of the
# maximum number of EXTRA ticks that the native call will need. The GCL compiler
# automatically makes this calculation from gross number of cycles to excess
# number of ticks.
label('retry')
ldzp(d(vPC));                   C('Retry until sufficient time')#13
suba(val(2))                    #14
st(d(vPC))                      #15
bra(d(lo('REENTER')))           #16
ld(val(-20/2))                  #17
label('SYS')
adda(d(vTicks),busRAM)          #10
blt(d(lo('retry')))             #11
ld(d(vAC+1),busRAM|regY)        #12
jmpy(d(vAC)|busRAM)             #13
#nop()                          #(14)
#
# Instruction SUBW: Word subtraction with zero page (AC-=[D]+256*[D+1]), 28 cycles
# All cases can be done in 26 cycles, but the code will become much larger
label('SUBW')
ld(busAC,regX)                  #10 (overlap with SYS) Address of low byte to be subtracted
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
bra(d(lo('NEXT')))              #26
#nop()                          #(27)
#
# Instruction ADDW: Word addition with zero page (AC+=[D]+256*[D+1]), 28 cycles
label('ADDW')
# The non-carry paths could be 26 cycles at the expense of (much) more code.
# But a smaller size is better so more instructions fit in this code page.
# 28 cycles is still 4.5 usec. The 6502 equivalent takes 20 cycles or 20 usec.
ld(busAC,regX)                  #10 (overlap with SUBW) Address of low byte to be added
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

# Instruction PEEK (AC=[AC]), 22 cycles
label('PEEK')
ldzp(d(vPC))                    #10
suba(val(1))                    #11
st(d(vPC))                      #12
ld(d(vAC),busRAM|regX)          #13
ld(d(vAC+1),busRAM|regY)        #14
ld(busRAM|eaYXregAC)            #15
st(d(vAC))                      #16
ld(val(0))                      #17
st(d(vAC+1))                    #18
ld(val(-22/2))                  #19
label('REENTER')
bra(d(lo('NEXT')));             C('Return from SYS calls')#20
ld(d(vPC+1),busRAM|regY)        #21

# Instruction CALL, (RT=PC+1, PC=AC-2), 22 cycles
label('CALL')
ldzp(d(vPC))                    #10
adda(val(1));                   C('CALL has no operand, advances PC by 1')#11
st(d(vRT))                      #12
ldzp(d(vAC))                    #13
suba(val(2));                   C('vAC is actual address, NEXT adds 2')#14
st(d(vPC))                      #15
ldzp(d(vPC+1))                  #16
st(d(vRT+1))                    #17
ldzp(d(vAC+1))                  #18
st(d(vPC+1),busAC|regY)         #19
bra(d(lo('NEXT')))              #20
ld(val(-22/2))                  #21

# Instruction ADDI, Add small positive constant (AC+=D), 28 cycles
label('ADDI')
ld(val(hi('addi')),regY)        #10
jmpy(d(lo('addi')))             #11
st(d(vTmp))                     #12

# Instruction SUBI, Subtract small positive constant (AC+=D), 28 cycles
label('SUBI')
ld(val(hi('subi')),regY)        #10
jmpy(d(lo('subi')))             #11
st(d(vTmp))                     #12

# Instruction INC, Increment zero page byte ([D]++), 16 cycles
label('INC')
ld(busAC,regX)                  #10
ld(busRAM,ea0XregAC)            #11
adda(val(1))                    #12
st(ea0XregAC)                   #13
bra(d(lo('NEXT')))              #14
ld(val(-16/2))                  #15

# Instruction DEF, Define data or code (AC,PCL=PC+2,D), 18 cycles
label('DEF')
ld(val(hi('def')),regY)         #10
jmpy(d(lo('def')))              #11
st(d(vTmp))                     #12

# Instruction RET, To be defined, 16 cycles
label('RET')
ldzp(d(vRT))                    #10
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
ldzp(d(vRT+1))                  #13
st(d(vPC+1))                    #14
ld(val(hi('REENTER')),regY)     #15
jmpy(d(lo('REENTER')))          #17
ld(val(-20/2))                  #18

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

#-----------------------------------------------------------------------
#
#  vCPU extension functions (for acceleration and compaction) follow below.
#
#  The naming convention is: SYS_<N>_<ALLCAPS>
#
#  With <N> the maximum number of cycles the function will run
#  (counted from NEXT to NEXT). This is the same number that must
#  be passed to the 'SYS' vCPU instruction as operand, and it will
#  appear in the GCL code upon use.
#
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Extension SYS_38_VCLEAR8: Zero a vertical slice of 8 bytes(pixels)
#-----------------------------------------------------------------------

label('SYS_38_VCLEAR8')
ld(d(args+0),busRAM|regX)       #15
ldzp(d(args+1))                 #16
for i in range(8):
  adda(val(i),regY)             #17+2i
  st(d(0),eaYXregAC)            #18+2i
ld(val(hi('REENTER')),regY)     #33
jmpy(d(lo('REENTER')))          #34
ld(val(-38/2))                  #35

#-----------------------------------------------------------------------
# Extension SYS_34_RANDOM: Update entropy and copy to vAC
#-----------------------------------------------------------------------

# This same algorithm runs automatically once per vertical blank.
# Use this function to get numbers at a higher rate.
label('SYS_34_RANDOM')
ldzp(d(frameCount))             #15
xora(d(entropy+1),busRAM)       #16
xora(d(serialInput),busRAM)     #17
adda(d(entropy+0),busRAM)       #18
st(d(entropy+0))                #19
st(d(vAC+0))                    #20
adda(d(entropy+2),busRAM)       #21
st(d(entropy+2))                #22
bmi(d(lo('.sys_rnd0')))         #23
bra(d(lo('.sys_rnd1')))         #24
xora(val(64+16+2+1))            #25
label('.sys_rnd0')
xora(val(64+32+8+4))            #25
label('.sys_rnd1')
adda(d(entropy+1),busRAM)       #26
st(d(entropy+1))                #27
st(d(vAC+1))                    #28
ld(val(hi('REENTER')),regY)     #29
jmpy(d(lo('REENTER')))          #30
ld(val(-34/2))                  #31

#-----------------------------------------------------------------------
#
#  ROM page 5-6: Gigatron font data
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
#  ROM page XX: Images
#-----------------------------------------------------------------------

def importImage(rgbName, width, height, ref):
  f = open(rgbName)
  raw = f.read()
  f.close()
  for y in range(height):
    align(0x100, 0x100)
    label('%s%d' % (ref, y))
    for x in range(width):
      R = ord(raw[3 * (y * width + x) + 0])+43
      G = ord(raw[3 * (y * width + x) + 1])+43
      B = ord(raw[3 * (y * width + x) + 2])+43
      byte = (R/85) + 4*(G/85) + 16*(B/85)
      ld(val(byte))
    trampoline()

importImage('Images/Jupiter-160x120.rgb', 160, 120, 'jupiter')
importImage('Images/Parrot-160x120.rgb',  160, 120, 'parrot')

#-----------------------------------------------------------------------
#
#  ROM page XX: Bootstrap vCPU
#
#-----------------------------------------------------------------------

align(0x100)
label('initVcpu')

# For info
print 'SYS warning %s error %s' % (repr(minSYS), repr(maxSYS))

# Compile test GCL program
program = gcl.Program(bStart)
for line in open('main.gcl').readlines():
  program.line(line)
program.end()
bLine = program.vPC
print bLine-bStart, 'GCL bytes loaded'
print bTop-bLine+1, 'GCL bytes free'

# Set start of user program ready to run
ld(val((bStart&255)-2))
st(d(vPC))
ld(val(bStart>>8))
st(d(vPC+1))

# Reset vCPU stack pointer
ld(val(0))
st(d(vSP))

# Preload vRT with address of next page, for easier setup section in GCL
#ld(val(0))
st(d(vRT))
ld(val((bStart>>8)+1))
st(d(vRT+1))

# Return
ld(d(returnTo+1), busRAM|ea0DregY)
jmpy(d(returnTo+0)| busRAM)
nop()

#-----------------------------------------------------------------------
# Finish assembly
#-----------------------------------------------------------------------
end()
