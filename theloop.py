#!/usr/bin/env python
#-----------------------------------------------------------------------
#
#  Core video, sound and interpreter loop for Gigatron TTL color computer
#  - 6.25MHz clock
#  - Rendering 160x120 pixels at 6.25MHz with flexible videoline programming
#  - Must stay above 31 kHz horizontal sync --> 200 cycles/scanline
#  - Must stay above 59.94 Hz vertical sync --> 521 scanlines/frame
#  - 4 channels sound
#  TODO: add screen font
#  TODO: add date/time clock
#
#-----------------------------------------------------------------------

from asm import *

# Output pin assignment for VGA
R, G, B, hSync, vSync = 1, 4, 16, 64, 128
syncBits = hSync+vSync # Both pulses negative

# When the XOUT register is in the circuit, the rising edge triggers its update.
# The loop can therefore not be agnostic to the horizontal pulse polarity.
assert(syncBits & hSync != 0)

# VGA defaults
vFront = 10     # Vertical front porch
vPulse = 2      # Vertical sync pulse
vBack = 33      # Vertical back porch
vgaLines = vFront + vPulse + vBack + 480
vgaClock = 25.175e6

# Adjustments for our system:
# 1. Get refresh rate back above minimum 59.92 Hz by cutting lines from vertical front porch
vFrontAdjust = vgaLines - int(4 * 6.25e6 / vgaClock * vgaLines)
vFront -= vFrontAdjust
# 2. Extend vertical sync pulse so we can feed the game controller the same signal
vPulseExtension = max(0, 8-vPulse)
vPulse += vPulseExtension
# 3. Borrow these lines from the back porch so the refresh rate is unaffected
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
buttonNone      = 255
buttonMulti     = 0

#-----------------------------------------------------------------------
#
#  RAM page 0: variables
#
#-----------------------------------------------------------------------

# With [0]=0 and [0x80]=1, "ld [x]" moves bit 7 to bit 0 (carry propagation)
zeroByte = zpByte()
assert zeroByte == 0
oneByte = 0x80

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

# Status of blinkenlights. Keep bit 4:7 clear.
# (Simplest is to keep the sequencer responsible for that)
leds      = zpByte()

# Visible video
screenY   = zpByte() # Counts up from 0 to 238 in steps of 2
frameX    = zpByte() # Starting byte within page
frameY    = zpByte() # Page number of current pixel row (updated by videoA)
nextVideo = zpByte()
videoDorF = zpByte() # Scanline mode

# Vertical blank, reuse some variables
blankY     = screenY  # Counts down during vertical blank (44 to 0)
videoSync0 = frameX  # Vertical sync type on current line (0xc0 or 0x40)
videoSync1 = frameY  # Same during horizontal pulse

# Function image return address
returnTo   = zpByte(2)

# Two bytes of havested entropy
# XXX Consider a larger entropy buffer
entropy    = zpByte(2)

#videoMode = zpByte()
#time0     = zpByte() # 1/60 seconds
#time1     = zpByte() # 1 seconds
#time2     = zpByte() # 256 seconds (4 minutes)
#time3     = zpByte() # 2^16 seconds (18 hours)

ledTimer        = zpByte() # Number of ticks until next LED change
ledState        = zpByte() # Current LED state
ledTempo        = zpByte() # Next value for ledTimer after LED state change
# Fow now the LED state machine itself is hard-coded in the program ROM

# Serial input (game controller)
serialInput     = zpByte()
buttonState     = zpByte() # Filtered button state

# Simple ball demo
ballX           = zpByte()
ballDX          = zpByte()
ballY           = zpByte()
ballDY          = zpByte()
playerX         = zpByte()
wipeOutX        = zpByte()
wipeOutY        = zpByte()
hitX            = zpByte()
hitY            = zpByte()
hitFlag         = zpByte()

# High level interpreter
vPC     = zpByte(2)             # Interpreter program counter (points into RAM)
vAC     = zpByte(2)             # Interpreter accumulator (16-bits)

# 'BASIC' variables
vVarA   = zpByte(2)             # BASIC variable A
vVarB   = zpByte(2)             # BASIC variable B
vVarC   = zpByte(2)             # BASIC variable C
vVarD   = zpByte(2)             # BASIC variable D

# All bytes above, except 0x80, are free for temporary/scratch/stacks etc
zpFree     = zpByte()

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
#  RAM page 3-7: application code 'BASIC'
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

imageUp=5    # Shift of source image
scrollerY=11 # Area for scroller
ledY=17      # Position for LED on screen

gridShiftX=9 # Bricks shift
gridShiftY=scrollerY-4

maxTicks = 28/2 # Duration of slowest virtual opcode (ADDW)
vOverhead = 9 # Overhead of jumping in and out. Cycles, not ticks
def runVcpu(n):
  """Run interpreter for exactly n cycles"""
  print 'runVcpu %s cycles' % n
  if n % 2 != (7 + vOverhead) % 2:
    nop()
    n -= 1
  n -= 7 + 2*maxTicks + vOverhead
  assert n >= 0
  assert n % 2 == 0
  n /= 2
  returnPc = pc() + 7
  ld(val(returnPc&255))         #0
  st(d(returnTo))               #1
  ld(val(returnPc>>8))          #2
  st(d(returnTo+1))             #3
  ld(val(hi('ENTER')),regY)     #4
  jmpy(d(lo('ENTER')))          #5
  print 'runVcpu %s ticks' % n
  ld(val(n))                    #6

#-----------------------------------------------------------------------
#
#  ROM page 0: boot and vertical blank
#
#-----------------------------------------------------------------------

align(0x100, 0x100)

# Give a first sign of life that can be checked with a voltmeter
ld(val(0b0000))                 # Physical: [oooo]
ld(val(syncBits^hSync), regOUT) # Prepare XOUT update, hSync goes down, RGB to black
ld(val(syncBits), regOUT)       # hSync goes up, updating XOUT

