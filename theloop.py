#!/usr/bin/env python
#-----------------------------------------------------------------------
#
#  Gigatron: a single-board microcomputer from TTL
#
#  Core video, sound and interpreter loop for Gigatron TTL color computer
#  - 6.25MHz clock.
#  - Rendering 160x120 pixels at 6.25MHz with flexible videoline programming
#  - Must stay above 31 kHz horizontal sync --> 200 cycles/scanline
#  - Must stay above 59.94 Hz vertical sync --> 521 scanlines/frame
#  - 4 channels sound
#  TODO: add scanner lights
#  TODO: add screen font
#  TODO: add date/time clock
#  TODO: add scanline mode ("turbo/retro")
#  TODO: add controller input capture
#
#-----------------------------------------------------------------------

from asm import *

# Output pin assignment for VGA
R, G, B, hSync, vSync = 1, 4, 16, 64, 128
syncBits = hSync + vSync # Both pulses negative
#sync = 0             # Both pulses positive

# When the XOUT register is in the circuit, the rising edge triggers its update.
# The loop can therefore not be agnostic to the horizontal pulse polarity.
assert(syncBits & hSync != 0)

vFront = 10
vPulse = 2
vBack  = 33

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
leds      = zpByte()

# Visible video
screenY   = zpByte() # Counts up from 0 to 238 in steps of 2
frameX    = zpByte() # Starting byte within page
frameY    = zpByte() # Page number of current pixel row (updated by videoA)
nextVideo = zpByte()

# Vertical blank, reuse some variables
blankY    = screenY  # Counts down during vertical blank (44 to 0)
videoSync0 = frameX  # Vertical sync type on current line (0xc0 or 0x40)
videoSync1 = frameY  # Same during horizontal pulse

# Function image return address
retImage  = zpByte(2)

# Blinking LED functionality
ledCount  = zpByte()
ledRomPage = zpByte()

# Two bytes of havested entropy
# TODO larger entropy buffer
entropy   = zpByte(2)

#videoMode = zpByte()
#time0     = zpByte() # 1/60 seconds
#time1     = zpByte() # 1 seconds
#time2     = zpByte() # 256 seconds (4 minutes)
#time3     = zpByte() # 2^16 seconds (18 hours)

# All bytes above, except 0x80, are free for temporary/scratch/stacks etc
zpFree    = zpByte()

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
#  Memory layout
#-----------------------------------------------------------------------

screenPages   = 0x80 - 120 # Default start of screen memory: 0x0800 to 0x7fff

#-----------------------------------------------------------------------
#  Application definitions
#-----------------------------------------------------------------------

imageUp=5    # Shift of source image
scrollerY=11 # Area for scroller
ledY=17      # Position for LED on screen

#-----------------------------------------------------------------------
#
#  ROM page 0: boot and vertical blank
#
#-----------------------------------------------------------------------

# Give a first sign of life that can be checked with a voltmeter
ld(val(0))                      # [oooo]
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
ld(val(1))                      # [*ooo]
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
ld(val(1+2))                    # [**oo]
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
st(val(zeroByte), ea0DregAC|busD) # [0] = 0
ld(val(1))
st(val(oneByte), ea0DregAC|busAC) # [0x80] = 1

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

# Init the shift2-right table for sound
ld(val(shiftTablePage), regY)
ld(val(0))
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

# Update LEDs (low pages are initialized)
ld(val(1+2+4))                  # [***o]
ld(val(syncBits^hSync), regOUT)
ld(val(syncBits), regOUT)

# Draw image (and subroutine test)
ld(val(lo('.retImage')))
st(d(retImage+0))
ld(val(hi('.retImage')))
st(d(retImage+1))
ld(d(hi('image')), regY)
jmpy(d(lo('image')))
label('.retImage')

# Update LEDs (subroutines are working)
ld(val(1+2+4+8))                # [****]
ld(val(syncBits^hSync), regOUT)
ld(val(syncBits), regOUT)
st(d(leds)) # Setup for control by video loop

#-----------------------------------------------------------------------
#  Vertical blank part of video loop
#-----------------------------------------------------------------------

# Enter vertical blank
ld(val(syncBits))

label('videoLoop')
st(d(videoSync0))               #32
ld(val(syncBits^hSync))         #33
st(d(videoSync1))               #34

nop()                           #35
nop()                           #36
nop()                           #37

# --- Scroll demo hack app
ld(val(scanTablePage), regY)    #38 # Left 1 pixel/frame
ld(d(0*2+1), busRAM|eaYDregAC)  #39
adda(val(1))                    #40
st(d(0*2+1), busAC|eaYDregAC)   #41
#
bne(d(lo('tick')))              #42 # Shift 1 pixel with every full cycle for ALU test
ld(d(60*2+1), busRAM|eaYDregAC) #43
suba(val(1))                    #44
bra(d(lo('tock')))              #45
st(d(60*2+1), busAC|eaYDregAC)  #46
label('tick')
nop()                           #44
nop()                           #45
nop()                           #46
label('tock')
#
ld(d(scrollerY*2+1), busRAM|eaYDregAC) #47 # Right 1 pixel/frame
suba(val(1))                    #48
st(d(scrollerY*2+1), busAC|eaYDregAC)  #49