# Simple RAM test and size check by writing to [1<<n] and see if [0] changes.
ld(val(1))
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

#ld(val(255))
#label('.debounce')
#st(d(0))
#bne(d(pc()))
#suba(val(1))
#ldzp(d(0))
#bne(d(lo('.debounce')))
#suba(val(1))

# Update LEDs (memory is present and counted, reset is stable)
ld(val(0b0001))                 # Physical: [*ooo]
ld(val(syncBits^hSync), regOUT)
ld(val(syncBits), regOUT)

# Scan the entire RAM space to collect entropy for a random number generator.
# This loop also serves as a debouncing delay for the reset button, if present.
# The 16-bit space is scanned, even if less RAM was detected.
ld(val(0))
st(d(zpFree+0), busAC|ea0DregX)
st(d(zpFree+1), busAC|ea0DregY)
label('.initEnt0')
ldzp(d(entropy+0))
bpl(d(lo('.initEnt1')))
adda(busRAM|eaYXregAC)
adda(val(191))
label('.initEnt1')
st(d(entropy+0))
ldzp(d(entropy+1))
bpl(d(lo('.initEnt2')))
adda(d(entropy+0),busRAM)
adda(val(193))
label('.initEnt2')
st(d(entropy+1))
ldzp(d(zpFree+0))
adda(val(1))
bne(d(lo('.initEnt0')))
st(d(zpFree+0), busAC|ea0DregX)
ldzp(d(zpFree+1))
adda(val(1))
bne(d(lo('.initEnt0')))
st(d(zpFree+1), busAC|ea0DregY)

# Update LEDs (debounce)
ld(val(0b0011))                  # Physical: [**oo]
ld(val(syncBits^hSync), regOUT)
ld(val(syncBits), regOUT)

# Determine if this is a cold or a warm start. We do this by checking the
# boot counter and comparing it to a simplistic checksum. The assumption
# is that after a cold start the checksum is invalid.

ldzp(d(bootCount))
adda(d(bootCheck), busRAM)
adda(d(0x5a))
bne(d(lo('cold')))
ld(val(0))
ldzp(d(bootCount))              # if warm start: bootCount += 1
adda(val(1))
label('cold')
st(d(bootCount))                # if cold start: bootCount = 0
xora(val(255))
suba(val(0x5a-1))
st(d(bootCheck))

# Initialize zeroByte and oneByte
st(val(zeroByte), ea0DregAC|busD)  # Physical: [0] = 0
ld(val(1))
st(val(oneByte), ea0DregAC|busAC)  # Physical: [0x80] = 1

# Initialize scan table for default video layout
ld(val(scanTablePage), regY)
ld(val(0), regX)
ld(val(screenPages))
st(eaYXregOUTIX)         # Yi  = 0x08+i
label('.initVideo')
st(val(0), eaYXregOUTIX) # dXi = 0
adda(val(1))
bge(d(lo('.initVideo'))) # stops at $80
st(eaYXregOUTIX)         # Yi  = 0x08+i
ld(d(lo('videoF')))
st(d(videoDorF))

# Init the shift2-right table for sound
ld(val(shiftTablePage), regY)
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
ld(val(60))
st(d(ledTimer))
ld(val(0))
st(d(ledState))
ld(val(60/5))
st(d(ledTempo))

# Setup a G-major chord to play
G3, G4, B4, D5 = 824, 1648, 2064, 2464

ld(val(1), regY) # Channel 1
ld(val(keyL), regX)
st(d(G3 & 0x7f), eaYXregOUTIX)
st(d(G3 >> 7), eaYXregAC)

ld(val(2), regY) # Channel 2
ld(val(keyL), regX)
st(d(G4 & 0x7f), eaYXregOUTIX)
st(d(G4 >> 7), eaYXregAC)

ld(val(3), regY) # Channel 3
ld(val(keyL), regX)
st(d(B4 & 0x7f), eaYXregOUTIX)
st(d(B4 >> 7), eaYXregAC)

ld(val(4), regY) # Channel 4
ld(val(keyL), regX)
st(d(D5 & 0x7f), eaYXregOUTIX)
st(d(D5 >> 7), eaYXregAC)

# Setup ball
ldzp(d(entropy+0))
anda(val(127))
adda(val(160/2-128/2))
st(d(ballX))
st(d(playerX))
ld(val(127))
st(d(ballY))
ld(val(-1))
st(d(ballDX))
st(d(ballDY))
ld(val(0))
st(d(wipeOutX))
ld(val(screenPages+scrollerY+gridShiftY+7))
st(d(wipeOutY))

# Setup serial input and derived button state
ld(val(-1))
st(d(serialInput))
st(d(buttonState))

# Update LEDs (low pages are initialized)
ld(val(0b0111))                 # Physical: [***o]
ld(val(syncBits^hSync), regOUT)
ld(val(syncBits), regOUT)

# Draw image (and subroutine test)
ld(val(lo('.retImage')))
st(d(returnTo+0))
ld(val(hi('.retImage')))
st(d(returnTo+1))
ld(d(hi('image')), regY)
jmpy(d(lo('image')))
label('.retImage')

# Load test 'BASIC' Fibonacci program into RAM
# There is no parsing, just hand-assembled virtual CPU instructions
#
# 10 A=0
# 20 B=1
# 30 C=A+B
# 40 A=B
# 50 B=C
# 55 IF B>=0 GOTO 30
# 60 D=17480
# 70 POKE D,5
# 80 IF C<0 POKE D,15
# 90 C=C+C
# 100 D=1+D
# 110 IF -17496+D<0 GOTO 70
# 130 GOTO 10

bLine = bStart
ld(val(bLine>>8), regY)
ld(val(bLine&255),regX)

line10 = bLine       # 10 A=0
bLine += 5
st(val(lo('LDWI')),     eaYXregOUTIX)   # LDWI 0
st(val(0),              eaYXregOUTIX)
st(val(0),              eaYXregOUTIX)
st(val(lo('STW')),      eaYXregOUTIX)   # STW 'A'
st(val(vVarA),          eaYXregOUTIX)

line20 = bLine       # 20 B=1
bLine += 5
st(val(lo('LDWI')),     eaYXregOUTIX)   # LDWI 1
st(val(1),              eaYXregOUTIX)
st(val(0),              eaYXregOUTIX)
st(val(lo('STW')),      eaYXregOUTIX)   # STW 'B'
st(val(vVarB),          eaYXregOUTIX)

line30 = bLine       # 30 C=A+B
bLine += 6
st(val(lo('LDW')),      eaYXregOUTIX)   # LDW 'A'
st(val(vVarA),          eaYXregOUTIX)
st(val(lo('ADDW')),     eaYXregOUTIX)   # ADDW 'B'
st(val(vVarB),          eaYXregOUTIX)
st(val(lo('STW')),      eaYXregOUTIX)   # STW 'C'
st(val(vVarC),          eaYXregOUTIX)

line40 = bLine       # 40 A=B
bLine += 4
st(val(lo('LDW')),      eaYXregOUTIX)   # LDW 'B'
st(val(vVarB),          eaYXregOUTIX)
st(val(lo('STW')),      eaYXregOUTIX)   # STW 'A'
st(val(vVarA),          eaYXregOUTIX)

line50 = bLine       # 50 B=C
bLine += 4
st(val(lo('LDW')),      eaYXregOUTIX)   # LDW 'C'
st(val(vVarC),          eaYXregOUTIX)
st(val(lo('STW')),      eaYXregOUTIX)   # STW 'B'
st(val(vVarB),          eaYXregOUTIX)

line55 = bLine       # 55 IF B>=0 GOTO 30
bLine += 8
st(val(lo('LDW')),      eaYXregOUTIX)   # LDW 'B'
st(val(vVarB),          eaYXregOUTIX)
st(val(lo('SIGNW')),    eaYXregOUTIX)   # SIGNW
st(val(lo('BLT')),      eaYXregOUTIX)   # BLT <next>
st(val((bLine&255)-2),  eaYXregOUTIX)
st(val(lo('JUMP')),     eaYXregOUTIX)   # JUMP <line30>
st(val((line30&255)-2), eaYXregOUTIX)
st(val(line30>>8),      eaYXregOUTIX)

line60 = bLine       # 60 D=17480
bLine += 5
st(val(lo('LDWI')),     eaYXregOUTIX)   # LDWI pen
st(val(17480&255),      eaYXregOUTIX)
st(val(17480>>8),       eaYXregOUTIX)
st(val(lo('STW')),      eaYXregOUTIX)   # STW 'D'
st(val(vVarD),          eaYXregOUTIX)

line70 = bLine       # 70 POKE D,5
bLine += 4
st(val(lo('LDW')),      eaYXregOUTIX)   # LDW 'D'
st(val(vVarD),          eaYXregOUTIX)
st(val(lo('POKEI')),    eaYXregOUTIX)   # POKEI 5
st(val(5),              eaYXregOUTIX)

line80 = bLine       # 80 IF C<0 POKE D,15
bLine += 9
st(val(lo('LDW')),      eaYXregOUTIX)   # LDW 'C'
st(val(vVarC),          eaYXregOUTIX)
st(val(lo('SIGNW')),    eaYXregOUTIX)   # SIGNW
st(val(lo('BGE')),      eaYXregOUTIX)   # BGE <next>
st(val((bLine&255)-2),  eaYXregOUTIX)
st(val(lo('LDW')),      eaYXregOUTIX)   # LDW 'D'
st(val(vVarD),          eaYXregOUTIX)
st(val(lo('POKEI')),    eaYXregOUTIX)   # POKEI 15
st(val(15),             eaYXregOUTIX)

line90 = bLine       # 90 C=C+C
bLine += 6
st(val(lo('LDW')),      eaYXregOUTIX)   # LDW 'C'
st(val(vVarC),          eaYXregOUTIX)
st(val(lo('ADDW')),     eaYXregOUTIX)   # ADDW 'C'
st(val(vVarC),          eaYXregOUTIX)
st(val(lo('STW')),      eaYXregOUTIX)   # STW 'C'
st(val(vVarC),          eaYXregOUTIX)

line100 = bLine      # 100 D=1+D
bLine += 7
st(val(lo('LDWI')),     eaYXregOUTIX)   # LDWI 1
st(val(1),              eaYXregOUTIX)
st(val(0),              eaYXregOUTIX)
st(val(lo('ADDW')),     eaYXregOUTIX)   # ADDW 'D'
st(val(vVarD),          eaYXregOUTIX)
st(val(lo('STW')),      eaYXregOUTIX)   # STW 'D'
st(val(vVarD),          eaYXregOUTIX)

line110 = bLine      # 110 IF -17496+D<0 GOTO 70
bLine += 11
st(val(lo('LDWI')),     eaYXregOUTIX)   # LDWI -17496
st(val((-17496)&255),   eaYXregOUTIX)
st(val((-17496)>>8),    eaYXregOUTIX)
st(val(lo('ADDW')),     eaYXregOUTIX)   # ADDW 'D'
st(val(vVarD),          eaYXregOUTIX)
st(val(lo('SIGNW')),    eaYXregOUTIX)   # SIGNW
st(val(lo('BGE')),      eaYXregOUTIX)   # BGE <next>
st(val((bLine&255)-2),  eaYXregOUTIX)
st(val(lo('JUMP')),     eaYXregOUTIX)   # JUMP <line70>
st(val((line70&255)-2), eaYXregOUTIX)
st(val(line70>>8),      eaYXregOUTIX)