# --- Blinking LED
# Do this by switching 2 scanlines and controlling ROM address 15
ld(d(hi('visiblePage')|(0<<7))) #50 # Real LED off
st(d(ledRomPage))               #51
ldzp(d(ledCount))               #52
adda(val(1))                    #53
st(d(ledCount))                 #54 # 60/256 = 1/4 Hz
adda(busAC)                     #55 # 1/2 Hz
adda(busAC)                     #56 # 1 Hz
adda(busAC)                     #57 # 2 Hz
blt(d(lo('led0')))              #58
ld(d(hi('visiblePage')|(1<<7))) #59 # Real LED on
bra(d(lo('led1')))              #60
ld(val(screenPages+ledY))       #61 # Regular image
label('led0')
st(d(ledRomPage))               #60
ld(val(screenPages-2))          #61 # Image with alternative scanlines
label('led1')
ld(d(scanTablePage),regY)       #62
st(d((ledY+0)*2+0),busAC|eaYDregAC)#63
adda(val(1))                    #64
st(d((ledY+1)*2+0),busAC|eaYDregAC)#65
# --- 

wait(198-66)                    #66 # Application (we're still in video scanline 0)

ld(val(vFront+vPulse+vBack-1))  #198
st(d(blankY))                   #199
ld(d(videoSync0), busRAM|regOUT)#0

label('sound1')
ldzp(d(channel))                #1 Hop to next sound channel
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
anda(val(0xfc),regX)            #17
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
beq(d(lo('vBlankLast')))        #30
suba(d(1))                      #31
st(d(blankY))                   #32

# Determine if we're in the vertical sync pulse
suba(d(vBack))                  #33
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
xora(d(hSync))                  #40 # Precompute, during the pulse there is no time for this
st(d(videoSync1))               #41

# Update [xout] with the next sound sample every 4 scan lines.
# Stay on the 'videoC equivalent' scan lines in vertical blank.
ldzp(d(blankY))                 #42
anda(d(3))                      #43
bne(d(lo('vBlankRegular')))     #44
ldzp(d(sample))                 #45
anda(d(0xf0))                   #46
ora(d(leds), busRAM|ea0DregAC)  #47
st(d(xout))                     #48
st(val(sample), ea0DregAC|busD) #49 Reset sample
wait(199-50)                    #50
bra(d(lo('sound1')))            #199
ld(d(videoSync0), busRAM|regOUT)#0 # Ends the vertical blank pulse at the right cycle

label('vBlankRegular')
wait(199-46)                    #46 # Application (video line 1-44)
bne(d(lo('sound1')))            #199
ld(d(videoSync0), busRAM|regOUT)#0 # Ends the vertical blank pulse at the right cycle

# Transfer to visible area
label('vBlankLast')
st(d(frameX))                   #32 AC==0, reset video X offset
st(d(nextVideo))                #33
wait(199-34)                    #34 Application cycles
ldzp(d(channel))                #199 Hop to next sound channel
anda(val(3))                    #0
adda(val(1))                    #1
ld(d(ledRomPage), busRAM|ea0DregY)#2
jmpy(d(lo('sound2')))           #3
ld(val(syncBits^hSync), regOUT) #4 Start horizontal pulse

#-----------------------------------------------------------------------
#
#  ROM page 1: visible part of video loop
#
#-----------------------------------------------------------------------
align(0x100)
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
ld(val(syncBits))                   #39

# Stream 160 pixels from memory location <Yi,Xi> onwards
# Superimpose the sync signal bits to be robust against misprogramming
label('pixels')
for i in range(160):           
  ora(eaYXregOUTIX, busRAM)     #40-199
ld(val(syncBits), regOUT)       #0 Back to black

# Front porch
ldzp(d(channel))           #1 Hop to next sound channel
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
st(d(frameX), busAC|ea0DregX)   #36 # Undocumented opcode "store in RAM and X"!
ld(d(frameY), busRAM|regY)      #37
bra(d(lo('pixels')))            #38
ld(val(syncBits))                   #39

# Back porch C: third of 4 repeated scanlines
# - Nothing new to do, Yi and Xi are known
label('videoC')
# First do something that didn't fit in the audio loop
ldzp(d(sample))                 #29
anda(d(0xf0))                   #30
ora(d(leds), busRAM|ea0DregAC)  #31
st(d(xout))                     #32
st(val(sample), ea0DregAC|busD) #33
# Now back to video business
ld(d(lo('videoD')))             #34
st(d(nextVideo))                #35
ld(d(frameX), busRAM|regX)      #36
ld(d(frameY), busRAM|regY)      #37
bra(d(lo('pixels')))            #38
ld(val(syncBits))                   #39

# Back porch D: last of 4 repeated scanlines
# - Calculate the next frame index
# - Decide if this is the last line or not
label('videoD')
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
label('videoE')
ld(d(hi('videoLoop')),ea0DregY) #29
jmpy(d(lo('videoLoop')))        #30
ld(val(syncBits))               #31

#-----------------------------------------------------------------------
# Selfie raw
#-----------------------------------------------------------------------

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

imageName = 'rrggbb-Selfie-160x120.rgb' # color raw
f = open(imageName)
raw = f.read()
f.close()

# Image
for y in range(scrollerY,120):
  ld(val(0), regX)
  ld(val(screenPages+y), regY)
  for x in range(160):
    v = palette(raw, x, y-imageUp, 160)
    st(val(v), eaYXregOUTIX)

# Alternative lines ("LED ON")
for y in range(2):
  ld(val(0), regX)
  ld(val(screenPages-2+y), regY)
  for x in range(160):
    if 23 <= x <= 25:
       v = 3*R
    else:
       v = palette(raw, x, y+ledY-imageUp, 160)
    st(val(v), eaYXregOUTIX)

ld(d(retImage+1), busRAM|ea0DregY)
jmpy(d(retImage+0)| busRAM)
nop()

#-----------------------------------------------------------------------
# Finish assembly
#-----------------------------------------------------------------------
end()