line130 = bLine      # 130 GOTO 10
bLine += 3
st(val(lo('JUMP')),     eaYXregOUTIX)   # JUMP <line10>
st(val((line10&255)-2), eaYXregOUTIX)
st(val((line10)>>8),    eaYXregOUTIX)

print bLine-bStart, 'BASIC bytes loaded'
print bTop-bLine+1, 'BASIC bytes free'

# Set start of user program ready to run
ld(val((bStart&255)-2))
st(d(vPC))
ld(val(bStart>>8))
st(d(vPC+1))

# Update LEDs (subroutines are working)
ld(val(0b1111))                 # Physical: [****]
ld(val(syncBits^hSync), regOUT)
ld(val(syncBits), regOUT)
st(d(leds)) # Setup for control by video loop

ld(d(hi('videoLoop')), busD|ea0DregY)
jmpy(d(lo('videoLoop')))
ld(val(syncBits))

#-----------------------------------------------------------------------
#
#  ROM page 1: Vertical blank part of video loop
#
#-----------------------------------------------------------------------
align(0x100, 0x100)
label('videoLoop')              # Enter vertical blank

st(d(videoSync0))               #32
ld(val(syncBits^hSync))         #33
st(d(videoSync1))               #34

# --- Uptime clock

# XXX todo...

# --- LED sequencer (20 cycles)

ldzp(d(ledTimer))               #35
bne(d(lo('.leds4')))            #36

ld(d(lo('.leds0')))             #37
adda(d(ledState)|busRAM)        #38
bra(busAC)                      #39
bra(d(lo('.leds1')))            #40

label('.leds0')
ld(d(0b1111))                   #41
ld(d(0b0111))                   #41
ld(d(0b0011))                   #41
ld(d(0b0001))                   #41
ld(d(0b0010))                   #41
ld(d(0b0100))                   #41
ld(d(0b1000))                   #41
ld(d(0b0100))                   #41
ld(d(0b0010))                   #41
ld(d(0b0001))                   #41
ld(d(0b0011))                   #41
ld(d(0b0111))                   #41
ld(d(0b1111))                   #41
ld(d(0b1110))                   #41
ld(d(0b1100))                   #41
ld(d(0b1000))                   #41
ld(d(0b0100))                   #41
ld(d(0b0010))                   #41
ld(d(0b0001))                   #41
ld(d(0b0010))                   #41
ld(d(0b0100))                   #41
ld(d(0b1000))                   #41
ld(d(0b1100))                   #41
ld(d(0b1110+128))               #41

label('.leds1')
st(d(leds))                     #42 Temporarily park here

bmi(d(lo('.leds2')))            #43
bra(d(lo('.leds3')))            #44
ldzp(d(ledState))               #45
label('.leds2')
ld(val(-1))                     #45
label('.leds3')
adda(val(1))                    #46
st(d(ledState))                 #47

ldzp(d(leds))                   #48 Low 4 bits are the LED output
anda(val(0b00001111))           #49
st(d(leds))                     #50
bra(d(lo('.leds5')))            #51
ldzp(d(ledTempo))               #52 Setup the LED timer for the next period

label('.leds4')
wait(51-38)                     #38
ldzp(d(ledTimer))               #51
suba(d(1))                      #52

label('.leds5')
st(d(ledTimer))                 #53

# --- Scroll demo hack app (7 cycles)

ld(val(scanTablePage), regY)    #54 # Left 1 pixel/frame
ld(d(0*2+1), busRAM|eaYDregAC)  #55
adda(val(1))                    #56
st(d(0*2+1), busAC|eaYDregAC)   #57
ld(d(scrollerY*2+1), busRAM|eaYDregAC)#558 # Right 1 pixel/frame
suba(val(1))                    #59
st(d(scrollerY*2+1), busAC|eaYDregAC)#660

# --- Bouncing ball demo (31 cycles)

# Remove ball
ld(d(ballX), busRAM|ea0DregX)   #61
ld(d(ballY), busRAM|ea0DregY)   #62
st(val(0), eaYXregAC)           #63

# Test hitting vertical walls left and right
extra = 0
ldzp(d(ballX))
adda(d(ballDX),busRAM)
bpl(d(lo('.ball10')))
bra(d(lo('.ball11')))
ld(val(0))
label('.ball10')
ld(val(1))
label('.ball11')
st(d(zpFree),busAC)
extra += 6

ldzp(d(ballX))
adda(d(ballDX),busRAM)
suba(val(160))
bmi(d(lo('.ball12')))
bra(d(lo('.ball13')))
ld(d(zpFree),busRAM)
label('.ball12')
ld(val(1))
label('.ball13')
extra += 6

bne(lo('.ball14'))
ld(val(0))
# Outside the playfield
suba(d(ballDX),busRAM)
bra(lo('.ball15'))
st(d(ballDX),busAC)
# Inside the playfield
label('.ball14')
wait(3) # Resync
label('.ball15')
extra += 5

# Calc hypo X and Y
ldzp(d(ballX))
adda(d(ballDX), busRAM)
st(d(hitX),busAC)
ldzp(d(ballY))
adda(d(ballDY), busRAM)
st(d(hitY),busAC)
assert(hitFlag != 0)
st(d(hitFlag),busD)
extra += 7

# Probe pixel after hypothetical X move
ldzp(d(ballX))                  #64
adda(d(ballDX), busRAM|ea0DregX)#65
ld(busRAM|eaYXregAC)            #66
bne(lo('.ball0'))               #67
ld(val(0))                      #68
# Update X
ldzp(d(ballX))                  #69
adda(d(ballDX), busRAM)         #70
bra(lo('.ball1'))               #71
st(d(ballX),busAC|ea0DregX)     #72
# Register hit and turn around
label('.ball0')
st(d(hitFlag),busAC)            #69
suba(d(ballDX),busRAM)          #70
st(d(ballDX),busAC)             #71
ld(d(ballX),busRAM|ea0DregX)    #72
label('.ball1')
# Test crossing of horizontal walls
ldzp(d(ballY))                  #73
adda(d(ballDY),busRAM)          #74
anda(val(0x7f))                 #75
suba(val(screenPages+scrollerY))#76
bpl(lo('.ball2'))               #77
ld(val(0))                      #78
# Outside the playfield
suba(d(ballDY),busRAM)          #79
bra(lo('.ball3'))               #80
st(d(ballDY),busAC)             #81
# Inside the playfield
label('.ball2')
wait(83-80)                     #79
# Probe pixel after hypothetical Y move
label('.ball3')
ldzp(d(ballY))                  #82
adda(d(ballDY), busRAM|ea0DregY)#83
ld(busRAM|eaYXregAC)            #84
bne(lo('.ball4'))               #85
ld(val(0))                      #86
# Update Y
ldzp(d(ballY))                  #87
adda(d(ballDY), busRAM)         #88
bra(lo('.ball5'))               #89
st(d(ballY),busAC|ea0DregY)     #90
# Register hit and turn around
label('.ball4')
st(d(hitFlag),busAC)            #87
suba(d(ballDY),busRAM)          #88
st(d(ballDY))                   #89
ld(d(ballY),busRAM|ea0DregY)    #90
# Redraw ball
label('.ball5')
st(val(63), eaYXregAC)          #91

ldzp(d(hitFlag))
bne(d(lo('ball6')))
bra(d(lo('ball7')))
ldzp(d(hitX))     # Update wipeout
label('ball6')
ldzp(d(wipeOutX)) # No change
label('ball7')
suba(val(gridShiftX))
anda(val(~15))
adda(val(gridShiftX))
st(d(wipeOutX))
extra += 8

ldzp(d(hitFlag))
bne(d(lo('ball8')))
bra(d(lo('ball9')))
ldzp(d(hitY))     # Update wipeout
label('ball8')
ldzp(d(wipeOutY)) # No change
label('ball9')
suba(val(gridShiftY))
anda(val(~7))
adda(val(gridShiftY))
st(d(wipeOutY))
extra += 8

# When the total number of scanlines per frame is not an exact multiple of the (4) channels,
# there will be an audible discontinuity if no measure is taken. This static noise can be
# suppressed by swallowing the first `lines%4' partial samples after transitioning into
# vertical blank. This is easiest if the modulo is 0 (do nothing) or 1 (reset sample while in
# the first blank scanline). For the two other cases there is no solution yet: give a warning.
soundDiscontinuity = (vFront+vPulse+vBack) % 4
if soundDiscontinuity == 1:
  st(val(sample), ea0DregAC|busD)
  extra += 1
if soundDiscontinuity > 1:
  print "Warning: sound discontinuity not supressed"
wait(198-92-extra)              #92 # XXX Application cycles (scanline 0)

ld(val(vFront+vPulse+vBack-2))  #198 `-2' because first and last are different
st(d(blankY))                   #199
ld(d(videoSync0), busRAM|regOUT)#0

label('sound1')
ldzp(d(channel))                #1 Advance to next sound channel
anda(val(3))                    #2
adda(val(1))                    #3
ld(d(videoSync1), busRAM|regOUT)#4 Start horizontal pulse
st(d(channel), busAC|ea0DregY)  #5
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
anda(val(0b11111100),regX)      #17
ld(d(shiftTablePage), regY)     #18
ld(busRAM|eaYXregAC)            #19
adda(d(sample), busRAM|ea0DregAC)#20
st(d(sample))                   #21
wait(26-22)                     #22
ldzp(d(xout))                   #26
nop()                           #27
ld(d(videoSync0), busRAM|regOUT)#28 End horizontal pulse

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
ldzp(d(blankY))                 #42
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
bne(d(lo('vBlankRegular')))     #49
ldzp(d(sample))                 #50
anda(d(0xf0))                   #51
ora(d(leds), busRAM|ea0DregAC)  #52
st(d(xout))                     #53
st(val(sample), ea0DregAC|busD) #54 Reset for next sample

# Blitter function: Wipe out hit objects
# XXX This runs more frequent than needed
ldzp(d(wipeOutY))
adda(val(6))
label('.wipe')
suba(val(1))
ld(busAC,regY)
ld(d(wipeOutX),busRAM|regX)
for i in range(14):
  st(val(0), eaYXregOUTIX)
xora(d(wipeOutY),busRAM)
bne(d(lo('.wipe')))
xora(d(wipeOutY),busRAM)
wipe = 2+6*(3+14+3)

# Re-redraw the ball
ld(d(ballX), busRAM|ea0DregX)
ld(d(ballY), busRAM|ea0DregY)
st(val(63), eaYXregAC)
wipe += 3

wait(199-55-wipe)               #55 XXX Appplication cycles (scanline 1-43 with sample update)
bra(d(lo('sound1')))            #199
ld(d(videoSync0), busRAM|regOUT)#0 # Ends the vertical blank pulse at the right cycle

label('vBlankRegular')
wait(199-51)                    #51 XXX Application cycles (scanline 1-43 without sample update)
bra(d(lo('sound1')))            #199
ld(d(videoSync0), busRAM|regOUT)#0 Ends the vertical blank pulse at the right cycle

# Last blank line before transfering to visible area
label('vBlankLast0')
ld(val(0))                      #32
st(d(frameX))                   #33
ld(d(hi('vBlankLast1')), busD|ea0DregY)#34
jmpy(d(lo('vBlankLast1')))      #35
st(d(nextVideo))                #36

#-----------------------------------------------------------------------
#
#  ROM page 2: Last invisible line
#
#-----------------------------------------------------------------------

align(0x100, 0x100)
label('vBlankLast1')

# The serial game controller freaks out when two buttons are pressed: it just sends zeroes.
# When we see this, preserve the old value and assume buttonA was added.
ldzp(d(serialInput))            #37
beq(lo('.multi0'))              #38
bra(lo('.multi1'))              #39
st(d(buttonState),busAC)        #40
label('.multi0')
ld(val(buttonA))                #40
label('.multi1')
ora(d(buttonState),busRAM)      #41
st(d(buttonState),busAC)        #42

# --- Respond to button state by moving the player
# Going right
anda(val(buttonRight))          #43
beq(d(lo('.r0')))               #44
bra(d(lo('.r1')))               #45
ld(val(0))                      #46
label('.r0')
ld(val(+1))                     #46
label('.r1')
adda(d(playerX),busRAM)         #47
st(d(playerX),busAC)            #48
# Going left
ldzp(d(buttonState))            #49
anda(val(buttonLeft))           #50
beq(d(lo('.l0')))               #51
bra(d(lo('.l1')))               #52
ld(val(0))                      #53
label('.l0')
ld(val(-1))                     #53
label('.l1')
adda(d(playerX),busRAM)         #54
st(d(playerX),busAC)            #55

# --- Render current serial input as pad near bottom of screen
ld(val(screenPages+115),regY)   #56
ld(d(playerX),busRAM|regX)      #57
st(val(0),eaYXregOUTIX)         #58
ld(val(1))                      #59
label('.N0')
st(d(zpFree))                   #60-116
anda(d(serialInput),busRAM)     #61-117
beq(d(lo('.N1')))               #62-118
bra(d(lo('.N2')))               #63-119
st(val(3*G),eaYXregOUTIX)       #64-120
label('.N1')
st(val(2*G),eaYXregOUTIX)       #64-120
label('.N2')
ldzp(d(zpFree))                 #65-121
bpl(d(lo('.N0')))               #66-122
adda(busAC)                     #67-123
st(val(0),eaYXregOUTIX)         #124

# --- Switch video mode when (only) select is pressed
ldzp(d(buttonState))            #125
xora(val(~buttonSelect))        #126
beq(d(lo('.sel0')))             #127
bra(d(lo('.sel1')))             #128
ld(val(0))                      #129
label('.sel0')
ld(val(lo('videoD')^lo('videoF')))#129
label('.sel1')
xora(d(videoDorF),busRAM)       #130
st(d(videoDorF))                #131

wait(199-132)                   #132 XXX Application cycles (scanline 44)
ldzp(d(channel))                #199 Advance to next sound channel
anda(val(3))                    #0
adda(val(1))                    #1
ld(d(hi('sound2')), busD|ea0DregY)#2
jmpy(d(lo('sound2')))           #3
ld(val(syncBits^hSync), regOUT) #4 Start horizontal pulse

#-----------------------------------------------------------------------
#
#  ROM page 3: Visible part of video loop
#
#-----------------------------------------------------------------------
align(0x100, 0x100)
label('visiblePage')

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
st(eaYXregOUTIX)                #34 # Just to increment X
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
ld(val(syncBits), regOUT)       #0 Back to black

# Front porch
ldzp(d(channel))                #1 Advance to next sound channel
label('soundF')
anda(val(3))                    #2
adda(val(1))                    #3
ld(val(syncBits^hSync), regOUT) #4 Start horizontal pulse

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
ld(d(shiftTablePage), regY)     #18
ld(busRAM|eaYXregAC)            #19
adda(d(sample), busRAM|ea0DregAC)#20
st(d(sample))                   #21
wait(26-22)                     #22
ldzp(d(xout))                   #26
bra(d(nextVideo) | busRAM)      #27
ld(val(syncBits), regOUT)       #28 End horizontal pulse

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
ldzp(d(sample))                 #29 First something that didn't fit in the audio loop
anda(d(0xf0))                   #30
ora(d(leds), busRAM|ea0DregAC)  #31
st(d(xout))                     #32 Update [xout] with new sample (4 channels just updated)
st(val(sample), ea0DregAC|busD) #33 Reset for next sample
ldzp(d(videoDorF))              #34 Now back to video business
st(d(nextVideo))                #35
ld(d(frameX), busRAM|regX)      #36
ld(d(frameY), busRAM|regY)      #37
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
adda(d(120*2))                  #34 # More pixel lines to go
st(d(screenY))                  #35
ld(d(lo('videoA')))             #36
st(d(nextVideo))                #37
bra(d(lo('pixels')))            #38
ld(val(syncBits))               #39
label('last')
wait(36-34)                     #34 # No more pixel lines
ld(d(lo('videoE')))             #36
st(d(nextVideo))                #37
bra(d(lo('pixels')))            #38
ld(val(syncBits))               #39

# Back porch "E": after the last line
# - Go back to program page 0 and enter vertical blank
label('videoE') # Exit visible area
ld(d(hi('videoLoop')),ea0DregY) #29
jmpy(d(lo('videoLoop')))        #30
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
runVcpu(199-36)                 #36 Application (every 4th of scanlines 45-524)
ld(d(hi('soundF')), busD|ea0DregY)#199
jmpy(d(lo('soundF')))           #0
ldzp(d(channel))                #1 Advance to next sound channel

#-----------------------------------------------------------------------
#
#  ROM page 4: Application interpreter
#
#-----------------------------------------------------------------------
align(0x100,0x100)
#-----------------------------------------------------------------------

vTicks  = zpFree                # Interpreter ticks are units of 2 clocks
vTmp    = zpFree+1

#-----------------------------------------------------------------------

#
# Enter the timing-aware application interpreter (aka virtual CPU)
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
ld(d(vPC+1),busRAM|regY)        #1

label('next14')
st(d(vAC))                      #14
ld(val(-16/2))                  #15
# Fetch next instruction and execute it, but only if there are sufficient
# ticks left for the slowest instruction.
label('NEXT')
adda(d(vTicks),busRAM)          #0 Actually counting down (AC<0)
blt(d(lo('RETURN')))            #1
label('.next2')
st(d(vTicks))                   #2
ldzp(d(vPC))                    #3 Advance vPC
adda(val(2))                    #4
st(d(vPC),busAC|ea0DregX)       #5
ld(busRAM|eaYXregAC)            #6 Fetch opcode (actually a branch target)
st(eaYXregOUTIX)                #7 Just to increment X
bra(busAC)                      #8 Execute opcode
ld(busRAM|eaYXregAC)            #9 Prefetch operand

# Resync with caller and return
label('RETURN')
adda(val(maxTicks))             #3
bgt(d(pc()))                    #4
suba(val(1))                    #5
ld(d(returnTo+1),busRAM|regY)   #6
jmpy(d(returnTo+0)|busRAM)      #7
nop()                           #8
assert vOverhead == 9

# Instruction LDI: Load immediate constant (ACL=$DD), 14 cycles
label('LDI')
st(d(vAC))                      #10
ld(val(-14/2))                  #11
bra(d(lo('NEXT')))              #12
nop()                           #13

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
# Instruction LD: Load from zero page (ACL=[D]), 16 cycles
label('LD')
ld(busAC,regX)                  #10 (overlap with LDWI)
ldzp(busRAM|ea0XregAC)          #11
bra(d(lo('next14')))            #12
nop()                           #13

# Instruction LDW: Word load from zero page (AC=[D],[D+1]), 20 cycles
label('LDW')
ld(busAC,regX)                  #10
adda(val(1))                    #11
st(d(vTmp))                     #12 Address of high byte
ld(busRAM|ea0XregAC)            #13
st(d(vAC))                      #14
ld(d(vTmp),busRAM|regX)         #15
ld(busRAM|ea0XregAC)            #16
st(d(vAC+1))                    #17
bra(d(lo('NEXT')))              #18
ld(val(-20/2))                  #19

# Instruction STW: Word load from zero page (AC=[D],[D+1]), 20 cycles
label('STW')
ld(busAC,regX)                  #10
adda(val(1))                    #11
st(d(vTmp))                     #12 Address of high byte
ldzp(d(vAC))                    #13
st(ea0XregAC)                   #14
ld(d(vTmp),busRAM|regX)         #15
ldzp(d(vAC+1))                  #16
st(ea0XregAC)                   #17
bra(d(lo('NEXT')))              #18
ld(val(-20/2))                  #19

# Instruction SIGNW: Test signedness of word (), 24 cycles
label('SIGNW')
ldzp(d(vPC))                    #10 Swallow operand
suba(val(1))                    #11
st(d(vPC))                      #12
ldzp(d(vAC+1))                  #13 First inspect high byte ACH
bne(d(lo('.testw3')))           #14
bmi(d(lo('.testw4')))           #15
st(d(vAC+1))                    #16 Clear ACH
ldzp(d(vAC))                    #17 Additionally inspect low byte ACL
bne(d(lo('.testw1')))           #18
bra(d(lo('.testw2')))           #19
label('.testw0')
ld(val(0))                      #20 ACH==0 and ACL==0
label('.testw1')
ld(val(1))                      #20 ACH==0 and ACL!=0
label('.testw2')
st(d(vAC))                      #21
bra(d(lo('NEXT')))              #22
ld(val(-24/2))                  #23
label('.testw3')
ld(val(0))                      #16 ACH>0
bra(d(lo('.testw0')))           #17 testw0 is labeled 20, but from here it is 19
st(d(vAC+1))                    #18
label('.testw4')
ld(val(-1))                     #17 ACH<0
st(d(vAC+1))                    #18
bra(d(lo('.testw2')))           #19
nop()                           #20

# Instruction JUMP
label('JUMP')
st(d(vPC))                      #10
st(eaYXregOUTIX)                #11 Just to increment X
ld(busRAM|eaYXregAC)            #12 Fetch second operand
st(d(vPC+1))                    #13
bra(d(lo('NEXT')))              #14
ld(val(-16/2))                  #15

# Instruction BEQ: Branch if zero (if(ALC==0)PCL=D), 16 cycles
label('BEQ')
ldzp(d(vAC))                    #10
bne(d(lo('br1')))               #11
ld(busRAM|eaYXregAC)            #12
label('br0')
st(d(vPC))                      #13
bra(d(lo('NEXT')))              #14
#ld(val(-16/2))                 #(15)
label('br1')
ld(val(-16/2))                  #13
bra(d(lo('NEXT')))              #14
#nop()                          #(15)
#
# Instruction ST: Store in zero page ([D]=ACL), 16 cycles
label('ST')
ld(busAC,regX)                  #10 (overlap with BEQ)
ldzp(d(vAC))                    #11
bra(d(lo('next14')))            #12
st(d(vAC),busAC|ea0XregAC)      #13

# Instruction BNE: Branch if not zero (if(ALC!=0)PCL=D), 16 cycles
label('BNE')
ldzp(d(vAC))                    #10
bne(d(lo('br0')))               #11
ld(busRAM|eaYXregAC)            #12
ld(val(-16/2))                  #13
bra(d(lo('NEXT')))              #14
#nop()                          #(15)
#
# Instruction AND: Logical-AND with zero page (ACL&=[D]), 16 cycles
label('AND')
ld(busAC,regX)                  #10 (overlap with BNE)
ldzp(d(vAC))                    #11
bra(d(lo('next14')))            #12
anda(busRAM,ea0XregAC)          #13

# Instruction BGT: Branch if positive (if(ALC>0)PCL=D), 16 cycles
label('BGT')
ldzp(d(vAC))                    #10
bgt(d(lo('br0')))               #11
ld(busRAM|eaYXregAC)            #12
ld(val(-16/2))                  #13
bra(d(lo('NEXT')))              #14
#nop()                          #(15)
#
# Instruction OR: Logical-OR with zero page (ACL|=[D]), 16 cycles
label('OR')
ld(busAC,regX)                  #10 (overlap with BGT)
ldzp(d(vAC))                    #11
bra(d(lo('next14')))            #12
ora(busRAM,ea0XregAC)           #13

# Instruction BLT: Branch if negative (if(ALC<0)PCL=D), 16 cycles
label('BLT')
ldzp(d(vAC))                    #10
blt(d(lo('br0')))               #11
ld(busRAM|eaYXregAC)            #12
ld(val(-16/2))                  #13
bra(d(lo('NEXT')))              #14
#nop()                          #(15)
#
# Instruction XOR: Logical-XOR with zero page (ACL^=[D]), 16 cycles
label('XOR')
ld(busAC,regX)                  #10 (overlap with BLT)
ldzp(d(vAC))                    #11
bra(d(lo('next14')))            #12
xora(busRAM,ea0XregAC)          #13

# Instruction ADDI: Add immediate (ACL+=D), 14 cycles
label('ADDI')
adda(d(vAC),busRAM)             #10
st(d(vAC))                      #11
bra(d(lo('NEXT')))              #12
ld(val(-14/2))                  #13

# Instruction BGE: Branch if positive or zero (if(ALC>=0)PCL=D), 16 cycles
label('BGE')
ldzp(d(vAC))                    #10
bge(d(lo('br0')))               #11
ld(busRAM|eaYXregAC)            #12
ld(val(-16/2))                  #13
bra(d(lo('NEXT')))              #14
#nop()                          #(15)
#
# Instruction ADD: Addition with zero page (ACL+=[D]), 16 cycles
label('ADD')
ld(busAC,regX)                  #10 (overlap with BGE)
ldzp(d(vAC))                    #11
bra(d(lo('next14')))            #12
adda(busRAM,ea0XregAC)          #13

# Instruction BLE: Branch if negative or zero (if(ALC<=0)PCL=D), 16 cycles
label('BLE')
ldzp(d(vAC))                    #10
ble(d(lo('br0')))               #11
ld(busRAM|eaYXregAC)            #12
ld(val(-16/2))                  #13
bra(d(lo('NEXT')))              #14
#nop()                          #(15)
#
# Instruction SUB: Subtraction with zero page (ACL-=[D]), 16 cycles
label('SUB')
ld(busAC,regX)                  #10 (overlap with BLE)
ldzp(d(vAC))                    #11
bra(d(lo('next14')))            #12
suba(busRAM,ea0XregAC)          #13

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
anda(val(0x80),regX)            #20 Move the carry to bit 0
ld(busRAM,ea0XregAC)            #21
adda(d(vAC+1),busRAM)           #22 Add the high bytes
ld(d(vTmp),busRAM|regX)         #23
adda(busRAM|ea0XregAC)          #24
st(d(vAC+1))                    #25 Store high result
bra(d(lo('NEXT')))              #26
ld(val(-28/2))                  #27

# Instruction POKEI
label('POKEI')
ld(d(vAC),busRAM|regX)          #10
ld(d(vAC+1),busRAM|regY)        #11
st(eaYXregAC)                   #12
ld(d(vPC+1),busRAM|regY)        #13
bra(d(lo('NEXT')))              #14
ld(val(-16/2))                  #15

#-----------------------------------------------------------------------
# Selfie raw
#-----------------------------------------------------------------------

align(0x100)
label('image')

def palette(raw, x, y, width):
  r = ord(raw[3 * (y * width + x) + 0])
  g = ord(raw[3 * (y * width + x) + 1])
  b = ord(raw[3 * (y * width + x) + 2])
  return (r/85) + 4*(g/85) + 16*(b/85)

scrollerName = 'Scroller64.rgb' # Scroll text
f = open(scrollerName)
raw = f.read()
f.close()

# Scroller
ld(val(0), regX)
for y in range(scrollerY):
  ld(val(screenPages+y), regY)
  for x in range(256):
    v = palette(raw, x, y, 256)
    st(val(v), eaYXregOUTIX)

colors = [
 63, 43, 38, 18,  3,  2,  1, 17, 16, # ### #++ +-+ +.- #.. +.. -.. -.- ..-
 59, 55, 39, 19,  7,  6,  5,  4, 20, # #+# #-# #-+ #.- #-. +-. --. .-. .--
 54, 51, 35, 23, 11, 10,  9,  8, 24, # +-# #.# #.+ #-- #+. ++. -+. .+. .+-
 50, 34, 22, 27, 15, 14, 13, 12, 25, # +.# +.+ +-- #+- ##. +#. -#. .#. -+-
 49, 33, 21, 26, 31, 30, 29, 28, 41, # -.# -.+ --- ++- ##- +#- -#- .#- -++
 32, 36, 37, 42, 47, 46, 45, 44, 40, # ..+ .-+ --+ +++ ##+ +#+ -#+ .#+ .++
 48, 52, 53, 57, 58, 62, 61, 60, 56, # ..# .-# --# -+# ++# +## -## .## .+#
] # error 1.220485

# Image
for y in range(scrollerY,120):
  ld(val(0), regX)
  ld(val(screenPages+y), regY)
  for x in range(160):
    v = 0
    xm, xd = (x-gridShiftX) % 16, (x-gridShiftX) // 16
    ym, yd = (y-gridShiftY) % 8, (y-gridShiftY) // 8
    if 0<=xd<9 and 2<=yd<9 and xm<14 and ym<6:
       v = colors[(yd-2)*9+xd]
    st(val(v), eaYXregOUTIX)

ld(d(returnTo+1), busRAM|ea0DregY)
jmpy(d(returnTo+0)| busRAM)
nop()

#-----------------------------------------------------------------------
# Finish assembly
#-----------------------------------------------------------------------
end()
