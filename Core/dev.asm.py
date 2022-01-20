#!/usr/bin/env python3
#-----------------------------------------------------------------------
#
#  Core video, sound and interpreter loop for Gigatron TTL microcomputer
#
#-----------------------------------------------------------------------
#
#  Main characteristics:
#
#  - 6.25 MHz clock
#  - Rendering 160x120 pixels at 6.25MHz with flexible videoline programming
#  - Must stay above 31 kHz horizontal sync --> 200 cycles/scanline
#  - Must stay above 59.94 Hz vertical sync --> 521 scanlines/frame
#  - 4 channels sound
#  - 16-bits vCPU interpreter
#  - 8-bits v6502 emulator
#  - Builtin vCPU programs (Snake, Racer, etc) loaded from unused ROM area
#  - Serial input handler, supporting ASCII input and two game controller types
#  - Serial output handler
#  - Soft reset button (keep 'Start' button down for 2 seconds)
#  - Low-level support for I/O and RAM expander (SPI and banking)
#
#-----------------------------------------------------------------------
#
#  ROM v2: Mimimal changes
#
#  DONE Snake color upgrade (just white, still a bit boring)
#  DONE Sound continuity fix
#  DONE A-C- mode (Also A--- added)
#  DONE Zero-page handling of ROM loader (SYS_Exec_88)
#  DONE Replace Screen test
#  DONE LED stopped mode
#  DONE Update font (69;=@Sc)
#  DONE Retire SYS_Reset_36 from all interfaces (replace with vReset)
#  DONE Added SYS_SetMemory_54 SYS_SetVideoMode_80
#  DONE Put in an example BASIC program? Self list, self start
#  DONE Move SYS_NextByteIn_32 out page 1 and rename SYS_LoaderNextByteIn_32
#       Same for SYS_PayloadCopy_34 -> SYS_LoaderPayloadCopy_34
#  DONE Update version number to v2a
#  DONE Test existing GT1 files, in all scan line modes
#  DONE Sanity test on HW
#  DONE Sanity test on several monitors
#  DONE Update version number to v2
#
#-----------------------------------------------------------------------
#
#  ROM v3: New applications
#
#  DONE vPulse width modulation (for SAVE in BASIC)
#  DONE Bricks
#  DONE Tetronis
#  DONE TinyBASIC v2
#  DONE TicTacToe
#  DONE SYS spites/memcpy acceleration functions (reflections etc.)
#  DONE Replace Easter egg
#  DONE Update version number to v3
#
#-----------------------------------------------------------------------
#
#  ROM v4: Numerous small updates, no new applications
#
#  DONE #81 Support alternative game controllers (TypeC added)
#  DONE SPI: Setup SPI at power-on and add 'ctrl' instruction to asm.py
#  DONE SPI: Expander control (Enable/disable slave, set bank etc)
#  DONE SPI: SYS Exchange bytes
#  DONE SYS: Reinitialize waveforms at soft reset, not just at power on
#  DONE v6502: Prototype. Retire bootCount to free up zp variables
#  DONE v6502: Allow soft reset when v6502 is active
#  DONE Apple1: As easter egg, preload with WozMon and Munching6502
#  DONE Apple1: Don't use buttonState but serialRaw
#  DONE Snake: Don't use serialRaw but buttonState
#  DONE Snake: Head-only snake shouldn't be allowed to turn around #52
#  DONE Snake: Improve game play and colors in general
#  DONE Snake: Tweak AI. Also autoplayer can't get hiscore anymore
#  DONE Racer: Don't use serialRaw but buttonState
#  DONE Racer: Faster road setup with SYS_SetMemory
#  DONE Makefile: Pass app-specific SYS functions on command line (.py files)
#  DONE Main: "Press [A] to start": accept keyboard also (incl. 'A') #38
#  DONE Add 4 arrows to font to fill up the ROM page
#  DONE Mode 1975 (for "zombie" mode), can do mode -1 to recover
#  DONE TinyBASIC: support larger font and MODE 1975. Fix indent issue #40
#  DONE Add channelMask to switch off the higher sound channels
#  DONE Loader: clear channelMask when loading into sound channels
#  DONE Update romTypeValue and interface.json
#  DONE Update version number to v4
#  DONE Formally Support SPI and RAM expander: publish in interface.json
#  DONE Use `inspect' to make program listing with original comments #127
#
#-----------------------------------------------------------------------
#
#  Ideas for ROM v5:
#
#  DONE v6502: Test with VTL02
#  DONE v6502: Test with Microchess
#  DONE Sound: Better noise by changing wavX every frame (at least in channel 1)
#  DONE Sound demo: Play SMB Underworld tune
#  DONE SPI: Also reset state at soft reset
#  DONE Fix clobbering of 0x81 by SPI SYS functions #103
#  DONE Control variable to black out the area at the top of the screen
#  DONE Fix possible video timing error in Loader #100
#  DONE Fix zero page usage in Bricks and Tetronis #41
#  DONE Add CALLI instruction to vCPU
#  XXX  Add CMPHS/CMPHU instructions to vCPU XXX Still needs testing
#  DONE Main: add Apple1 to main menu
#  DONE Replace egg with something new
#  DONE Split interface.json and interface-dev.json
#  DONE MSBASIC
#  DONE Speed up SetMemory by 300% using bursts #126
#  DONE Discoverable ROM contents #46
#  DONE Vertical blank interrupt #125
#  DONE TinyBASIC: Support hexadecimal numbers $....
#  XXX  Expander: Auto-detect banking, 64K and 128K -> needs FIX
#  DONE Cardboot: Boot from *.GT1 file if SDC/MMC detected
#  XXX  CardBoot: Strip non-essentials
#  XXX  CardBoot: Fix card type detection
#  XXX  CardBoot: Read full sector
#  DONE Apple-1: Memory mapped PIA emulation using interrupt (D010-D013)
#  DONE Apple-1: Include A1 Integer BASIC
#  DONE Apple-1: Suppress lower case
#  DONE Apple-1: Include Mastermind and 15-Puzzle
#  DONE Apple-1: Include mini-assembler
#  DONE Apple-1: Intercept cassette interface = menu
#  XXX  Reduce the Pictures application ROM footprint #120
#  XXX  Mandelbrot: add more color schemes, eg. with permutations of RGB
#  XXX  Main: Better startup chime, eg. sequence the 4 notes and then decay
#  XXX  Main: Some startup logo as intro, eg. gigatron letters from the box
#  XXX  Racer: Control speed with up/down (better for TypeC controllers)
#  XXX  Racer: Make noise when crashing
#  XXX  Loader: make noise while loading (only channel 1 is safe to use)
#  XXX  Faster SYS_Exec_88, with start address (GT1)?
#  XXX  Let SYS_Exec_88 clear channelMask when loading into live channels
#  XXX  Investigate: Babelfish sometimes freezes during power-on?
#
#  Ideas for ROM v6+
#  XXX  ROM functions: SYS_PrintString, control codes, SYS_DrawChar, SYS_Newline
#  XXX  v8080 prepping for CP/M
#  XXX  vForth virtual CPU
#  XXX  Video: Increase vertical resolution with same videoTable (160 lines?)
#  XXX  Video mode for 12.5 MHz systems
#  XXX  Pucrunch (well documented) or eximozer 3.0.2 (better compression)
#  XXX  SPI: Think about SPI modes (polarities)
#  XXX  I2C: Turn SPI port 2-3 into a I2C port as suggesred by jwolfram
#  XXX  Reset.c and Main.c (that is: port these from GCL to C, but requires LCC fixed)
#  XXX  Need keymaps in ROM? (perhaps undocumented if not tested)
#  XXX  FrogStroll (not in Contrib/)
#  XXX  How it works memo: brief description of every software function
#  XXX  Adjustable return for LUP trampolines (in case SYS functions need it)
#  XXX  Loader: make noise when data comes in
#  XXX  vCPU: Multiplication (mulShift8?)
#  XXX  vCPU: Interrupts / Task switching (e.g for clock, LED sequencer)
#  XXX  Scroll out the top line of text, or generic vertical scroll SYS call
#  XXX  SYS function for plotting a full character in one go
#  XXX  Multitasking/threading/sleeping (start with date/time clock in GCL)
#-----------------------------------------------------------------------

import importlib
from sys import argv
from os  import getenv

from asm import *
import gcl0x as gcl
import font_v4 as font

enableListing()
#-----------------------------------------------------------------------
#
#  Start of core
#
#-----------------------------------------------------------------------

# Pre-loading the formal interface as a way to get warnings when
# accidentally redefined with a different value
loadBindings('interface.json')
loadBindings('Core/interface-dev.json') # Provisional values for DEVROM

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
# 1. Our clock is (slightly) slower than 1/4th VGA clock. Not all monitors will
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

# Start value of vertical blank counter
videoYline0 = 1-2*(vFront+vPulse+vBack-2)

# Mismatch between video lines and sound channels
soundDiscontinuity = (vFront+vPulse+vBack) % 4

# QQVGA resolution
qqVgaWidth      = 160
qqVgaHeight     = 120

# Game controller bits (actual controllers in kit have negative output)
# +----------------------------------------+
# |       Up                        B*     |
# |  Left    Right               B     A*  |
# |      Down      Select Start     A      |
# +----------------------------------------+ *=Auto fire
buttonRight     = 1
buttonLeft      = 2
buttonDown      = 4
buttonUp        = 8
buttonStart     = 16
buttonSelect    = 32
buttonB         = 64
buttonA         = 128

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
# The difference is not audible. This is fine when the reset/address
# value is low and doesn't overflow with 4 channels added to it.
# There is an alternative, but it requires pull-down diodes on the data bus:
#       st [sample],[sample]
assert 4*63 + sample < 256
# We pin this reset/address value to 3, so `sample' swings from 3 to 255
assert sample == 3

# Former bootCount and bootCheck (<= ROMv3)
zpReserved      = zpByte() # Recycled and still unused. Candidate future uses:
                           # - Video driver high address (for alternative video modes)
                           # - v6502: ADH offset ("MMU")
                           # - v8080: ???
vCpuSelect      = zpByte() # Active interpreter page

# Entropy harvested from SRAM startup and controller input
entropy         = zpByte(3)

# Visible video
videoY          = zpByte() # Counts up from 0 to 238 in steps of 2
                           # Counts up (and is odd) during vertical blank
videoModeB      = zpByte() # Handler for every 2nd line (pixel burst or vCPU)
videoModeC      = zpByte() # Handler for every 3rd line (pixel burst or vCPU)
videoModeD      = zpByte() # Handler for every 4th line (pixel burst or vCPU)

nextVideo       = zpByte() # Jump offset to scan line handler (videoA, B, C...)
videoPulse      = nextVideo # Used for pulse width modulation

# Frame counter is good enough as system clock
frameCount      = zpByte(1)

# Serial input (game controller)
serialRaw       = zpByte() # New raw serial read
serialLast      = zpByte() # Previous serial read
buttonState     = zpByte() # Clearable button state
resetTimer      = zpByte() # After 2 seconds of holding 'Start', do a soft reset
                           # XXX move to page 1 to free up space

# Extended output (blinkenlights in bit 0:3 and audio in bit 4:7). This
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
vReturn         = zpByte()  # Return into video loop (in page of vBlankStart)

# Scratch
frameX          = zpByte() # Starting byte within page
frameY          = zpByte() # Page of current pixel line (updated by videoA)

# Vertical blank (reuse some variables used in the visible part)
videoSync0      = frameX   # Vertical sync type on current line (0xc0 or 0x40)
videoSync1      = frameY   # Same during horizontal pulse (0x80 or 0x00)

# Versioning for GT1 compatibility
# Please refer to Docs/GT1-files.txt for interpreting this variable
romType         = zpByte(1)

# The low 3 bits are repurposed to select the actively updated sound channels.
# Valid bit combinations are:
#  xxxxx011     Default after reset: 4 channels (page 1,2,3,4)
#  xxxxx001     2 channels at double update rate (page 1,2)
#  xxxxx000     1 channel at quadruple update rate (page 1)
# The main application for this is to free up the high bytes of page 2,3,4.
channelMask = symbol('channelMask_v4')
assert romType == channelMask

# SYS function arguments and results/scratch
sysFn           = zpByte(2)
sysArgs         = zpByte(8)

# Play sound if non-zero, count down and stop sound when zero
soundTimer      = zpByte()

# Fow now the LED state machine itself is hard-coded in the program ROM
ledTimer        = zpByte() # Number of ticks until next LED change
ledState_v2     = zpByte() # Current LED state
ledTempo        = zpByte() # Next value for ledTimer after LED state change

# All bytes above, except 0x80, are free for temporary/scratch/stacks etc
userVars        = zpByte(0)

#-----------------------------------------------------------------------
#
#  RAM page 1: video line table
#
#-----------------------------------------------------------------------

# Byte 0-239 define the video lines
videoTable      = 0x0100 # Indirection table: Y[0] dX[0]  ..., Y[119] dX[119]

vReset          = 0x01f0
vIRQ_v5         = 0x01f6
ctrlBits        = 0x01f8
videoTop_v5     = 0x01f9 # Number of skip lines

# Highest bytes are for sound channel variables
wavA = 250      # Waveform modulation with `adda'
wavX = 251      # Waveform modulation with `xora'
keyL = 252      # Frequency low 7 bits (bit7 == 0)
keyH = 253      # Frequency high 8 bits
oscL = 254      # Phase low 7 bits
oscH = 255      # Phase high 8 bits

#-----------------------------------------------------------------------
#  Memory layout
#-----------------------------------------------------------------------

userCode = 0x0200       # Application vCPU code
soundTable = 0x0700     # Wave form tables (doubles as right-shift-2 table)
screenMemory = 0x0800   # Default start of screen memory: 0x0800 to 0x7fff

#-----------------------------------------------------------------------
#  Application definitions
#-----------------------------------------------------------------------

maxTicks = 28//2                 # Duration of vCPU's slowest virtual opcode (ticks)
minTicks = 14//2                 # vcPU's fastest instruction
v6502_maxTicks = 38//2           # Max duration of v6502 processing phase (ticks)

runVcpu_overhead = 5            # Caller overhead (cycles)
vCPU_overhead = 9               # Callee overhead of jumping in and out (cycles)
v6502_overhead = 11             # Callee overhead for v6502 (cycles)

v6502_adjust = (v6502_maxTicks - maxTicks) + (v6502_overhead - vCPU_overhead)//2
assert v6502_adjust >= 0        # v6502's overhead is a bit more than vCPU

def runVcpu(n, ref=None, returnTo=None):
  """Macro to run interpreter for exactly n cycles. Returns 0 in AC.

  - `n' is the number of available Gigatron cycles including overhead.
    This is converted into interpreter ticks and takes into account
    the vCPU calling overheads. A `nop' is inserted when necessary
    for alignment between cycles and ticks.
  - `returnTo' is where program flow continues after return. If not set
     explicitely, it will be the first instruction behind the expansion.
  - If another interpreter than vCPU is active (v6502...), that one
    must adjust for the timing differences, because runVcpu wouldn't know."""

  overhead = runVcpu_overhead + vCPU_overhead
  if returnTo == 0x100:         # Special case for videoZ
    overhead -= 2

  if n is None:
    # (Clumsily) create a maximum time slice, corresponding to a vTicks
    # value of 127 (giving 282 cycles). A higher value doesn't work because
    # then SYS functions that just need 28 cycles (0 excess) won't start.
    n = (127 + maxTicks) * 2 + overhead

  n -= overhead
  assert n > 0

  if n % 2 == 1:
    nop()                       # Tick alignment
    n -= 1
  assert n % 2 == 0

  print('runVcpu at $%04x net cycles %3s info %s' % (pc(), n, ref))

  if returnTo != 0x100:
    if returnTo is None:
      returnTo = pc() + 5       # Next instruction
    ld(lo(returnTo))            #0
    st([vReturn])               #1

  n //= 2
  n -= maxTicks                 # First instruction always runs
  assert n < 128
  assert n >= v6502_adjust

  ld([vCpuSelect],Y)            #2
  jmp(Y,'ENTER')                #3
  ld(n)                         #4
assert runVcpu_overhead ==       5

#-----------------------------------------------------------------------
#       v6502 definitions
#-----------------------------------------------------------------------

# Registers are zero page variables
v6502_PC        = vLR           # Program Counter
v6502_PCL       = vLR+0         # Program Counter Low
v6502_PCH       = vLR+1         # Program Counter High
v6502_S         = vSP           # Stack Pointer (kept as "S+1")
v6502_A         = vAC+0         # Accumulator
v6502_BI        = vAC+1         # B Input Register (used by SBC)
v6502_ADL       = sysArgs+0     # Low Address Register
v6502_ADH       = sysArgs+1     # High Address Register
v6502_IR        = sysArgs+2     # Instruction Register
v6502_P         = sysArgs+3     # Processor Status Register (V flag in bit 7)
v6502_Qz        = sysArgs+4     # Quick Status Register for Z flag
v6502_Qn        = sysArgs+5     # Quick Status Register for N flag
v6502_X         = sysArgs+6     # Index Register X
v6502_Y         = sysArgs+7     # Index Register Y
v6502_Tmp       = vTmp          # Scratch (may be clobbered outside v6502)

# MOS 6502 definitions for P register
v6502_Cflag = 1                 # Carry Flag (unsigned overflow)
v6502_Zflag = 2                 # Zero Flag (all bits zero)
v6502_Iflag = 4                 # Interrupt Enable Flag (1=Disable)
v6502_Dflag = 8                 # Decimal Enable Flag (aka BCD mode, 1=Enable)
v6502_Bflag = 16                # Break (or PHP) Instruction Flag
v6502_Uflag = 32                # Unused (always 1)
v6502_Vflag = 64                # Overflow Flag (signed overflow)
v6502_Nflag = 128               # Negative Flag (bit 7 of result)

# In emulation it is much faster to keep the V flag in bit 7
# This can be corrected when importing/exporting with PHP, PLP, etc
v6502_Vemu = 128

# On overflow:
#       """Overflow is set if two inputs with the same sign produce
#          a result with a different sign. Otherwise it is clear."""
# Formula (without carry/borrow in!):
#       (A ^ (A+B)) & (B ^ (A+B)) & 0x80
# References:
#       http://www.righto.com/2012/12/the-6502-overflow-flag-explained.html
#       http://6502.org/tutorials/vflag.html

# Memory layout
v6502_Stack     = 0x0000        # 0x0100 is already used in the Gigatron
#v6502_NMI      = 0xfffa
#v6502_RESET    = 0xfffc
#v6502_IRQ      = 0xfffe

#-----------------------------------------------------------------------
#
#  $0000 ROM page 0: Boot
#
#-----------------------------------------------------------------------

align(0x100, size=0x80)

# Give a first sign of life that can be checked with a voltmeter
ld(0b0000)                      # LEDs |OOOO|
ld(syncBits^hSync,OUT)          # Prepare XOUT update, hSync goes down, RGB to black
ld(syncBits,OUT)                # hSync goes up, updating XOUT

# Setup I/O and RAM expander
ctrl(0b01111111)                # Reset signal (default state | 0x3)
ctrl(0b01111100)                # Disable SPI slaves, enable RAM, bank 1
#      ^^^^^^^^
#      |||||||`-- SCLK
#      ||||||`--- Not connected
#      |||||`---- /SS0
#      ||||`----- /SS1
#      |||`------ /SS2
#      ||`------- /SS3
#      |`-------- B0
#      `--------- B1
# bit15 --------- MOSI = 0

# Simple RAM test and size check by writing to [1<<n] and see if [0] changes or not.
ld(1)                           # Quick RAM test and count
label('.countMem0')
st([memSize],Y)                 # Store in RAM and load AC in Y
ld(255)
xora([Y,0])                     # Invert value from memory
st([Y,0])                       # Test RAM by writing the new value
st([0])                         # Copy result in [0]
xora([Y,0])                     # Read back and compare if written ok
bne(pc())                       # Loop forever on RAM failure here
ld(255)
xora([Y,0])                     # Invert memory value again
st([Y,0])                       # To restore original value
xora([0])                       # Compare with inverted copy
beq('.countMem1')               # If equal, we wrapped around
ld([memSize])
bra('.countMem0')               # Loop to test next address line
adda(AC)                        # Executes in the branch delay slot!
label('.countMem1')

# Momentarily wait to allow for debouncing of the reset switch by spinning
# roughly 2^15 times at 2 clocks per loop: 6.5ms@10MHz to 10ms@6.3MHz
# Real-world switches normally bounce shorter than that.
# "[...] 16 switches exhibited an average 1557 usec of bouncing, with,
#  as I said, a max of 6200 usec" (From: http://www.ganssle.com/debouncing.htm)
# Relevant for the breadboard version, as the kit doesn't have a reset switch.

ld(255)                         # Debounce reset button
label('.debounce')
st([0])
bne(pc())
suba(1)                         # Branch delay slot
ld([0])
bne('.debounce')
suba(1)                         # Branch delay slot

# Update LEDs (memory is present and counted, reset is stable)
ld(0b0001)                      # LEDs |*OOO|
ld(syncBits^hSync,OUT)
ld(syncBits,OUT)

# Scan the entire RAM space to collect entropy for a random number generator.
# The 16-bit address space is scanned, even if less RAM was detected.
ld(0)                           # Collect entropy from RAM
st([vAC+0],X)
st([vAC+1],Y)
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
st([vAC+0],X)
ld([vAC+1])
adda(1)
bne('.initEnt0')
st([vAC+1],Y)

# Update LEDs
ld(0b0011)                      # LEDs |**OO|
ld(syncBits^hSync,OUT)
ld(syncBits,OUT)

# vCPU reset handler
ld((vReset&255)-2)              # Setup vCPU reset handler
st([vPC])
adda(2,X)
ld(vReset>>8)
st([vPC+1],Y)
st('LDI',             [Y,Xpp])
st('SYS_Reset_88',    [Y,Xpp])
st('STW',             [Y,Xpp])
st(sysFn,             [Y,Xpp])
st('SYS',             [Y,Xpp])  # SYS -> SYS_Reset_88 -> SYS_Exec_88
st(256-88//2+maxTicks,[Y,Xpp])
st(0,                 [Y,Xpp])  # vIRQ_v5: Disable interrupts
st(0,                 [Y,Xpp])  # vIRQ_v5
st(0b11111100,        [Y,Xpp])  # Control register
st(0,                 [Y,Xpp])  # videoTop

ld(hi('ENTER'))                 # Active interpreter (vCPU,v6502) = vCPU
st([vCpuSelect])

ld(255)                         # Setup serial input
st([frameCount])
st([serialRaw])
st([serialLast])
st([buttonState])
st([resetTimer])                # resetTimer<0 when entering Main.gcl

ld(0b0111)                      # LEDs |***O|
ld(syncBits^hSync,OUT)
ld(syncBits,OUT)

ld(0)
st([0])                         # Carry lookup ([0x80] in 1st line of vBlank)
st([channel])
st([soundTimer])

ld(0b1111)                      # LEDs |****|
ld(syncBits^hSync,OUT)
ld(syncBits,OUT)
st([xout])                      # Setup for control by video loop
st([xoutMask])

ld(hi('startVideo'),Y)          # Enter video loop at vertical blank
jmp(Y,'startVideo')
st([ledState_v2])               # Setting to 1..126 means "stopped"

#-----------------------------------------------------------------------
# Extension SYS_Reset_88: Soft reset
#-----------------------------------------------------------------------

# SYS_Reset_88 initiates an immediate Gigatron reset from within the vCPU.
# The reset sequence itself is mostly implemented in GCL by Reset.gcl,
# which must first be loaded into RAM. But as that takes more than 1 scanline,
# some vCPU bootstrapping code gets loaded with SYS_Exec_88.
# !!! This function was REMOVED from interface.json
# !!! Better use vReset as generic entry point for soft reset

# ROM type (see also Docs/GT1-files.txt)
romTypeValue = symbol('romTypeValue_DEVROM')

label('SYS_Reset_88')
assert pc()>>8 == 0
assert (romTypeValue & 7) == 0
ld(romTypeValue)                #15 Set ROM type/version and clear channel mask
st([romType])                   #16
ld(0)                           #17
st([vSP])                       #18 vSP
ld(hi('videoTop_v5'),Y)         #19
st([Y,lo('videoTop_v5')])       #20 Show all 120 pixel lines
st([Y,vIRQ_v5])                 #21 Disable vIRQ dispatch
st([Y,vIRQ_v5+1])               #22
st([soundTimer])                #23 soundTimer
assert userCode&255 == 0
st([vLR])                       #24 vLR
ld(userCode>>8)                 #25
st([vLR+1])                     #26
ld('nopixels')                  #27 Video mode 3 (fast)
st([videoModeB])                #28
st([videoModeC])                #29
st([videoModeD])                #30
ld('SYS_Exec_88')               #31 SYS_Exec_88
st([sysFn])                     #32 High byte (remains) 0
ld('Reset')                     #33 Reset.gt1 from EPROM
st([sysArgs+0])                 #34
ld(hi('Reset'))                 #35
st([sysArgs+1])                 #36
ld([vPC])                       #37 Force second SYS call
suba(2)                         #38
st([vPC])                       #39
# Reset expansion board
ctrl(0b01111111)                #40 Reset signal (default state | 0x3)
ctrl(0b01111100)                #41 Default state.
ld([vTmp])                      #42 Always load after ctrl
# Return to interpreter
ld(hi('REENTER'),Y)             #43
jmp(Y,'REENTER')                #44
ld(-48/2)                       #45

#-----------------------------------------------------------------------
# Placeholders for future SYS functions. This works as a kind of jump
# table. The indirection allows SYS implementations to be moved around
# between ROM versions, at the expense of 2 clock cycles (or 1). When
# the function is not present it just acts as a NOP. Of course, when a
# SYS function must be patched or extended it needs to have budget for
# that in its declared maximum cycle count.
#
# Technically the same goal can be achieved by starting each function
# with 2 nop's, or by overdeclaring their duration in the first place
# (a bit is still wise to do). But this can result in fragmentation
# of future ROM images. The indirection avoids that.
#
# An added advantage of having these in ROM page 0 is that it saves one
# byte when setting sysFn: LDI+STW (4 bytes) instead of LDWI+STW (5 bytes)
#-----------------------------------------------------------------------

align(0x80, size=0x80)
assert pc() == 0x80

ld(hi('REENTER'),Y)             #15 slot 0x80
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0x83
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0x86
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0x89
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0x8c
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0x8f
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0x92
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0x95
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0x98
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0x9b
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0x9e
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xa1
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xa4
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xa7
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xaa
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

#-----------------------------------------------------------------------
# Extension SYS_Exec_88: Load code from ROM into memory and execute it
#-----------------------------------------------------------------------
#
# This loads the vCPU code with consideration of the current vSP
# Used during reset, but also for switching between applications or for
# loading data from ROM from within an application (overlays).
#
# ROM stream format is [<addrH> <addrL> <n&255> n*<byte>]* 0
# on top of lookup tables.
#
# Variables:
#       sysArgs[0:1]    ROM pointer (in)
#       sysArgs[2:3]    RAM pointer (changed)
#       sysArgs[4]      State counter (changed)
#       vLR             vCPU continues here (in)

label('SYS_Exec_88')
ld(hi('sys_Exec'),Y)            #15
jmp(Y,'sys_Exec')               #16
ld(0)                           #17 Address of loader on zero page

#-----------------------------------------------------------------------
# More placeholders for future SYS functions
#-----------------------------------------------------------------------

ld(hi('REENTER'),Y)             #15 slot 0xb0
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xb3
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xb6
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xb9
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xbc
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xbf
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xc2
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xc5
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xc8
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xcb
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xce
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xd1
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xd4
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xd7
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xda
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xdd
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

ld(hi('REENTER'),Y)             #15 slot 0xe0
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

#-----------------------------------------------------------------------
# Extension SYS_OsCall_DEVROM_80
#-----------------------------------------------------------------------

# SYS function for calling OS routines in bank2
# Returns 0x8080 when OS signature is wrong.
# Otherwise returns status code
#
#  vAC: routine number
#  sysArgs0-7: routine arguments

label('SYS_OsCall_DEVROM_80')
ld(hi('sys_OsCall'),Y)          #15 slot 0xe3
jmp(Y,'sys_OsCall')             #16
ld(hi(ctrlBits),Y)              #17

#-----------------------------------------------------------------------
# Extension SYS_ScanMemory_DEVROM_50
#-----------------------------------------------------------------------

# SYS function for searching a byte in a 0 to 256 bytes string
# Returns a pointer to the target if found or zero.
# Doesn't cross page boundaries.
#
# sysArgs[0:1]            Start address
# sysArgs[2], sysArgs[3]  Bytes to locate in the string
# vACL                    Length of the string (0 means 256)

label('SYS_ScanMemory_DEVROM_50')
ld(hi('sys_ScanMemory'),Y)      #15 slot 0xe6
jmp(Y,'sys_ScanMemory')         #16
ld([sysArgs+1],Y)               #17

#-----------------------------------------------------------------------
# Extension SYS_CopyMemory_DEVROM_80
#-----------------------------------------------------------------------

# SYS function for copying 1..256 bytes
#
# sysArgs[0:1]    Destination address
# sysArgs[2:3]    Source address
# vAC[0]          Count (0 means 256)
#
# Doesn't cross page boundaries.
# Overwrites sysArgs[4:7] and vLR.

label('SYS_CopyMemory_DEVROM_80')
ld(hi('sys_CopyMemory'),Y)       # 15 slot 0xe9
jmp(Y, 'sys_CopyMemory')         # 16
ld([vAC])                        # 17

#-----------------------------------------------------------------------
# Extension SYS_CopyMemoryExt_DEVROM_100
#-----------------------------------------------------------------------

# SYS function for copying 1..256 bytes across banks
#
# sysArgs[0:1]  Destination address
# sysArgs[2:3]  Source address
# vAC[0]        Count (0 means 256)
# vAC[1]        Bits 7 and 6 contain the destination bank number,
#               and bits 5 and 4 the source bank number.
#
# Doesn't cross page boundaries.
# Overwrites sysArgs[4:7], vLR, and vTmp.
# Returns -1 in vAC if no expansion card is present.

label('SYS_CopyMemoryExt_DEVROM_100')
ld(hi('sys_CopyMemoryExt'),Y)    # 15 slot 0xec
jmp(Y, 'sys_CopyMemoryExt')      # 16
ld(hi(ctrlBits), Y)              # 17

#-----------------------------------------------------------------------
# Extension SYS_ReadRomDir_v5_80
#-----------------------------------------------------------------------

# Get next entry from ROM file system. Use vAC=0 to get the first entry.

# Variables:
#       vAC             Start address of current entry (inout)
#       sysArgs[0:7]    File name, padded with zeroes (out)

label('SYS_ReadRomDir_v5_80')
ld(hi('sys_ReadRomDir'),Y)      #15
jmp(Y,'sys_ReadRomDir')         #16
ld([vAC+1])                     #17

fillers(until=symbol('SYS_Out_22') & 255)

#-----------------------------------------------------------------------
# Extension SYS_Out_22
#-----------------------------------------------------------------------

# Send byte to output port
#
# Variables:
#       vAC

label('SYS_Out_22')
ld([sysArgs+0],OUT)             #15
nop()                           #16
ld(hi('REENTER'),Y)             #17
jmp(Y,'REENTER')                #18
ld(-22/2)                       #19

#-----------------------------------------------------------------------
# Extension SYS_In_24
#-----------------------------------------------------------------------

# Read a byte from the input port
#
# Variables:
#       vAC

label('SYS_In_24')
st(IN, [vAC])                   #15
ld(0)                           #16
st([vAC+1])                     #17
nop()                           #18
ld(hi('REENTER'),Y)             #19
jmp(Y,'REENTER')                #20
ld(-24/2)                       #21

assert pc()&255 == 0

#-----------------------------------------------------------------------
#
#  $0100 ROM page 1: Video loop vertical blank
#
#-----------------------------------------------------------------------
align(0x100, size=0x100)

# Video off mode (also no sound, serial, timer, blinkenlights, ...).
# For benchmarking purposes. This still has the overhead for the vTicks
# administration, time slice granularity etc.
label('videoZ')
videoZ = pc()
runVcpu(None, '---- novideo', returnTo=videoZ)

label('startVideo')             # (Re)start of video signal from idle state
ld(syncBits)

# Start of vertical blank interval
label('vBlankStart')
st([videoSync0])                #32 Start of vertical blank interval
ld(syncBits^hSync)              #33
st([videoSync1])                #34

# Reset line counter before vCPU can see it
ld(videoYline0)                 #35
st([videoY])                    #36

# Update frame count and [0x80] (4 cycles)
ld(1)                           #37 Reinitialize carry lookup, for robustness
st([0x80])                      #38
adda([frameCount])              #39 Frame counter
st([frameCount])                #40

# Mix entropy (11 cycles)
xora([entropy+1])               #41 Mix entropy
xora([serialRaw])               #42 Mix in serial input
adda([entropy+0])               #43
st([entropy+0])                 #44
adda([entropy+2])               #45 Some hidden state
st([entropy+2])                 #46
bmi(pc()+3)                     #47
bra(pc()+3)                     #48
xora(64+16+2+1)                 #49
xora(64+32+8+4)                 #49(!)
adda([entropy+1])               #50
st([entropy+1])                 #51

# LED sequencer (18 cycles)
ld([ledTimer])                  #52 Blinkenlight sequencer
beq(pc()+3)                     #53
bra(pc()+3)                     #54
suba(1)                         #55
ld([ledTempo])                  #55(!)
st([ledTimer])                  #56
beq(pc()+3)                     #57
bra(pc()+3)                     #58
ld(0)                           #59 Don't advance state
ld(1)                           #59(!) Advance state when timer passes through 0
adda([ledState_v2])             #60
bne(pc()+3)                     #61
bra(pc()+3)                     #62
ld(-24)                         #63 State 0 becomes -24, start of sequence
bgt('.leds#65')                 #63(!) Catch the stopped state (>0)
st([ledState_v2])               #64
adda('.leds#69')                #65
bra(AC)                         #66 Jump to lookup table
bra('.leds#69')                 #67 Single-instruction subroutine

label('.leds#65')
ld(0x0f)                        #65 Maintain stopped state
st([ledState_v2])               #66
bra('.leds#69')                 #67
anda([xoutMask])                #68 Always clear sound bits (this is why AC=0x0f)

ld(0b1111)                      #68 LEDs |****| offset -24 Low 4 bits are the LED output
ld(0b0111)                      #68 LEDs |***O|
ld(0b0011)                      #68 LEDs |**OO|
ld(0b0001)                      #68 LEDs |*OOO|
ld(0b0010)                      #68 LEDs |O*OO|
ld(0b0100)                      #68 LEDs |OO*O|
ld(0b1000)                      #68 LEDs |OOO*|
ld(0b0100)                      #68 LEDs |OO*O|
ld(0b0010)                      #68 LEDs |O*OO|
ld(0b0001)                      #68 LEDs |*OOO|
ld(0b0011)                      #68 LEDs |**OO|
ld(0b0111)                      #68 LEDs |***O|
ld(0b1111)                      #68 LEDs |****|
ld(0b1110)                      #68 LEDs |O***|
ld(0b1100)                      #68 LEDs |OO**|
ld(0b1000)                      #68 LEDs |OOO*|
ld(0b0100)                      #68 LEDs |OO*O|
ld(0b0010)                      #68 LEDs |O*OO|
ld(0b0001)                      #68 LEDs |*OOO|
ld(0b0010)                      #68 LEDs |O*OO|
ld(0b0100)                      #68 LEDs |OO*O|
ld(0b1000)                      #68 LEDs |OOO*|
ld(0b1100)                      #68 LEDs |OO**|
ld(0b1110)                      #68 LEDs |O***| offset -1
label('.leds#69')
st([xoutMask])                  #69 Sound bits will be re-enabled below
ld(vPulse*2)                    #70 vPulse default length when not modulated
st([videoPulse])                #71

# When the total number of scan lines per frame is not an exact multiple of the
# (4) channels, there will be an audible discontinuity if no measure is taken.
# This static noise can be suppressed by swallowing the first `lines mod 4'
# partial samples after transitioning into vertical blank. This is easiest if
# the modulo is 0 (do nothing), 1 (reset sample when entering the last visible
# scan line), or 2 (reset sample while in the first blank scan line). For the
# last case there is no solution yet: give a warning.
extra = 0
if soundDiscontinuity == 2:
  st(sample, [sample])          # Sound continuity
  extra += 1
if soundDiscontinuity > 2:
  highlight('Warning: sound discontinuity not suppressed')

# vCPU interrupt
ld([frameCount])                #72
beq('vBlankFirst#75')           #73

runVcpu(186-74-extra,           #74 Application cycles (scan line 0)
    '---D line 0 no timeout',
    returnTo='vBlankFirst#186')

label('vBlankFirst#75')
ld(hi('vBlankFirst#78'),Y)      #75
jmp(Y,'vBlankFirst#78')         #76
ld(hi(vIRQ_v5),Y)               #77
label('vBlankFirst#186')

# Mitigation for rogue channelMask (3 cycles)
ld([channelMask])               #186 Normalize channelMask, for robustness
anda(0b11111011)                #187
st([channelMask])               #188

# Sound on/off (6 cycles)
ld([soundTimer])                #189 Sound on/off
bne(pc()+3)                     #190
bra(pc()+3)                     #191
ld(0)                           #192 Keeps sound unchanged (should be off here)
ld(0xf0)                        #192(!) Turns sound back on
ora([xoutMask])                 #193
st([xoutMask])                  #194

# Sound timer count down (5 cycles)
ld([soundTimer])                #195 Sound timer
beq(pc()+3)                     #196
bra(pc()+3)                     #197
suba(1)                         #198
ld(0)                           #198
st([soundTimer])                #199

ld([videoSync0],OUT)            #0 <New scan line start>
label('sound1')
ld([channel])                   #1 Advance to next sound channel
anda([channelMask])             #2
adda(1)                         #3
ld([videoSync1],OUT)            #4 Start horizontal pulse
st([channel],Y)                 #5
ld(0x7f)                        #6 Update sound channel
anda([Y,oscL])                  #7
adda([Y,keyL])                  #8
st([Y,oscL])                    #9
anda(0x80,X)                    #10
ld([X])                         #11
adda([Y,oscH])                  #12
adda([Y,keyH])                  #13
st([Y,oscH])                    #14
anda(0xfc)                      #15
xora([Y,wavX])                  #16
ld(AC,X)                        #17
ld([Y,wavA])                    #18
ld(soundTable>>8,Y)             #19
adda([Y,X])                     #20
bmi(pc()+3)                     #21
bra(pc()+3)                     #22
anda(63)                        #23
ld(63)                          #23(!)
adda([sample])                  #24
st([sample])                    #25

ld([xout])                      #26 Gets copied to XOUT
ld(hi('vBlankLast#34'),Y)       #27 Prepare jumping out of page in last line
ld([videoSync0],OUT)            #28 End horizontal pulse

# Count through the vertical blank interval until its last scan line
ld([videoY])                    #29
bpl('.vBlankLast#32')           #30
adda(2)                         #31
st([videoY])                    #32

# Determine if we're in the vertical sync pulse
suba(1-2*(vBack+vPulse-1))      #33 Prepare sync values
bne('.prepSync36')              #34 Tests for start of vPulse
suba([videoPulse])              #35
ld(syncBits^vSync)              #36 Entering vertical sync pulse
bra('.prepSync39')              #37
st([videoSync0])                #38
label('.prepSync36')
bne('.prepSync38')              #36 Tests for end of vPulse
ld(syncBits)                    #37
bra('.prepSync40')              #38 Entering vertical back porch
st([videoSync0])                #39
label('.prepSync38')
ld([videoSync0])                #38 Load current value
label('.prepSync39')
nop()                           #39
label('.prepSync40')
xora(hSync)                     #40 Precompute, as during the pulse there is no time
st([videoSync1])                #41

# Capture the serial input before the '595 shifts it out
ld([videoY])                    #42 Capture serial input
xora(1-2*(vBack-1-1))           #43 Exactly when the 74HC595 has captured all 8 controller bits
bne(pc()+3)                     #44
bra(pc()+3)                     #45
st(IN, [serialRaw])             #46
st(0,[0])                       #46(!) Reinitialize carry lookup, for robustness

# Update [xout] with the next sound sample every 4 scan lines.
# Keep doing this on 'videoC equivalent' scan lines in vertical blank.
ld([videoY])                    #47
anda(6)                         #48
beq('vBlankSample')             #49
ld([sample])                    #50

label('vBlankNormal')
runVcpu(199-51, 'AB-D line 1-36')#51 Application cycles (vBlank scan lines without sound sample update)
bra('sound1')                   #199
ld([videoSync0],OUT)            #0 <New scan line start>

label('vBlankSample')
ora(0x0f)                       #51 New sound sample is ready
anda([xoutMask])                #52
st([xout])                      #53
st(sample, [sample])            #54 Reset for next sample

runVcpu(199-55, '--C- line 3-39')#55 Application cycles (vBlank scan lines with sound sample update)
bra('sound1')                   #199
ld([videoSync0],OUT)            #0 <New scan line start>

#-----------------------------------------------------------------------

label('.vBlankLast#32')
jmp(Y,'vBlankLast#34')          #32 Jump out of page for space reasons
#assert hi(controllerType) == hi(pc()) # Assume these share the high address
ld(hi(pc()),Y)                  #33

label('vBlankLast#52')

# Respond to reset button (14 cycles)
# - ResetTimer decrements as long as just [Start] is pressed down
# - Reaching 0 (normal) or 128 (extended) triggers the soft reset sequence
# - Initial value is 128 (or 255 at boot), first decrement, then check
# - This starts vReset -> SYS_Reset_88 -> SYS_Exec_88 -> Reset.gcl -> Main.gcl
# - Main.gcl then recognizes extended presses if resetTimer is 0..127 ("paasei")
# - This requires a full cycle (4s) in the warm boot scenario
# - Or a half cycle (2s) when pressing [Select] down during hard reset
# - This furthermore requires >=1 frame (and <=128) to have passed between
#   reaching 128 and getting through Reset and the start of Main, while [Start]
#   was still pressed so the count reaches <128. Two reasonable expectations.
# - The unintended power-up scenarios of ROMv1 (pulling SER_DATA low, or
#   pressing [Select] together with another button) now don't trigger anymore.

ld([buttonState])               #52 Check [Start] for soft reset
xora(~buttonStart)              #53
bne('.restart#56')              #54
ld([resetTimer])                #55 As long as button pressed
suba(1)                         #56 ... count down the timer
st([resetTimer])                #57
anda(127)                       #58
beq('.restart#61')              #59 Reset at 0 (normal 2s) or 128 (extended 4s)
ld((vReset&255)-2)              #60 Start force reset when hitting 0
bra('.restart#63')              #61 ... otherwise do nothing yet
bra('.restart#64')              #62
label('.restart#56')
wait(62-56)                     #56
ld(128)                         #62 Not pressed, reset the timer
st([resetTimer])                #63
label('.restart#64')
bra('.restart#66')              #64
label('.restart#63')
nop()                           #63,65
label('.restart#61')
st([vPC])                       #61 Point vPC at vReset
ld(vReset>>8)                   #62
st([vPC+1])                     #63
ld(hi('ENTER'))                 #64 Set active interpreter to vCPU
st([vCpuSelect])                #65
label('.restart#66')

# Switch video mode when (only) select is pressed (16 cycles)
# XXX We could make this a vCPU interrupt
ld([buttonState])               #66 Check [Select] to switch modes
xora(~buttonSelect)             #67 Only trigger when just [Select] is pressed
bne('.select#70')               #68
ld([videoModeC])                #69
bmi('.select#72')               #70 Branch when line C is off
ld([videoModeB])                #71 Rotate: Off->D->B->C
st([videoModeC])                #72
ld([videoModeD])                #73
st([videoModeB])                #74
bra('.select#77')               #75
label('.select#72')
ld('nopixels')                  #72,76
ld('pixels')                    #73 Reset: On->D->B->C
st([videoModeC])                #74
st([videoModeB])                #75
nop()                           #76
label('.select#77')
st([videoModeD])                #77
wait(188-78)                    #78 Don't waste code space expanding runVcpu here
# AC==255 now
st([buttonState])               #188
bra('vBlankEnd#191')            #189
ld(0)                           #190
label('.select#70')

# Mitigation of runaway channel variable
ld([channel])                   #70 Normalize channel, for robustness
anda(0b00000011)                #71
st([channel])                   #72 Stop wild channel updates

runVcpu(191-73, '---D line 40') #73 Application cycles (scan line 40)

# AC==0 now
label('vBlankEnd#191')
ld(videoTop_v5>>8,Y)            #191
ld([Y,videoTop_v5])             #192
st([videoY])                    #193
st([frameX])                    #194
bne(pc()+3)                     #195
bra(pc()+3)                     #196
ld('videoA')                    #197
ld('videoF')                    #197(!)
st([nextVideo])                 #198
ld([channel])                   #199 Advance to next sound channel
anda([channelMask])             #0 <New scan line start>
adda(1)                         #1
ld(hi('sound2'),Y)              #2
jmp(Y,'sound2')                 #3
ld(syncBits^hSync,OUT)          #4 Start horizontal pulse

fillers(until=0xff)

#-----------------------------------------------------------------------
# Return point for vCPU slices during visible screen area
#-----------------------------------------------------------------------

assert pc() == 0x1ff            # Enables runVcpu() to re-enter into the next page
bra('sound3')                   #200,0 <New scan line start>

#-----------------------------------------------------------------------
#
#  $0200 ROM page 2: Video loop visible scanlines
#
#-----------------------------------------------------------------------
align(0x100, size=0x100)
ld([channel])                   #1 Advance to next sound channel

# Back porch A: first of 4 repeated scan lines
# - Fetch next Yi and store it for retrieval in the next scan lines
# - Calculate Xi from dXi, but there is no cycle time left to store it as well
label('videoA')
ld('videoB')                    #29 1st scanline of 4 (always visible)
st([nextVideo])                 #30
ld(videoTable>>8,Y)             #31
ld([videoY],X)                  #32
ld([Y,X])                       #33
st([Y,Xpp])                     #34 Just X++
st([frameY])                    #35
ld([Y,X])                       #36
adda([frameX],X)                #37
label('pixels')
ld([frameY],Y)                  #38
ld(syncBits)                    #39

# Stream 160 pixels from memory location <Yi,Xi> onwards
# Superimpose the sync signal bits to be robust against misprogramming
for i in range(qqVgaWidth):
  ora([Y,Xpp],OUT)              #40-199 Pixel burst
ld(syncBits,OUT)                #0 <New scan line start> Back to black

# Front porch
ld([channel])                   #1 Advance to next sound channel
label('sound3')                 # Return from vCPU interpreter
anda([channelMask])             #2
adda(1)                         #3
ld(syncBits^hSync,OUT)          #4 Start horizontal pulse

# Horizontal sync and sound channel update for scanlines outside vBlank
label('sound2')
st([channel],Y)                 #5
ld(0x7f)                        #6
anda([Y,oscL])                  #7
adda([Y,keyL])                  #8
st([Y,oscL])                    #9
anda(0x80,X)                    #10
ld([X])                         #11
adda([Y,oscH])                  #12
adda([Y,keyH])                  #13
st([Y,oscH] )                   #14
anda(0xfc)                      #15
xora([Y,wavX])                  #16
ld(AC,X)                        #17
ld([Y,wavA])                    #18
ld(soundTable>>8,Y)             #19
adda([Y,X])                     #20
bmi(pc()+3)                     #21
bra(pc()+3)                     #22
anda(63)                        #23
ld(63)                          #23(!)
adda([sample])                  #24
st([sample])                    #25

ld([xout])                      #26 Gets copied to XOUT
bra([nextVideo])                #27
ld(syncBits,OUT)                #28 End horizontal pulse

# Back porch B: second of 4 repeated scan lines
# - Recompute Xi from dXi and store for retrieval in the next scan lines
label('videoB')
ld('videoC')                    #29 2nd scanline of 4
st([nextVideo])                 #30
ld(videoTable>>8,Y)             #31
ld([videoY])                    #32
adda(1,X)                       #33
ld([frameX])                    #34
adda([Y,X])                     #35
bra([videoModeB])               #36
st([frameX],X)                  #37 Store in RAM and X

# Back porch C: third of 4 repeated scan lines
# - Nothing new to for video do as Yi and Xi are known,
# - This is the time to emit and reset the next sound sample
label('videoC')
ld('videoD')                    #29 3rd scanline of 4
st([nextVideo])                 #30
ld([sample])                    #31 New sound sample is ready (didn't fit in the audio loop)
ora(0x0f)                       #32
anda([xoutMask])                #33
st([xout])                      #34 Update [xout] with new sample (4 channels just updated)
st(sample, [sample])            #35 Reset for next sample
bra([videoModeC])               #36
ld([frameX],X)                  #37

# Back porch D: last of 4 repeated scan lines
# - Calculate the next frame index
# - Decide if this is the last line or not
label('videoD')                 # Default video mode
ld([frameX], X)                 #29 4th scanline of 4
ld([videoY])                    #30
suba((120-1)*2)                 #31
beq('.lastpixels#34')           #32
adda(120*2)                     #33 More pixel lines to go
st([videoY])                    #34
ld('videoA')                    #35
bra([videoModeD])               #36
st([nextVideo])                 #37

label('.lastpixels#34')
if soundDiscontinuity == 1:
  st(sample, [sample])          #34 Sound continuity
else:
  nop()                         #34
ld('videoE')                    #35 No more pixel lines to go
bra([videoModeD])               #36
st([nextVideo])                 #37

# Back porch "E": after the last line
# - Go back and and enter vertical blank (program page 2)
label('videoE') # Exit visible area
ld(hi('vBlankStart'),Y)         #29 Return to vertical blank interval
jmp(Y,'vBlankStart')            #30
ld(syncBits)                    #31

# Video mode that blacks out one or more pixel lines from the top of screen.
# This yields some speed, but also frees up screen memory for other purposes.
# Note: Sound output becomes choppier the more pixel lines are skipped
# Note: The vertical blank driver leaves 0x80 behind in [videoSync1]
label('videoF')
ld([videoSync1])                #29 Completely black pixel line
adda(0x80)                      #30
st([videoSync1],X)              #31
ld([frameX])                    #32
suba([X])                       #33 Decrements every two VGA scanlines
beq('.videoF#36')               #34
st([frameX])                    #35
bra('nopixels')                 #36
label('.videoF#36')
ld('videoA')                    #36,37 Transfer to visible screen area
st([nextVideo])                 #37
#
# Alternative for pixel burst: faster application mode
label('nopixels')
runVcpu(200-38, 'ABCD line 40-520',
  returnTo=0x1ff)               #38 Application interpreter (black scanlines)

#-----------------------------------------------------------------------
#
#  $0300 ROM page 3: Application interpreter primary page
#
#-----------------------------------------------------------------------

# Enter the timing-aware application interpreter (aka virtual CPU, vCPU)
#
# This routine will execute as many as possible instructions in the
# allotted time. When time runs out, it synchronizes such that the total
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
# --- Page boundary ---
align(0x100,size=0x100)
label('NEXTY')                  # Alternative for REENTER
ld([vPC+1],Y)                   #1

# Fetch next instruction and execute it, but only if there are sufficient
# ticks left for the slowest instruction.
label('NEXT')
adda([vTicks])                  #0 Track elapsed ticks (actually counting down: AC<0)
blt('EXIT')                     #1 Escape near time out
label('.next2')
st([vTicks])                    #2
ld([vPC])                       #3 Advance vPC
adda(2)                         #4
st([vPC],X)                     #5
ld([Y,X])                       #6 Fetch opcode (actually a branch target)
st([Y,Xpp])                     #7 Just X++
bra(AC)                         #8 Dispatch
ld([Y,X])                       #9 Prefetch operand

# Resync with video driver and transfer control
label('EXIT')
adda(maxTicks)                  #3
label('RESYNC')
bgt(pc()&255)                   #4 Resync
suba(1)                         #5
ld(hi('vBlankStart'),Y)         #6
jmp(Y,[vReturn])                #7 To video driver
ld(0)                           #8 AC should be 0 already. Still..
assert vCPU_overhead ==          9

# Instruction LDWI: Load immediate word constant (vAC=D), 20 cycles
label('LDWI')
st([vAC])                       #10
st([Y,Xpp])                     #11 Just X++
ld([Y,X])                       #12 Fetch second operand
st([vAC+1])                     #13
ld([vPC])                       #14 Advance vPC one more
adda(1)                         #15
st([vPC])                       #16
ld(-20/2)                       #17
bra('NEXT')                     #18
#dummy()                        #19 Overlap
#
# Instruction LD: Load byte from zero page (vAC=[D]), 22 cycles
label('LD')
ld(AC,X)                        #10,19
ld([X])                         #11
ld(hi('ld#15'),Y)               #12
jmp(Y,'ld#15')                  #13
st([vAC])                       #14

# Instruction CMPHS: Adjust high byte for signed compare (vACH=XXX), 28 cycles
label('CMPHS_v5')
ld(hi('cmphs#13'),Y)            #10
jmp(Y,'cmphs#13')               #11
#ld(AC,X)                       #12 Overlap
#
# Instruction LDW: Load word from zero page (vAC=[D]+256*[D+1]), 20 cycles
label('LDW')
ld(AC,X)                        #10,12
adda(1)                         #11
st([vTmp])                      #12 Address of high byte
ld([X])                         #13
st([vAC])                       #14
ld([vTmp],X)                    #15
ld([X])                         #16
st([vAC+1])                     #17
bra('NEXT')                     #18
ld(-20/2)                       #19

# Instruction STW: Store word in zero page ([D],[D+1]=vAC&255,vAC>>8), 20 cycles
label('STW')
ld(AC,X)                        #10,20
adda(1)                         #11
st([vTmp])                      #12 Address of high byte
ld([vAC])                       #13
st([X])                         #14
ld([vTmp],X)                    #15
ld([vAC+1])                     #16
st([X])                         #17
bra('NEXT')                     #18
ld(-20/2)                       #19

# Instruction BCC: Test AC sign and branch conditionally, 28 cycles
label('BCC')
ld([vAC+1])                     #10 First inspect high byte of vAC
bne('.bcc#13')                  #11
st([vTmp])                      #12
ld([vAC])                       #13 Additionally inspect low byte of vAC
beq('.bcc#16')                  #14
ld(1)                           #15
st([vTmp])                      #16
ld([Y,X])                       #17 Operand is the conditional
label('.bcc#18')
bra(AC)                         #18
ld([vTmp])                      #19

# Conditional EQ: Branch if zero (if(vACL==0)vPCL=D)
label('EQ')
bne('.bcc#22')                  #20
label('.bcc#13')
beq('.bcc#23')                  #21,13 AC=0 in EQ, AC!=0 from BCC... Overlap with BCC
ld([Y,X])                       #22,14 Overlap with BCC
#
# (continue BCC)
#label('.bcc#13')
#dummy()                        #13
#dummy()                        #14
nop()                           #15
label('.bcc#16')
bra('.bcc#18')                  #16
ld([Y,X])                       #17 Operand is the conditional
label('.bcc#22')
ld([vPC])                       #22 False condition
bra('.bcc#25')                  #23
adda(1)                         #24
label('.bcc#23')
st([Y,Xpp])                     #23 Just X++ True condition
ld([Y,X])                       #24
label('.bcc#25')
st([vPC])                       #25
bra('NEXT')                     #26
ld(-28/2)                       #27

# Conditional GT: Branch if positive (if(vACL>0)vPCL=D)
label('GT')
ble('.bcc#22')                  #20
bgt('.bcc#23')                  #21
ld([Y,X])                       #22

# Conditional LT: Branch if negative (if(vACL<0)vPCL=D)
label('LT')
bge('.bcc#22')                  #20
blt('.bcc#23')                  #21
ld([Y,X])                       #22

# Conditional GE: Branch if positive or zero (if(vACL>=0)vPCL=D)
label('GE')
blt('.bcc#22')                  #20
bge('.bcc#23')                  #21
ld([Y,X])                       #22

# Conditional LE: Branch if negative or zero (if(vACL<=0)vPCL=D)
label('LE')
bgt('.bcc#22')                  #20
ble('.bcc#23')                  #21
ld([Y,X])                       #22

# Instruction LDI: Load immediate small positive constant (vAC=D), 16 cycles
label('LDI')
st([vAC])                       #10
ld(0)                           #11
st([vAC+1])                     #12
bra('NEXTY')                    #13
ld(-16/2)                       #14

# Instruction ST: Store byte in zero page ([D]=vAC&255), 16 cycles
label('ST')
ld(AC,X)                        #10,15
ld([vAC])                       #11
st([X])                         #12
bra('NEXTY')                    #13
ld(-16/2)                       #14

# Instruction POP: Pop address from stack (vLR,vSP==[vSP]+256*[vSP+1],vSP+2), 26 cycles
label('POP')
ld([vSP],X)                     #10,15
ld([X])                         #11
st([vLR])                       #12
ld([vSP])                       #13
adda(1,X)                       #14
ld([X])                         #15
st([vLR+1])                     #16
ld([vSP])                       #17
adda(2)                         #18
st([vSP])                       #19
label('.pop#20')
ld([vPC])                       #20
suba(1)                         #21
st([vPC])                       #22
bra('NEXTY')                    #23
ld(-26/2)                       #24

# Conditional NE: Branch if not zero (if(vACL!=0)vPCL=D)
label('NE')
beq('.bcc#22')                  #20,25
bne('.bcc#23')                  #21
ld([Y,X])                       #22

# Instruction PUSH: Push vLR on stack ([vSP-2],v[vSP-1],vSP=vLR&255,vLR>>8,vLR-2), 26 cycles
label('PUSH')
ld([vSP])                       #10
suba(1,X)                       #11
ld([vLR+1])                     #12
st([X])                         #13
ld([vSP])                       #14
suba(2)                         #15
st([vSP],X)                     #16
ld([vLR])                       #17
bra('.pop#20')                  #18
st([X])                         #19

# Instruction LUP: ROM lookup (vAC=ROM[vAC+D]), 26 cycles
label('LUP')
ld([vAC+1],Y)                   #10
jmp(Y,251)                      #11 Trampoline offset
adda([vAC])                     #12

# Instruction ANDI: Logical-AND with small constant (vAC&=D), 22 cycles
label('ANDI')
ld(hi('andi#13'),Y)             #10
jmp(Y,'andi#13')                #11
anda([vAC])                     #12

# Instruction CALLI: Goto immediate address and remember vPC (vLR,vPC=vPC+3,$HHLL-2), 28 cycles
label('CALLI_v5')
ld(hi('calli#13'),Y)            #10
jmp(Y,'calli#13')               #11
ld([vPC])                       #12

# Instruction ORI: Logical-OR with small constant (vAC|=D), 14 cycles
label('ORI')
ora([vAC])                      #10
st([vAC])                       #11
bra('NEXT')                     #12
ld(-14/2)                       #13

# Instruction XORI: Logical-XOR with small constant (vAC^=D), 14 cycles
label('XORI')
xora([vAC])                     #10
st([vAC])                       #11
bra('NEXT')                     #12
ld(-14/2)                       #13

# Instruction BRA: Branch unconditionally (vPC=(vPC&0xff00)+D), 14 cycles
label('BRA')
st([vPC])                       #10
bra('NEXTY')                    #11
ld(-14/2)                       #12

# Instruction INC: Increment zero page byte ([D]++), 20 cycles
label('INC')
ld(AC,X)                        #10,13
ld(hi('inc#14'),Y)              #11
jmp(Y,'inc#14')                 #12
ld(1)                           #13

# Instruction CMPHU: Adjust high byte for unsigned compare (vACH=XXX), 28 cycles
label('CMPHU_v5')
ld(hi('cmphu#13'),Y)            #10
jmp(Y,'cmphu#13')               #11
#ld(AC,X)                       #12 Overlap
#
# Instruction ADDW: Word addition with zero page (vAC+=[D]+256*[D+1]), 28 cycles
label('ADDW')
# The non-carry paths could be 26 cycles at the expense of (much) more code.
# But a smaller size is better so more instructions fit in this code page.
# 28 cycles is still 4.5 usec. The 6502 equivalent takes 20 cycles or 20 usec.
ld(AC,X)                        #10,12 Address of low byte to be added
adda(1)                         #11
st([vTmp])                      #12 Address of high byte to be added
ld([vAC])                       #13 Add the low bytes
adda([X])                       #14
st([vAC])                       #15 Store low result
bmi('.addw#18')                 #16 Now figure out if there was a carry
suba([X])                       #17 Gets back the initial value of vAC
bra('.addw#20')                 #18
ora([X])                        #19 Carry in bit 7
label('.addw#18')
anda([X])                       #18 Carry in bit 7
nop()                           #19
label('.addw#20')
anda(0x80,X)                    #20 Move carry to bit 0
ld([X])                         #21
adda([vAC+1])                   #22 Add the high bytes with carry
ld([vTmp],X)                    #23
adda([X])                       #24
st([vAC+1])                     #25 Store high result
bra('NEXT')                     #26
ld(-28/2)                       #27

# Instruction PEEK: Read byte from memory (vAC=[vAC]), 26 cycles
label('PEEK')
ld(hi('peek'),Y)                #10
jmp(Y,'peek')                   #11
#ld([vPC])                      #12 Overlap
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
label('.sys#13')
ld([vPC])                       #13,12 Retry until sufficient time
suba(2)                         #14
st([vPC])                       #15
bra('REENTER')                  #16
ld(-20/2)                       #17
label('SYS')
adda([vTicks])                  #10
blt('.sys#13')                  #11
ld([sysFn+1],Y)                 #12
jmp(Y,[sysFn])                  #13
#dummy()                        #14 Overlap
#
# Instruction SUBW: Word subtract with zero page (AC-=[D]+256*[D+1]), 28 cycles
# All cases can be done in 26 cycles, but the code will become much larger
label('SUBW')
ld(AC,X)                        #10,14 Address of low byte to be subtracted
adda(1)                         #11
st([vTmp])                      #12 Address of high byte to be subtracted
ld([vAC])                       #13
bmi('.subw#16')                 #14
suba([X])                       #15
st([vAC])                       #16 Store low result
bra('.subw#19')                 #17
ora([X])                        #18 Carry in bit 7
label('.subw#16')
st([vAC])                       #16 Store low result
anda([X])                       #17 Carry in bit 7
nop()                           #18
label('.subw#19')
anda(0x80,X)                    #19 Move carry to bit 0
ld([vAC+1])                     #20
suba([X])                       #21
ld([vTmp],X)                    #22
suba([X])                       #23
st([vAC+1])                     #24
label('REENTER_28')
ld(-28/2)                       #25
label('REENTER')
bra('NEXT')                     #26 Return from SYS calls
ld([vPC+1],Y)                   #27

# Instruction DEF: Define data or code (vAC,vPC=vPC+2,(vPC&0xff00)+D), 24 cycles
label('DEF')
ld(hi('def#13'),Y)              #10
jmp(Y,'def#13')                 #11
#st([vTmp])                     #12 Overlap
#
# Instruction CALL: Goto address and remember vPC (vLR,vPC=vPC+2,[D]+256*[D+1]-2), 26 cycles
label('CALL')
st([vTmp])                      #10,12
ld([vPC])                       #11
adda(2)                         #12 Point to instruction after CALL
st([vLR])                       #13
ld([vPC+1])                     #14
st([vLR+1])                     #15
ld([vTmp],X)                    #16
ld([X])                         #17
suba(2)                         #18 Because NEXT will add 2
st([vPC])                       #19
ld([vTmp])                      #20
adda(1,X)                       #21
ld([X])                         #22
st([vPC+1],Y)                   #23
bra('NEXT')                     #24
ld(-26/2)                       #25

# Instruction ALLOC: Create or destroy stack frame (vSP+=D), 14 cycles
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

# Instruction ADDI: Add small positive constant (vAC+=D), 28 cycles
label('ADDI')
ld(hi('addi'),Y)                #10
jmp(Y,'addi')                   #11
st([vTmp])                      #12

# Instruction SUBI: Subtract small positive constant (vAC+=D), 28 cycles
label('SUBI')
ld(hi('subi'),Y)                #10
jmp(Y,'subi')                   #11
st([vTmp])                      #12

# Instruction LSLW: Logical shift left (vAC<<=1), 28 cycles
# Useful, because ADDW can't add vAC to itself. Also more compact.
label('LSLW')
ld(hi('lslw'),Y)                #10
jmp(Y,'lslw')                   #11
ld([vAC])                       #12

# Instruction STLW: Store word in stack frame ([vSP+D],[vSP+D+1]=vAC&255,vAC>>8), 26 cycles
label('STLW')
ld(hi('stlw'),Y)                #10
jmp(Y,'stlw')                   #11
#dummy()                        #12 Overlap
#
# Instruction LDLW: Load word from stack frame (vAC=[vSP+D]+256*[vSP+D+1]), 26 cycles
label('LDLW')
ld(hi('ldlw'),Y)                #10,12
jmp(Y,'ldlw')                   #11
#dummy()                        #12 Overlap
#
# Instruction POKE: Write byte in memory ([[D+1],[D]]=vAC&255), 28 cycles
label('POKE')
ld(hi('poke'),Y)                #10,12
jmp(Y,'poke')                   #11
st([vTmp])                      #12

# Instruction DOKE: Write word in memory ([[D+1],[D]],[[D+1],[D]+1]=vAC&255,vAC>>8), 28 cycles
label('DOKE')
ld(hi('doke'),Y)                #10
jmp(Y,'doke')                   #11
st([vTmp])                      #12

# Instruction DEEK: Read word from memory (vAC=[vAC]+256*[vAC+1]), 28 cycles
label('DEEK')
ld(hi('deek'),Y)                #10
jmp(Y,'deek')                   #11
#dummy()                        #12 Overlap
#
# Instruction ANDW: Word logical-AND with zero page (vAC&=[D]+256*[D+1]), 28 cycles
label('ANDW')
ld(hi('andw'),Y)                #10,12
jmp(Y,'andw')                   #11
#dummy()                        #12 Overlap
#
# Instruction ORW: Word logical-OR with zero page (vAC|=[D]+256*[D+1]), 28 cycles
label('ORW')
ld(hi('orw'),Y)                 #10,12
jmp(Y,'orw')                    #11
#dummy()                        #12 Overlap
#
# Instruction XORW: Word logical-XOR with zero page (vAC^=[D]+256*[D+1]), 26 cycles
label('XORW')
ld(hi('xorw'),Y)                #10,12
jmp(Y,'xorw')                   #11
st([vTmp])                      #12
# We keep XORW 2 cycles faster than ANDW/ORW, because that
# can be useful for comparing numbers for equality a tiny
# bit faster than with SUBW

# Instruction RET: Function return (vPC=vLR-2), 16 cycles
label('RET')
ld([vLR])                       #10
assert pc()&255 == 0

#-----------------------------------------------------------------------
#
#  $0400 ROM page 4: Application interpreter extension
#
#-----------------------------------------------------------------------
align(0x100, size=0x100)

# (Continue RET)
suba(2)                         #11
st([vPC])                       #12
ld([vLR+1])                     #13
st([vPC+1])                     #14
ld(hi('REENTER'),Y)             #15
jmp(Y,'REENTER')                #16
ld(-20/2)                       #17

# DEF implementation
label('def#13')
ld([vPC])                       #13
adda(2)                         #14
st([vAC])                       #15
ld([vPC+1])                     #16
st([vAC+1])                     #17
ld([vTmp])                      #18
st([vPC])                       #19
ld(hi('NEXTY'),Y)               #20
jmp(Y,'NEXTY')                  #21
ld(-24/2)                       #22

# Clear vACH (continuation of ANDI and LD instructions)
label('andi#13')
nop()                           #13
st([vAC])                       #14
#
label('ld#15')
ld(0)                           #15 Clear high byte
st([vAC+1])                     #16
ld(hi('REENTER'),Y)             #17
jmp(Y,'REENTER')                #18
ld(-22/2)                       #19

# ADDI implementation
label('addi')
adda([vAC])                     #13
st([vAC])                       #14 Store low result
bmi('.addi#17')                 #15 Now figure out if there was a carry
suba([vTmp])                    #16 Gets back the initial value of vAC
bra('.addi#19')                 #17
ora([vTmp])                     #18 Carry in bit 7
label('.addi#17')
anda([vTmp])                    #17 Carry in bit 7
nop()                           #18
label('.addi#19')
anda(0x80,X)                    #19 Move carry to bit 0
ld([X])                         #20
adda([vAC+1])                   #21 Add the high bytes with carry
ld(hi('REENTER_28'),Y)          #22
jmp(Y,'REENTER_28')             #23
st([vAC+1])                     #24 Store high result

# SUBI implementation
label('subi')
ld([vAC])                       #13
bmi('.subi#16')                 #14
suba([vTmp])                    #15
st([vAC])                       #16 Store low result
bra('.subi#19')                 #17
ora([vTmp])                     #18 Carry in bit 7
label('.subi#16')
st([vAC])                       #16 Store low result
anda([vTmp])                    #17 Carry in bit 7
nop()                           #18
label('.subi#19')
anda(0x80,X)                    #19 Move carry to bit 0
ld([vAC+1])                     #20
suba([X])                       #21
ld(hi('REENTER_28'),Y)          #22
jmp(Y,'REENTER_28')             #23
st([vAC+1])                     #24

# LSLW implementation
label('lslw')
anda(128,X)                     #13
adda([vAC])                     #14
st([vAC])                       #15
ld([X])                         #16
adda([vAC+1])                   #17
adda([vAC+1])                   #18
st([vAC+1])                     #19
ld([vPC])                       #20
suba(1)                         #21
ld(hi('REENTER_28'),Y)          #22
jmp(Y,'REENTER_28')             #23
st([vPC])                       #24

# STLW implementation
label('stlw')
adda([vSP])                     #13
st([vTmp])                      #14
adda(1,X)                       #15
ld([vAC+1])                     #16
st([X])                         #17
ld([vTmp],X)                    #18
ld([vAC])                       #19
st([X])                         #20
ld(hi('REENTER'),Y)             #21
jmp(Y,'REENTER')                #22
ld(-26/2)                       #23

# LDLW implementation
label('ldlw')
adda([vSP])                     #13
st([vTmp])                      #14
adda(1,X)                       #15
ld([X])                         #16
st([vAC+1])                     #17
ld([vTmp],X)                    #18
ld([X])                         #19
st([vAC])                       #20
ld(hi('REENTER'),Y)             #21
jmp(Y,'REENTER')                #22
ld(-26/2)                       #23

# POKE implementation
label('poke')
adda(1,X)                       #13
ld([X])                         #14
ld(AC,Y)                        #15
ld([vTmp],X)                    #16
ld([X])                         #17
ld(AC,X)                        #18
ld([vAC])                       #19
st([Y,X])                       #20
ld(hi('REENTER'),Y)             #21
jmp(Y,'REENTER')                #22
ld(-26/2)                       #23

# PEEK implementation
label('peek')
suba(1)                         #13
st([vPC])                       #14
ld([vAC],X)                     #15
ld([vAC+1],Y)                   #16
ld([Y,X])                       #17
st([vAC])                       #18
label('lupReturn#19')           #Nice coincidence that lupReturn can be here
ld(0)                           #19
st([vAC+1])                     #20
ld(hi('REENTER'),Y)             #21
jmp(Y,'REENTER')                #22
ld(-26/2)                       #23

# DOKE implementation
label('doke')
adda(1,X)                       #13
ld([X])                         #14
ld(AC,Y)                        #15
ld([vTmp],X)                    #16
ld([X])                         #17
ld(AC,X)                        #18
ld([vAC])                       #19
st([Y,Xpp])                     #20
ld([vAC+1])                     #21
st([Y,X])                       #22 Incompatible with REENTER_28
ld(hi('REENTER'),Y)             #23
jmp(Y,'REENTER')                #24
ld(-28/2)                       #25

# DEEK implementation
label('deek')
ld([vPC])                       #13
suba(1)                         #14
st([vPC])                       #15
ld([vAC],X)                     #16
ld([vAC+1],Y)                   #17
ld([Y,X])                       #18
st([Y,Xpp])                     #19 Just X++
st([vAC])                       #20
ld([Y,X])                       #21
ld(hi('REENTER_28'),Y)          #22
jmp(Y,'REENTER_28')             #23
st([vAC+1])                     #24

# ANDW implementation
label('andw')
st([vTmp])                      #13
adda(1,X)                       #14
ld([X])                         #15
anda([vAC+1])                   #16
st([vAC+1])                     #17
ld([vTmp],X)                    #18
ld([X])                         #19
anda([vAC])                     #20
st([vAC])                       #21
ld(hi('REENTER_28'),Y)          #22
jmp(Y,'REENTER_28')             #23
#dummy()                        #24 Overlap
#
# ORW implementation
label('orw')
st([vTmp])                      #13,24
adda(1,X)                       #14
ld([X])                         #15
ora([vAC+1])                    #16
st([vAC+1])                     #17
ld([vTmp],X)                    #18
ld([X])                         #19
ora([vAC])                      #20
st([vAC])                       #21
ld(hi('REENTER_28'),Y)          #22
jmp(Y,'REENTER_28')             #23
#dummy()                        #24 Overlap
#
# XORW implementation
label('xorw')
adda(1,X)                       #13,24
ld([X])                         #14
xora([vAC+1])                   #15
st([vAC+1])                     #16
ld([vTmp],X)                    #17
ld([X])                         #18
xora([vAC])                     #19
st([vAC])                       #20
ld(hi('REENTER'),Y)             #21
jmp(Y,'REENTER')                #22
ld(-26/2)                       #23

#-----------------------------------------------------------------------
#
#  vCPU extension functions (for acceleration and compaction) follow below.
#
#  The naming convention is: SYS_<CamelCase>[_v<V>]_<N>
#
#  With <N> the maximum number of cycles the function will run
#  (counted from NEXT to NEXT). This is the same number that must
#  be passed to the 'SYS' vCPU instruction as operand, and it will
#  appear in the GCL code upon use.
#
#  If a SYS extension got introduced after ROM v1, the version number of
#  introduction is included in the name. This helps the programmer to be
#  reminded to verify the acutal ROM version and fail gracefully on older
#  ROMs than required. See also Docs/GT1-files.txt on using [romType].
#
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Extension SYS_Random_34: Update entropy and copy to vAC
#-----------------------------------------------------------------------

# This same algorithm runs automatically once per vertical blank.
# Use this function to get numbers at a higher rate.
#
# Variables:
#       vAC

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
ld(hi('REENTER'),Y)             #29
jmp(Y,'REENTER')                #30
ld(-34/2)                       #31

label('SYS_LSRW7_30')
ld([vAC])                       #15
anda(128,X)                     #16
ld([vAC+1])                     #17
adda(AC)                        #18
ora([X])                        #19
st([vAC])                       #20
ld([vAC+1])                     #21
anda(128,X)                     #22
ld([X])                         #23
st([vAC+1])                     #24
ld(hi('REENTER'),Y)             #25
jmp(Y,'REENTER')                #26
ld(-30/2)                       #27

label('SYS_LSRW8_24')
ld([vAC+1])                     #15
st([vAC])                       #16
ld(0)                           #17
st([vAC+1])                     #18
ld(hi('REENTER'),Y)             #19
jmp(Y,'REENTER')                #20
ld(-24/2)                       #21

label('SYS_LSLW8_24')
ld([vAC])                       #15
st([vAC+1])                     #16
ld(0)                           #17
st([vAC])                       #18
ld(hi('REENTER'),Y)             #19
jmp(Y,'REENTER')                #20
ld(-24/2)                       #21

#-----------------------------------------------------------------------
# Extension SYS_Draw4_30
#-----------------------------------------------------------------------

# Draw 4 pixels on screen, horizontally next to each other
#
# Variables:
#       sysArgs[0:3]    Pixels (in)
#       sysArgs[4:5]    Position on screen (in)

label('SYS_Draw4_30')
ld([sysArgs+4],X)               #15
ld([sysArgs+5],Y)               #16
ld([sysArgs+0])                 #17
st([Y,Xpp])                     #18
ld([sysArgs+1])                 #19
st([Y,Xpp])                     #20
ld([sysArgs+2])                 #21
st([Y,Xpp])                     #22
ld([sysArgs+3])                 #23
st([Y,Xpp])                     #24
ld(hi('REENTER'),Y)             #25
jmp(Y,'REENTER')                #26
ld(-30/2)                       #27

#-----------------------------------------------------------------------
# Extension SYS_VDrawBits_134:
#-----------------------------------------------------------------------

# Draw slice of a character, 8 pixels vertical
#
# Variables:
#       sysArgs[0]      Color 0 "background" (in)
#       sysArgs[1]      Color 1 "pen" (in)
#       sysArgs[2]      8 bits, highest bit first (in, changed)
#       sysArgs[4:5]    Position on screen (in)

label('SYS_VDrawBits_134')
ld(hi('sys_VDrawBits'),Y)       #15
jmp(Y,'sys_VDrawBits')          #16
ld([sysArgs+4],X)               #17

#-----------------------------------------------------------------------

# INC implementation
label('inc#14')
adda([X])                       #14
st([X])                         #15
ld(hi('NEXTY'),Y)               #16
jmp(Y,'NEXTY')                  #17
ld(-20/2)                       #18

# Interrupt handler:
#       ST   $xx        -> optionally store vCpuSelect
#       ... IRQ payload ...
# either:
#       LDWI $400
#       LUP  0          -> vRTI and don't switch interpreter (immediate resume)
# or:
#       LDWI $400
#       LUP  $xx        -> vRTI and switch interpreter type as stored in [$xx]
fillers(until=251-11)
label('vRTI#15')
ld([0x30])                      #15 Continue with vCPU in the same timeslice (faster)
st([vPC])                       #16
ld([0x31])                      #17
st([vPC+1])                     #18
ld([0x32])                      #19
st([vAC])                       #20
ld([0x33])                      #21
st([vAC+1])                     #22
ld(hi('REENTER'),Y)             #23
jmp(Y,'REENTER')                #24
ld(-28/2)                       #25
# vRTI entry point
assert(pc()&255 == 251)         # The landing offset 251 for LUP trampoline is fixed
beq('vRTI#15')                  #13 vRTI sequence
adda(1,X)                       #14
ld(hi('vRTI#18'),Y)             #15 Switch and wait for end of timeslice (slower)
jmp(Y,'vRTI#18')                #16
st([vTmp])                      #17

#-----------------------------------------------------------------------
#
#  $0500 ROM page 5-6: Shift table and code
#
#-----------------------------------------------------------------------

align(0x100, size=0x200)

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

label('shiftTable')
shiftTable = pc()

for ix in range(255):
  for n in range(1,7): # Find first zero
    if ~ix & (1 << (n-1)):
      break
  pattern = ['x' if i<n else '1' if ix&(1<<i) else '0' for i in range(8)]
  ld(ix>>n); C('0b%s >> %d' % (''.join(reversed(pattern)), n))

assert pc()&255 == 255
bra([vTmp])                     # Jumps back into next page

label('SYS_LSRW1_48')
assert pc()&255 == 0            # First instruction on this page *must* be a nop
nop()                           #15
ld(hi('shiftTable'),Y)          #16 Logical shift right 1 bit (X >> 1)
ld('.sysLsrw1a')                #17 Shift low byte
st([vTmp])                      #18
ld([vAC])                       #19
anda(0b11111110)                #20
jmp(Y,AC)                       #21
bra(255)                        #22 bra shiftTable+255
label('.sysLsrw1a')
st([vAC])                       #26
ld([vAC+1])                     #27 Transfer bit 8
anda(1)                         #28
adda(127)                       #29
anda(128)                       #30
ora([vAC])                      #31
st([vAC])                       #32
ld('.sysLsrw1b')                #33 Shift high byte
st([vTmp])                      #34
ld([vAC+1])                     #35
anda(0b11111110)                #36
jmp(Y,AC)                       #37
bra(255)                        #38 bra shiftTable+255
label('.sysLsrw1b')
st([vAC+1])                     #42
ld(hi('REENTER'),Y)             #43
jmp(Y,'REENTER')                #44
ld(-48/2)                       #45

label('SYS_LSRW2_52')
ld(hi('shiftTable'),Y)          #15 Logical shift right 2 bit (X >> 2)
ld('.sysLsrw2a')                #16 Shift low byte
st([vTmp])                      #17
ld([vAC])                       #18
anda(0b11111100)                #19
ora( 0b00000001)                #20
jmp(Y,AC)                       #21
bra(255)                        #22 bra shiftTable+255
label('.sysLsrw2a')
st([vAC])                       #26
ld([vAC+1])                     #27 Transfer bit 8:9
adda(AC)                        #28
adda(AC)                        #29
adda(AC)                        #30
adda(AC)                        #31
adda(AC)                        #32
adda(AC)                        #33
ora([vAC])                      #34
st([vAC])                       #35
ld('.sysLsrw2b')                #36 Shift high byte
st([vTmp])                      #37
ld([vAC+1])                     #38
anda(0b11111100)                #39
ora( 0b00000001)                #40
jmp(Y,AC)                       #41
bra(255)                        #42 bra shiftTable+255
label('.sysLsrw2b')
st([vAC+1])                     #46
ld(hi('REENTER'),Y)             #47
jmp(Y,'REENTER')                #48
ld(-52/2)                       #49

label('SYS_LSRW3_52')
ld(hi('shiftTable'),Y)          #15 Logical shift right 3 bit (X >> 3)
ld('.sysLsrw3a')                #16 Shift low byte
st([vTmp])                      #17
ld([vAC])                       #18
anda(0b11111000)                #19
ora( 0b00000011)                #20
jmp(Y,AC)                       #21
bra(255)                        #22 bra shiftTable+255
label('.sysLsrw3a')
st([vAC])                       #26
ld([vAC+1])                     #27 Transfer bit 8:10
adda(AC)                        #28
adda(AC)                        #29
adda(AC)                        #30
adda(AC)                        #31
adda(AC)                        #32
ora([vAC])                      #33
st([vAC])                       #34
ld('.sysLsrw3b')                #35 Shift high byte
st([vTmp])                      #36
ld([vAC+1])                     #37
anda(0b11111000)                #38
ora( 0b00000011)                #39
jmp(Y,AC)                       #40
bra(255)                        #41 bra shiftTable+255
label('.sysLsrw3b')
st([vAC+1])                     #45
ld(-52/2)                       #46
ld(hi('REENTER'),Y)             #47
jmp(Y,'REENTER')                #48
#nop()                          #49

label('SYS_LSRW4_50')
ld(hi('shiftTable'),Y)          #15,49 Logical shift right 4 bit (X >> 4)
ld('.sysLsrw4a')                #16 Shift low byte
st([vTmp])                      #17
ld([vAC])                       #18
anda(0b11110000)                #19
ora( 0b00000111)                #20
jmp(Y,AC)                       #21
bra(255)                        #22 bra shiftTable+255
label('.sysLsrw4a')
st([vAC])                       #26
ld([vAC+1])                     #27 Transfer bit 8:11
adda(AC)                        #28
adda(AC)                        #29
adda(AC)                        #30
adda(AC)                        #31
ora([vAC])                      #32
st([vAC])                       #33
ld('.sysLsrw4b')                #34 Shift high byte'
st([vTmp])                      #35
ld([vAC+1])                     #36
anda(0b11110000)                #37
ora( 0b00000111)                #38
jmp(Y,AC)                       #39
bra(255)                        #40 bra shiftTable+255
label('.sysLsrw4b')
st([vAC+1])                     #44
ld(hi('REENTER'),Y)             #45
jmp(Y,'REENTER')                #46
ld(-50/2)                       #47

label('SYS_LSRW5_50')
ld(hi('shiftTable'),Y)          #15 Logical shift right 5 bit (X >> 5)
ld('.sysLsrw5a')                #16 Shift low byte
st([vTmp])                      #17
ld([vAC])                       #18
anda(0b11100000)                #19
ora( 0b00001111)                #20
jmp(Y,AC)                       #21
bra(255)                        #22 bra shiftTable+255
label('.sysLsrw5a')
st([vAC])                       #26
ld([vAC+1])                     #27 Transfer bit 8:13
adda(AC)                        #28
adda(AC)                        #29
adda(AC)                        #30
ora([vAC])                      #31
st([vAC])                       #32
ld('.sysLsrw5b')                #33 Shift high byte
st([vTmp])                      #34
ld([vAC+1])                     #35
anda(0b11100000)                #36
ora( 0b00001111)                #37
jmp(Y,AC)                       #38
bra(255)                        #39 bra shiftTable+255
label('.sysLsrw5b')
st([vAC+1])                     #44
ld(-50/2)                       #45
ld(hi('REENTER'),Y)             #46
jmp(Y,'REENTER')                #47
#nop()                          #48

label('SYS_LSRW6_48')
ld(hi('shiftTable'),Y)          #15,44 Logical shift right 6 bit (X >> 6)
ld('.sysLsrw6a')                #16 Shift low byte
st([vTmp])                      #17
ld([vAC])                       #18
anda(0b11000000)                #19
ora( 0b00011111)                #20
jmp(Y,AC)                       #21
bra(255)                        #22 bra shiftTable+255
label('.sysLsrw6a')
st([vAC])                       #26
ld([vAC+1])                     #27 Transfer bit 8:13
adda(AC)                        #28
adda(AC)                        #29
ora([vAC])                      #30
st([vAC])                       #31
ld('.sysLsrw6b')                #32 Shift high byte
st([vTmp])                      #33
ld([vAC+1])                     #34
anda(0b11000000)                #35
ora( 0b00011111)                #36
jmp(Y,AC)                       #37
bra(255)                        #38 bra shiftTable+255
label('.sysLsrw6b')
st([vAC+1])                     #42
ld(hi('REENTER'),Y)             #43
jmp(Y,'REENTER')                #44
ld(-48/2)                       #45

label('SYS_LSLW4_46')
ld(hi('shiftTable'),Y)          #15 Logical shift left 4 bit (X << 4)
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
jmp(Y,AC)                       #27
bra(255)                        #28 bra shiftTable+255
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
ld(hi('REENTER'),Y)             #41
jmp(Y,'REENTER')                #42
#nop()                          #43

#-----------------------------------------------------------------------
# Extension SYS_Read3_40
#-----------------------------------------------------------------------

# Read 3 consecutive bytes from ROM
#
# Note: This function a bit obsolete, as it has very limited use. It's
#       effectively an application-specific SYS function for the Pictures
#       application from ROM v1. It requires the ROM data be organized
#       with trampoline3a and trampoline3b fragments, and their address
#       in ROM to be known. Better avoid using this.
#
# Variables:
#       sysArgs[0:2]    Bytes (out)
#       sysArgs[6:7]    ROM pointer (in)

label('SYS_Read3_40')
ld([sysArgs+7],Y)               #15,32
jmp(Y,128-7)                    #16 trampoline3a
ld([sysArgs+6])                 #17
label('txReturn')
st([sysArgs+2])                 #34
ld(hi('REENTER'),Y)             #35
jmp(Y,'REENTER')                #36
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
  align(1, size=0x80)

def trampoline3b():
  """Read 3 bytes from ROM page (continue)"""
  while pc()&255 < 256-6:
    nop()
  st([sysArgs+1])               #27
  C('Trampoline for page $%02x00 reading (continue)' % (pc()>>8))
  ld([sysArgs+6])               #28
  adda(2)                       #29
  ld(hi('txReturn'),Y)          #30
  bra(AC)                       #31
  jmp(Y,'txReturn')             #32
  align(1, size=0x100)

#-----------------------------------------------------------------------
# Extension SYS_Unpack_56
#-----------------------------------------------------------------------

# Unpack 3 bytes into 4 pixels
#
# Variables:
#       sysArgs[0:2]    Packed bytes (in)
#       sysArgs[0:3]    Pixels (out)

label('SYS_Unpack_56')
ld(soundTable>>8,Y)             #15
ld([sysArgs+2])                 #16 a[2]>>2
ora(0x03,X)                     #17
ld([Y,X])                       #18
st([sysArgs+3])                 #19 -> Pixel 3

ld([sysArgs+2])                 #20 (a[2]&3)<<4
anda(0x03)                      #21
adda(AC)                        #22
adda(AC)                        #23
adda(AC)                        #24
adda(AC)                        #25
st([sysArgs+2])                 #26
ld([sysArgs+1])                 #27 | a[1]>>4
ora(0x03,X)                     #28
ld([Y,X])                       #29
ora(0x03,X)                     #30
ld([Y,X])                       #31
ora([sysArgs+2])                #32
st([sysArgs+2])                 #33 -> Pixel 2

ld([sysArgs+1])                 #34 (a[1]&15)<<2
anda(0x0f)                      #35
adda(AC)                        #36
adda(AC)                        #37
st([sysArgs+1])                 #38

ld([sysArgs+0])                 #39 | a[0]>>6
ora(0x03,X)                     #40
ld([Y,X])                       #41
ora(0x03,X)                     #42
ld([Y,X])                       #43
ora(0x03,X)                     #44
ld([Y,X])                       #45
ora([sysArgs+1])                #46
st([sysArgs+1])                 #47 -> Pixel 1

ld([sysArgs+0])                 #48 a[1]&63
anda(0x3f)                      #49
st([sysArgs+0])                 #50 -> Pixel 0

ld(hi('REENTER'),Y)             #51
jmp(Y,'REENTER')                #52
ld(-56/2)                       #53

#-----------------------------------------------------------------------
#       v6502 right shift instruction
#-----------------------------------------------------------------------

label('v6502_lsr#30')
ld([v6502_ADH],Y)               #30 Result
st([Y,X])                       #31
st([v6502_Qz])                  #32 Z flag
st([v6502_Qn])                  #33 N flag
ld(hi('v6502_next'),Y)          #34
ld(-38/2)                       #35
jmp(Y,'v6502_next')             #36
#nop()                          #37 Overlap
#
label('v6502_ror#38')
ld([v6502_ADH],Y)               #38,38 Result
ora([v6502_BI])                 #39 Transfer bit 8
st([Y,X])                       #40
st([v6502_Qz])                  #41 Z flag
st([v6502_Qn])                  #42 N flag
ld(hi('v6502_next'),Y)          #43
jmp(Y,'v6502_next')             #44
ld(-46/2)                       #45

#-----------------------------------------------------------------------
#       Reserved
#-----------------------------------------------------------------------

# XXX Reserve space for LSRW?

#-----------------------------------------------------------------------
#
#  $0700 ROM page 7-8: Gigatron font data
#
#-----------------------------------------------------------------------

align(0x100, size=0x100)

label('font32up')
for ch in range(32, 32+50):
  comment = 'Char %s' % repr(chr(ch))
  for byte in font.font[ch-32]:
    ld(byte)
    comment = C(comment)

trampoline()

#-----------------------------------------------------------------------

align(0x100, size=0x100)

label('font82up')
for ch in range(32+50, 132):
  comment = 'Char %s' % repr(chr(ch))
  for byte in font.font[ch-32]:
    ld(byte)
    comment = C(comment)

trampoline()

#-----------------------------------------------------------------------
#
#  $0900 ROM page 9: Key table for music
#
#-----------------------------------------------------------------------

align(0x100, size=0x100)
notes = 'CCDDEFFGGAAB'
sampleRate = cpuClock / 200.0 / 4
label('notesTable')
ld(0)
ld(0)
for i in range(0, 250, 2):
  j = i//2-1
  freq = 440.0*2.0**((j-57)/12.0)
  if j>=0 and freq <= sampleRate/2.0:
    key = int(round(32768 * freq / sampleRate))
    octave, note = j//12, notes[j%12]
    sharp = '-' if notes[j%12-1] != note else '#'
    comment = '%s%s%s (%0.1f Hz)' % (note, sharp, octave, freq)
    ld(key&127); C(comment); ld(key>>7)

trampoline()

#-----------------------------------------------------------------------
#
#  $0a00 ROM page 10: Inversion table
#
#-----------------------------------------------------------------------

align(0x100, size=0x100)
label('invTable')

# Unit 64, table offset 16 (=1/4), value offset 1: (x+16)*(y+1) == 64*64 - e
for i in range(251):
  ld(4096//(i+16)-1)

trampoline()

#-----------------------------------------------------------------------
#
#  $0d00 ROM page 11: More SYS functions
#
#-----------------------------------------------------------------------

align(0x100, size=0x100)

#-----------------------------------------------------------------------
# Extension SYS_SetMode_v2_80
#-----------------------------------------------------------------------

# Set video mode to 0 to 3 black scanlines per pixel line.
#
# Mainly for making the MODE command available in Tiny BASIC, so that
# the user can experiment. It's adviced to refrain from using
# SYS_SetMode_v2_80 in regular applications. Video mode is a deeply
# personal preference, and the programmer shouldn't overrule the user
# in that choice. The Gigatron philisophy is that the end user has
# the final say on what happens on the system, not the application,
# even if that implies a degraded performance. This doesn't mean that
# all applications must work well in all video modes: mode 1 is still
# the default. If an application really doesn't work at all in that
# mode, it's acceptable to change mode once after loading.
#
# There's no "SYS_GetMode" function.
#
# Variables:
#       vAC bit 0:1     Mode:
#                         0      "ABCD" -> Full mode (slowest)
#                         1      "ABC-" -> Default mode after reset
#                         2      "A-C-" -> at67's mode
#                         3      "A---" -> HGM's mode
#       vAC bit 2:15    Ignored bits and should be 0
#
# Special values (ROM v4):
#       vAC = 1975      Zombie mode (no video signals, no input,
#                        no blinkenlights).
#       vAC = -1        Leave zombie mode and restore previous mode.

# Actual duration is <80 cycles, but keep some room for future extensions
label('SYS_SetMode_v2_80')
ld(hi('sys_SetMode'),Y)         #15
jmp(Y,'sys_SetMode')            #16
ld([vReturn])                   #17

#-----------------------------------------------------------------------
# Extension SYS_SetMemory_v2_54
#-----------------------------------------------------------------------

# SYS function for setting 1..256 bytes
#
# sysArgs[0]   Copy count (in, changed)
# sysArgs[1]   Copy value (in)
# sysArgs[2:3] Destination address (in, changed)
#
# Sets up to 8 bytes per invocation before restarting itself through vCPU.
# Doesn't wrap around page boundary. Can run 3 times per 148-cycle time slice.
# All combined that gives a 300% speedup over ROMv4 and before.

label('SYS_SetMemory_v2_54')
ld([sysArgs+0])                 #15
bra('sys_SetMemory#18')         #16
ld([sysArgs+2],X)               #17

#-----------------------------------------------------------------------
# Extension SYS_SendSerial1_v3_80
#-----------------------------------------------------------------------

# SYS function for sending data over serial controller port using
# pulse width modulation of the vertical sync signal.
#
# Variables:
#       sysArgs[0:1]    Source address               (in, changed)
#       sysArgs[2]      Start bit mask (typically 1) (in, changed)
#       sysArgs[3]      Number of send frames X      (in, changed)
#
# The sending will abort if input data is detected on the serial port.
# Returns 0 in case of all bits sent, or <>0 in case of abort
#
# This modulates the next upcoming X vertical pulses with the supplied
# data. A zero becomes a 7 line vPulse, a one will be 9 lines.
# After that, the vPulse width falls back to 8 lines (idle).

label('SYS_SendSerial1_v3_80')
ld([videoY])                    #15
bra('sys_SendSerial1')          #16
xora(videoYline0)               #17 First line of vertical blank

#-----------------------------------------------------------------------
# Extension SYS_ExpanderControl_v4_40
#-----------------------------------------------------------------------

# Sets the I/O and RAM expander's control register
#
# Variables:
#       vAC bit 2       Device enable /SS0
#           bit 3       Device enable /SS1
#           bit 4       Device enable /SS2
#           bit 5       Device enable /SS3
#           bit 6       Banking B0
#           bit 7       Banking B1
#           bit 15      Data out MOSI
#       sysArgs[7]      Cache for control state (written to)
#
# Intended for prototyping, and probably too low-level for most applications
# Still there's a safeguard: it's not possible to disable RAM using this

label('SYS_ExpanderControl_v4_40')
ld(hi('sys_ExpanderControl'),Y) #15
jmp(Y,'sys_ExpanderControl')    #16
ld(hi(ctrlBits),Y)              #17

#-----------------------------------------------------------------------
# Extension SYS_Run6502_v4_80
#-----------------------------------------------------------------------

# Transfer control to v6502
#
# Calling 6502 code from vCPU goes (only) through this SYS function.
# Directly modifying the vCpuSelect variable is unreliable. The
# control transfer is immediate, without waiting for the current
# time slice to end or first returning to vCPU.
#
# vCPU code and v6502 code can interoperate without much hassle:
# - The v6502 program counter is vLR, and v6502 doesn't touch vPC
# - Returning to vCPU is with the BRK instruction
# - BRK doesn't dump process state on the stack
# - vCPU can save/restore the vLR with PUSH/POP
# - Stacks are shared, vAC is shared
# - vAC can indicate what the v6502 code wants. vAC+1 will be cleared
# - Alternative is to leave a word in sysArgs[6:7] (v6502 X and Y registers)
# - Another way is to set vPC before BRK, and vCPU will continue there(+2)
#
# Calling v6502 code from vCPU looks like this:
#       LDWI  SYS_Run6502_v4_80
#       STW   sysFn
#       LDWI  $6502_start_address
#       STW   vLR
#       SYS   80
#
# Variables:
#       vAC             Accumulator
#       vLR             Program Counter
#       vSP             Stack Pointer (+1)
#       sysArgs[6]      Index Register X
#       sysArgs[7]      Index Register Y
# For info:
#       sysArgs[0:1]    Address Register, free to clobber
#       sysArgs[2]      Instruction Register, free to clobber
#       sysArgs[3:5]    Flags, don't touch
#
# Implementation details::
#
#  The time to reserve for this transition is the maximum time
#  between NEXT and v6502_check. This is
#       SYS call duration + 2*v6502_maxTicks + (v6502_overhead - vCPU_overhead)
#     = 22 + 28 + (11 - 9) = 62 cycles.
#  So reserving 80 cycles is future proof. This isn't overhead, as it includes
#  the fetching of the first 6502 opcode and its operands..
#
#                      0            10                 28=0         9
#    ---+----+---------+------------+------------------+-----------+---
# video | nop| runVcpu |   ENTER    | At least one ins |   EXIT    | video
#    ---+----+---------+------------+------------------+-----------+---
#        sync  prelude  ENTER-to-ins    ins-to-NEXT     NEXT-to-video
#       |<-->|
#        0/1 |<------->|
#                 5    |<----------------------------->|
#          runVCpu_overhead           28               |<--------->|
#                                 2*maxTicks                 9
#                                                      vCPU_overhead
#
#                      0                21                    38=0       11
#    ---+----+---------+----------------+--------------------+-----------+---
# video | nop| runVcpu |   v6502_ENTER  | At least one fetch |v6502_exitB| video
#    ---+----+---------+----------------+--------------------+-----------+---
#        sync  prelude   enter-to-fetch     fetch-to-check    check-to-video
#       |<-->|
#        0/1 |<------->|
#                 5    |<----------------------------------->|
#          runVcpu_overhead           38                     |<--------->|
#                              2*v6520_maxTicks                    11
#                                                            v6502_overhead

label('SYS_Run6502_v4_80')
ld(hi('sys_v6502'),Y)           #15
jmp(Y,'sys_v6502')              #16
ld(hi('v6502_ENTER'))           #17 Activate v6502

#-----------------------------------------------------------------------
# Extension SYS_ResetWaveforms_v4_50
#-----------------------------------------------------------------------

# soundTable[4x+0] = sawtooth, to be modified into metallic/noise
# soundTable[4x+1] = pulse
# soundTable[4x+2] = triangle
# soundTable[4x+3] = sawtooth, also useful to right shift 2 bits

label('SYS_ResetWaveforms_v4_50')
ld(hi('sys_ResetWaveforms'),Y)  #15 Initial setup of waveforms. [vAC+0]=i
jmp(Y,'sys_ResetWaveforms')     #16
ld(soundTable>>8,Y)             #17

#-----------------------------------------------------------------------
# Extension SYS_ShuffleNoise_v4_46
#-----------------------------------------------------------------------

# Use simple 6-bits variation of RC4 to permutate waveform 0 in soundTable

label('SYS_ShuffleNoise_v4_46')
ld(hi('sys_ShuffleNoise'),Y)    #15 Shuffle soundTable[4i+0]. [vAC+0]=4j, [vAC+1]=4i
jmp(Y,'sys_ShuffleNoise')       #16
ld(soundTable>>8,Y)             #17

#-----------------------------------------------------------------------
# Extension SYS_SpiExchangeBytes_v4_134
#-----------------------------------------------------------------------

# Send AND receive 1..256 bytes over SPI interface

# Variables:
#       sysArgs[0]      Page index start, for both send/receive (in, changed)
#       sysArgs[1]      Memory page for send data (in)
#       sysArgs[2]      Page index stop (in)
#       sysArgs[3]      Memory page for receive data (in)
#       sysArgs[4]      Scratch (changed)

label('SYS_SpiExchangeBytes_v4_134')
ld(hi('sys_SpiExchangeBytes'),Y)#15
jmp(Y,'sys_SpiExchangeBytes')   #16
ld(hi(ctrlBits),Y)              #17 Control state as saved by SYS_ExpanderControl


#-----------------------------------------------------------------------
# Extension SYS_ReceiveSerial1_DEVROM_32
#-----------------------------------------------------------------------

# SYS function for receiving one byte over the serial controller port.
# This is a public version of SYS_NextByteIn from the loader private
# extension.  A byte is read from IN when videoY reaches
# sysArgs[3]. The byte is added to the checksum sysArgs[2] then stored
# at address sysArgs[0:1] which gets incremented.
#
# The 65 bytes of a serial frame can be read for the following values
# of videoY: 207 (protocol byte) 219 (length, 6 bits only) 235, 251
# (address) then 2, 6, 10, .. 238 stepping by four, then 185.
#
# Variables:
#     sysArgs[0:1] Address
#     sysArgs[2]   Checksum
#     sysArgs[3]   Wait value (videoY)

label('SYS_ReceiveSerial1_DEVROM_32')
bra('sys_ReceiveSerial1')       #15
ld([sysArgs+3])                 #16

#-----------------------------------------------------------------------
#  Implementations
#-----------------------------------------------------------------------

# SYS_SetMemory_54 implementation
label('sys_SetMemory#18')
ld([sysArgs+3],Y)               #18
ble('.sysSb#21')                #19 Enter fast lane if >=128 or at 0 (-> 256)
suba(8)                         #20
bge('.sysSb#23')                #21 Or when >=8
st([sysArgs+0])                 #22
anda(4)                         #23
beq('.sysSb#26')                #24
ld([sysArgs+1])                 #25 Set 4 pixels
st([Y,Xpp])                     #26
st([Y,Xpp])                     #27
st([Y,Xpp])                     #28
bra('.sysSb#31')                #29
st([Y,Xpp])                     #30
label('.sysSb#26')
wait(31-26)                     #26
label('.sysSb#31')
ld([sysArgs+0])                 #31
anda(2)                         #32
beq('.sysSb#35')                #33
ld([sysArgs+1])                 #34 Set 2 pixels
st([Y,Xpp])                     #35
bra('.sysSb#38')                #36
st([Y,Xpp])                     #37
label('.sysSb#35')
wait(38-35)                     #35
label('.sysSb#38')
ld([sysArgs+0])                 #38
anda(1)                         #39
beq(pc()+3)                     #40
bra(pc()+3)                     #41
ld([sysArgs+1])                 #42 Set 1 pixel
ld([Y,X])                       #42(!) No change
st([Y,X])                       #43
ld(hi('NEXTY'),Y)               #44 Return
jmp(Y,'NEXTY')                  #45 All done
ld(-48/2)                       #46
label('.sysSb#21')
nop()                           #21
st([sysArgs+0])                 #22
label('.sysSb#23')
ld([sysArgs+1])                 #23 Set 8 pixels
st([Y,Xpp])                     #24
st([Y,Xpp])                     #25
st([Y,Xpp])                     #26
st([Y,Xpp])                     #27
st([Y,Xpp])                     #28
st([Y,Xpp])                     #29
st([Y,Xpp])                     #30
st([Y,Xpp])                     #31
ld([sysArgs+2])                 #32 Advance write pointer
adda(8)                         #33
st([sysArgs+2])                 #34
ld([sysArgs+0])                 #35
beq(pc()+3)                     #36
bra(pc()+3)                     #37
ld(-2)                          #38 Self-restart when more to do
ld(0)                           #38(!)
adda([vPC])                     #39
st([vPC])                       #40
ld(hi('REENTER'),Y)             #41
jmp(Y,'REENTER')                #42
ld(-46/2)                       #43

# SYS_SetMode_80 implementation
label('sys_SetMode')
bne(pc()+3)                     #18
bra(pc()+2)                     #19
ld('startVideo')                #20 First enable video if disabled
st([vReturn])                   #20,21
ld([vAC+1])                     #22
beq('.sysSm#25')                #23
ld(hi('REENTER'),Y)             #24
xora([vAC])                     #25
xora((1975>>8)^(1975&255))      #26 Poor man\'s 1975 detection
bne(pc()+3)                     #27
bra(pc()+3)                     #28
assert videoZ == 0x0100
st([vReturn])                   #29 DISABLE video/audio/serial/etc
nop()                           #29(!) Ignore and return
jmp(Y,'REENTER')                #30
ld(-34/2)                       #31
label('.sysSm#25')
ld([vAC])                       #25 Mode 0,1,2,3
anda(3)                         #26
adda('.sysSm#30')               #27
bra(AC)                         #28
bra('.sysSm#31')                #29
label('.sysSm#30')
ld('pixels')                    #30 videoB lines
ld('pixels')                    #30
ld('nopixels')                  #30
ld('nopixels')                  #30
label('.sysSm#31')
st([videoModeB])                #31
ld([vAC])                       #32
anda(3)                         #33
adda('.sysSm#37')               #34
bra(AC)                         #35
bra('.sysSm#38')                #36
label('.sysSm#37')
ld('pixels')                    #37 videoC lines
ld('pixels')                    #37
ld('pixels')                    #37
ld('nopixels')                  #37
label('.sysSm#38')
st([videoModeC])                #38
ld([vAC])                       #39
anda(3)                         #40
adda('.sysSm#44')               #41
bra(AC)                         #42
bra('.sysSm#45')                #43
label('.sysSm#44')
ld('pixels')                    #44 videoD lines
ld('nopixels')                  #44
ld('nopixels')                  #44
ld('nopixels')                  #44
label('.sysSm#45')
st([videoModeD])                #45
jmp(Y,'REENTER')                #46
ld(-50/2)                       #47

# SYS_SendSerial1_v3_80 implementation
label('sys_SendSerial1')
beq('.sysSs#20')                #18
ld([sysArgs+0],X)               #19
ld([vPC])                       #20 Wait for vBlank
suba(2)                         #21
ld(hi('REENTER_28'),Y)          #22
jmp(Y,'REENTER_28')             #23
st([vPC])                       #24
label('.sysSs#20')
ld([sysArgs+1],Y)               #20 Synchronized with vBlank
ld([Y,X])                       #21 Copy next bit
anda([sysArgs+2])               #22
bne(pc()+3)                     #23
bra(pc()+3)                     #24
ld(7*2)                         #25
ld(9*2)                         #25
st([videoPulse])                #26
ld([sysArgs+2])                 #27 Rotate input bit
adda(AC)                        #28
bne(pc()+3)                     #29
bra(pc()+2)                     #30
ld(1)                           #31
st([sysArgs+2])                 #31,32 (must be idempotent)
anda(1)                         #33 Optionally increment pointer
adda([sysArgs+0])               #34
st([sysArgs+0],X)               #35
ld([sysArgs+3])                 #36 Frame counter
suba(1)                         #37
beq('.sysSs#40')                #38
ld(hi('REENTER'),Y)             #39
st([sysArgs+3])                 #40
ld([serialRaw])                 #41 Test for anything being sent back
xora(255)                       #42
beq('.sysSs#45')                #43
st([vAC])                       #44 Abort after key press with non-zero error
st([vAC+1])                     #45
jmp(Y,'REENTER')                #46
ld(-50/2)                       #47
label('.sysSs#45')
ld([vPC])                       #45 Continue sending bits
suba(2)                         #46
st([vPC])                       #47
jmp(Y,'REENTER')                #48
ld(-52/2)                       #49
label('.sysSs#40')
st([vAC])                       #40 Stop sending bits, no error
st([vAC+1])                     #41
jmp(Y,'REENTER')                #42
ld(-46/2)                       #43

# SYS_ReceiveSerialByte implementation
label('sys_ReceiveSerial1')
xora([videoY])                  #17
bne('.sysRsb#20')               #18
ld([sysArgs+0],X)               #19
ld([sysArgs+1],Y)               #20
ld(IN)                          #21
st([Y,X])                       #22
adda([sysArgs+2])               #23
st([sysArgs+2])                 #24
ld([sysArgs+0])                 #25
adda(1)                         #26
st([sysArgs+0])                 #27
ld(hi('NEXTY'),Y)               #28
jmp(Y,'NEXTY')                  #29
ld(-32/2)                       #30
# Restart the instruction in the next timeslice
label('.sysRsb#20')
ld([vPC])                       #20
suba(2)                         #21
st([vPC])                       #22
ld(hi('REENTER'),Y)             #23
jmp(Y,'REENTER')                #24
ld(-28/2)                       #25

# CALLI implementation (vCPU instruction)
label('calli#13')
adda(3)                         #13,43
st([vLR])                       #14
ld([vPC+1])                     #15
st([vLR+1],Y)                   #16
ld([Y,X])                       #17
st([Y,Xpp])                     #18 Just X++
suba(2)                         #19
st([vPC])                       #20
ld([Y,X])                       #21
ld(hi('REENTER_28'),Y)          #22
jmp(Y,'REENTER_28')             #23
st([vPC+1])                     #24

# -------------------------------------------------------------
# vCPU instructions for comparisons between two 16-bit operands
# -------------------------------------------------------------
#
# vCPU's conditional branching (BCC) always compares vAC against 0,
# treating vAC as a two's complement 16-bit number. When we need to
# compare two arbitrary numnbers we normally first take their difference
# with SUBW.  However, when this difference is too large, the subtraction
# overflows and we get the wrong outcome. To get it right over the
# entire range, an elaborate sequence is needed. TinyBASIC uses this
# blurp for its relational operators. (It compares stack variable $02
# with zero page variable $3a.)
#
#       0461  ee 02            LDLW  $02
#       0463  fc 3a            XORW  $3a
#       0465  35 53 6a         BGE   $046c
#       0468  ee 02            LDLW  $02
#       046a  90 6e            BRA   $0470
#       046c  ee 02            LDLW  $02
#       046e  b8 3a            SUBW  $3a
#       0470  35 56 73         BLE   $0475
#
# The CMPHS and CMPHU instructions were introduced to simplify this.
# They inspect both operands to see if there is an overflow risk. If
# so, they modify vAC such that their difference gets smaller, while
# preserving the relation between the two operands. After that, the
# SUBW instruction can't overflow and we achieve a correct comparison.
# Use CMPHS for signed comparisons and CMPHU for unsigned. With these,
# the sequence above becomes:
#
#       0461  ee 02            LDLW  $02
#       0463  1f 3b            CMPHS $3b        Note: high byte of operand
#       0465  b8 3a            SUBW  $3a
#       0467  35 56 73         BLE   $0475
#
# CMPHS/CMPHU don't make much sense other than in combination with
# SUBW. These modify vACH, if needed, as given in the following table:
#
#       vACH  varH  |     vACH
#       bit7  bit7  | CMPHS  CMPHU
#       ---------------------------
#         0     0   |  vACH   vACH      no change needed
#         0     1   | varH+1 varH-1     narrowing the range
#         1     0   | varH-1 varH+1     narrowing the range
#         1     1   |  vACH   vACH      no change needed
#       ---------------------------

# CMPHS implementation (vCPU instruction)
label('cmphs#13')
ld(hi('REENTER'),Y)             #13
ld([X])                         #14
xora([vAC+1])                   #15
bpl('.cmphu#18')                #16 Skip if same sign
ld([vAC+1])                     #17
bmi(pc()+3)                     #18
bra(pc()+3)                     #19
label('.cmphs#20')
ld(+1)                          #20    vAC < variable
ld(-1)                          #20(!) vAC > variable
label('.cmphs#21')
adda([X])                       #21
st([vAC+1])                     #22
jmp(Y,'REENTER_28')             #23
#dummy()                        #24 Overlap
#
# CMPHS implementation (vCPU instruction)
label('cmphu#13')
ld(hi('REENTER'),Y)             #13,24
ld([X])                         #14
xora([vAC+1])                   #15
bpl('.cmphu#18')                #16 Skip if same sign
ld([vAC+1])                     #17
bmi('.cmphs#20')                #18
bra('.cmphs#21')                #19
ld(-1)                          #20    vAC > variable

# No-operation for CMPHS/CMPHU when high bits are equal
label('.cmphu#18')
jmp(Y,'REENTER')                #18
ld(-22/2)                       #19

#-----------------------------------------------------------------------
#
#  $0c00 ROM page 12: More SYS functions (sprites)
#
#       Page 1: vertical blank interval
#       Page 2: visible scanlines
#
#-----------------------------------------------------------------------

align(0x100, size=0x100)

#-----------------------------------------------------------------------
# Extension SYS_Sprite6_v3_64
# Extension SYS_Sprite6x_v3_64
# Extension SYS_Sprite6y_v3_64
# Extension SYS_Sprite6xy_v3_64
#-----------------------------------------------------------------------

# Blit sprite in screen memory
#
# Variables
#       vAC             Destination address in screen
#       sysArgs[0:1]    Source address of 6xY pixels (colors 0..63) terminated
#                       by negative byte value N (typically N = -Y)
#       sysArgs[2:7]    Scratch (user as copy buffer)
#
# This SYS function draws a sprite of 6 pixels wide and Y pixels high.
# The pixel data is read sequentually from RAM, in horizontal chunks
# of 6 pixels at a time, and then written to the screen through the
# destination pointer (each chunk underneath the previous), thus
# drawing a 6xY stripe. Pixel values should be non-negative. The first
# negative byte N after a chunk signals the end of the sprite data.
# So the sprite's height Y is determined by the source data and is
# therefore flexible. This negative byte value, typically N == -Y,
# is then used to adjust the destination pointer's high byte, to make
# it easier to draw sprites wider than 6 pixels: just repeat the SYS
# call for as many 6-pixel wide stripes you need. All arguments are
# already left in place to facilitate this. After one call, the source
# pointer will point past that source data, effectively:
#       src += Y * 6 + 1
# The destination pointer will have been adjusted as:
#       dst += (Y + N) * 256 + 6
# (With arithmetic wrapping around on the same memory page)
#
# Y is only limited by source memory, not by CPU cycles. The
# implementation is such that the SYS function self-repeats, each
# time drawing the next 6-pixel chunk. It can typically draw 12
# pixels per scanline this way.

label('SYS_Sprite6_v3_64')

ld([sysArgs+0],X)               #15 Pixel data source address
ld([sysArgs+1],Y)               #16
ld([Y,X])                       #17 Next pixel or stop
bpl('.sysDpx0')                 #18
st([Y,Xpp])                     #19 Just X++

adda([vAC+1])                   #20 Adjust dst for convenience
st([vAC+1])                     #21
ld([vAC])                       #22
adda(6)                         #23
st([vAC])                       #24
ld([sysArgs+0])                 #25 Adjust src for convenience
adda(1)                         #26
st([sysArgs+0])                 #27
nop()                           #28
ld(hi('REENTER'),Y)             #29 Normal exit (no self-repeat)
jmp(Y,'REENTER')                #30
ld(-34/2)                       #31

label('.sysDpx0')
st([sysArgs+2])                 #20 Gobble 6 pixels into buffer
ld([Y,X])                       #21
st([Y,Xpp])                     #22 Just X++
st([sysArgs+3])                 #23
ld([Y,X])                       #24
st([Y,Xpp])                     #25 Just X++
st([sysArgs+4])                 #26
ld([Y,X])                       #27
st([Y,Xpp])                     #28 Just X++
st([sysArgs+5])                 #29
ld([Y,X])                       #30
st([Y,Xpp])                     #31 Just X++
st([sysArgs+6])                 #32
ld([Y,X])                       #33
st([Y,Xpp])                     #34 Just X++
st([sysArgs+7])                 #35

ld([vAC],X)                     #36 Screen memory destination address
ld([vAC+1],Y)                   #37
ld([sysArgs+2])                 #38 Write 6 pixels
st([Y,Xpp])                     #39
ld([sysArgs+3])                 #40
st([Y,Xpp])                     #41
ld([sysArgs+4])                 #42
st([Y,Xpp])                     #43
ld([sysArgs+5])                 #44
st([Y,Xpp])                     #45
ld([sysArgs+6])                 #46
st([Y,Xpp])                     #47
ld([sysArgs+7])                 #48
st([Y,Xpp])                     #49

ld([sysArgs+0])                 #50 src += 6
adda(6)                         #51
st([sysArgs+0])                 #52
ld([vAC+1])                     #53 dst += 256
adda(1)                         #54
st([vAC+1])                     #55

ld([vPC])                       #56 Self-repeating SYS call
suba(2)                         #57
st([vPC])                       #58
ld(hi('REENTER'),Y)             #59
jmp(Y,'REENTER')                #60
ld(-64/2)                       #61

align(64)
label('SYS_Sprite6x_v3_64')

ld([sysArgs+0],X)               #15 Pixel data source address
ld([sysArgs+1],Y)               #16
ld([Y,X])                       #17 Next pixel or stop
bpl('.sysDpx1')                 #18
st([Y,Xpp])                     #19 Just X++

adda([vAC+1])                   #20 Adjust dst for convenience
st([vAC+1])                     #21
ld([vAC])                       #22
suba(6)                         #23
st([vAC])                       #24
ld([sysArgs+0])                 #25 Adjust src for convenience
adda(1)                         #26
st([sysArgs+0])                 #27
nop()                           #28
ld(hi('REENTER'),Y)             #29 Normal exit (no self-repeat)
jmp(Y,'REENTER')                #30
ld(-34/2)                       #31

label('.sysDpx1')
st([sysArgs+7])                 #20 Gobble 6 pixels into buffer (backwards)
ld([Y,X])                       #21
st([Y,Xpp])                     #22 Just X++
st([sysArgs+6])                 #23
ld([Y,X])                       #24
st([Y,Xpp])                     #25 Just X++
st([sysArgs+5])                 #26
ld([Y,X])                       #27
st([Y,Xpp])                     #28 Just X++
st([sysArgs+4])                 #29
ld([Y,X])                       #30
st([Y,Xpp])                     #31 Just X++
st([sysArgs+3])                 #32
ld([Y,X])                       #33
st([Y,Xpp])                     #34 Just X++

ld([vAC],X)                     #35 Screen memory destination address
ld([vAC+1],Y)                   #36
st([Y,Xpp])                     #37 Write 6 pixels
ld([sysArgs+3])                 #38
st([Y,Xpp])                     #39
ld([sysArgs+4])                 #40
st([Y,Xpp])                     #41
ld([sysArgs+5])                 #42
st([Y,Xpp])                     #43
ld([sysArgs+6])                 #44
st([Y,Xpp])                     #45
ld([sysArgs+7])                 #46
st([Y,Xpp])                     #47

ld([sysArgs+0])                 #48 src += 6
adda(6)                         #49
st([sysArgs+0])                 #50
ld([vAC+1])                     #51 dst += 256
adda(1)                         #52
st([vAC+1])                     #53

ld([vPC])                       #54 Self-repeating SYS call
suba(2)                         #55
st([vPC])                       #56
ld(hi('REENTER'),Y)             #57
jmp(Y,'REENTER')                #58
ld(-62/2)                       #59

align(64)
label('SYS_Sprite6y_v3_64')

ld([sysArgs+0],X)               #15 Pixel data source address
ld([sysArgs+1],Y)               #16
ld([Y,X])                       #17 Next pixel or stop
bpl('.sysDpx2')                 #18
st([Y,Xpp])                     #19 Just X++

xora(255)                       #20 Adjust dst for convenience
adda(1)                         #21
adda([vAC+1])                   #22
st([vAC+1])                     #23
ld([vAC])                       #24
adda(6)                         #25
st([vAC])                       #26
ld([sysArgs+0])                 #27 Adjust src for convenience
adda(1)                         #28
st([sysArgs+0])                 #29
nop()                           #30
ld(hi('REENTER'),Y)             #31 Normal exit (no self-repeat)
jmp(Y,'REENTER')                #32
ld(-36/2)                       #33

label('.sysDpx2')
st([sysArgs+2])                 #20 Gobble 6 pixels into buffer
ld([Y,X])                       #21
st([Y,Xpp])                     #22 Just X++
st([sysArgs+3])                 #23
ld([Y,X])                       #24
st([Y,Xpp])                     #25 Just X++
st([sysArgs+4])                 #26
ld([Y,X])                       #27
st([Y,Xpp])                     #28 Just X++
st([sysArgs+5])                 #29
ld([Y,X])                       #30
st([Y,Xpp])                     #31 Just X++
st([sysArgs+6])                 #32
ld([Y,X])                       #33
st([Y,Xpp])                     #34 Just X++
st([sysArgs+7])                 #35

ld([vAC],X)                     #36 Screen memory destination address
ld([vAC+1],Y)                   #37
ld([sysArgs+2])                 #38 Write 6 pixels
st([Y,Xpp])                     #39
ld([sysArgs+3])                 #40
st([Y,Xpp])                     #41
ld([sysArgs+4])                 #42
st([Y,Xpp])                     #43
ld([sysArgs+5])                 #44
st([Y,Xpp])                     #45
ld([sysArgs+6])                 #46
st([Y,Xpp])                     #47
ld([sysArgs+7])                 #48
st([Y,Xpp])                     #49

ld([sysArgs+0])                 #50 src += 6
adda(6)                         #51
st([sysArgs+0])                 #52
ld([vAC+1])                     #53 dst -= 256
suba(1)                         #54
st([vAC+1])                     #55

ld([vPC])                       #56 Self-repeating SYS call
suba(2)                         #57
st([vPC])                       #58
ld(hi('REENTER'),Y)             #59
jmp(Y,'REENTER')                #60
ld(-64/2)                       #61

align(64)
label('SYS_Sprite6xy_v3_64')

ld([sysArgs+0],X)               #15 Pixel data source address
ld([sysArgs+1],Y)               #16
ld([Y,X])                       #17 Next pixel or stop
bpl('.sysDpx3')                 #18
st([Y,Xpp])                     #19 Just X++

xora(255)                       #20 Adjust dst for convenience
adda(1)                         #21
adda([vAC+1])                   #22
st([vAC+1])                     #23
ld([vAC])                       #24
suba(6)                         #25
st([vAC])                       #26
ld([sysArgs+0])                 #27 Adjust src for convenience
adda(1)                         #28
st([sysArgs+0])                 #29
nop()                           #30
ld(hi('REENTER'),Y)             #31 Normal exit (no self-repeat)
jmp(Y,'REENTER')                #32
ld(-36/2)                       #33

label('.sysDpx3')
st([sysArgs+7])                 #20 Gobble 6 pixels into buffer (backwards)
ld([Y,X])                       #21
st([Y,Xpp])                     #22 Just X++
st([sysArgs+6])                 #23
ld([Y,X])                       #24
st([Y,Xpp])                     #25 Just X++
st([sysArgs+5])                 #26
ld([Y,X])                       #27
st([Y,Xpp])                     #28 Just X++
st([sysArgs+4])                 #29
ld([Y,X])                       #30
st([Y,Xpp])                     #31 Just X++
st([sysArgs+3])                 #32
ld([Y,X])                       #33
st([Y,Xpp])                     #34 Just X++

ld([vAC],X)                     #35 Screen memory destination address
ld([vAC+1],Y)                   #36
st([Y,Xpp])                     #37 Write 6 pixels
ld([sysArgs+3])                 #38
st([Y,Xpp])                     #39
ld([sysArgs+4])                 #40
st([Y,Xpp])                     #41
ld([sysArgs+5])                 #42
st([Y,Xpp])                     #43
ld([sysArgs+6])                 #44
st([Y,Xpp])                     #45
ld([sysArgs+7])                 #46
st([Y,Xpp])                     #47

ld([sysArgs+0])                 #48 src += 6
adda(6)                         #49
st([sysArgs+0])                 #50
ld([vAC+1])                     #51 dst -= 256
suba(1)                         #52
st([vAC+1])                     #53

ld([vPC])                       #54 Self-repeating SYS call
suba(2)                         #55
st([vPC])                       #56
ld(hi('REENTER'),Y)             #57
jmp(Y,'REENTER')                #58
ld(-62/2)                       #59

#-----------------------------------------------------------------------

align(0x100)

label('sys_ExpanderControl')   
ld(0b00001100)                      #18 bits 2 and 3
anda([vAC])                         #19 check for special ctrl code space
beq('sysEx#22')                     #20
ld([vAC])                           #21 load low byte of ctrl code in delay slot
anda(0xfc)                          #22 sanitize normal ctrl code
st([Y,ctrlBits])                    #23 store in ctrlBits
st([vTmp])                          #24 store in vTmp
bra('sysEx#27')                     #25 jump to issuing normal ctrl code 
ld([vAC+1],Y)                       #26 load high byte of ctrl code in delay slot
label('sysEx#22')
anda(0xfc,X)                        #22 * special ctrl code
ld([Y,ctrlBits])                    #23 get previous normal code from ctrlBits
st([vTmp])                          #24 save it in vTmp
ld([vAC+1],Y)                       #25 load high byte of ctrl code
ctrl(Y,X)                           #26 issue special ctrl code
label('sysEx#27')
ld([vTmp])                          #27 load saved normal ctrl code
anda(0xfc,X)                        #28 sanitize ctrlBits
ctrl(Y,X)                           #29 issue normal ctrl code
ld([vTmp])                          #30 always load something after ctrl
ld(hi('REENTER'),Y)                 #31
jmp(Y,'REENTER')                    #32
ld(-36/2)                           #33


#-----------------------------------------------------------------------

label('sys_SpiExchangeBytes')

ld([Y,ctrlBits])                #18
st([sysArgs+4])                 #19

ld([sysArgs+0],X)               #20 Fetch byte to send
ld([sysArgs+1],Y)               #21
ld([Y,X])                       #22

for i in range(8):
  st([vTmp],Y);C('Bit %d'%(7-i))#23+i*12
  ld([sysArgs+4],X)             #24+i*12
  ctrl(Y,Xpp)                   #25+i*12 Set MOSI
  ctrl(Y,Xpp)                   #26+i*12 Raise SCLK, disable RAM!
  ld([0])                       #27+i*12 Get MISO
  anda(0b00001111)              #28+i*12 This is why R1 as pull-DOWN is simpler
  beq(pc()+3)                   #29+i*12
  bra(pc()+2)                   #30+i*12
  ld(1)                         #31+i*12
  ctrl(Y,X)                     #32+i*12,29+i*12 (Must be idempotent) Lower SCLK
  adda([vTmp])                  #33+i*12 Shift
  adda([vTmp])                  #34+i*12

ld([sysArgs+0],X)               #119 Store received byte
ld([sysArgs+3],Y)               #120
st([Y,X])                       #121

ld([sysArgs+0])                 #122 Advance pointer
adda(1)                         #123
st([sysArgs+0])                 #124

xora([sysArgs+2])               #125 Reached end?
beq('.sysSpi#128')              #126

ld([vPC])                       #127 Self-repeating SYS call
suba(2)                         #128
st([vPC])                       #129
ld(hi('NEXTY'),Y)               #130
jmp(Y,'NEXTY')                  #131
ld(-134/2)                      #132

label('.sysSpi#128')
ld(hi('NEXTY'),Y)               #128 Continue program
jmp(Y,'NEXTY')                  #129
ld(-132/2)                      #130

#-----------------------------------------------------------------------

label('sys_v6502')

st([vCpuSelect],Y)              #18 Activate v6502
ld(-22/2)                       #19
jmp(Y,'v6502_ENTER')            #20 Transfer control in the same time slice
adda([vTicks])                  #21
assert (38 - 22)//2 >= v6502_adjust

#-----------------------------------------------------------------------
#       MOS 6502 emulator
#-----------------------------------------------------------------------

# Some quirks:
# - Stack in zero page instead of page 1
# - No interrupts
# - No decimal mode (may never be added). D flag is emulated but ignored.
# - BRK switches back to running 16-bits vCPU
# - Illegal opcodes map to BRK, but can read ghost operands before trapping
# - Illegal opcode $ff won't be trapped and cause havoc instead

# Big things TODO:
# XXX Tuning, put most frequent instructions in the primary page

label('v6502_ror')
assert v6502_Cflag == 1
ld([v6502_ADH],Y)               #12
ld(-46//2+v6502_maxTicks)       #13 Is there enough time for the excess ticks?
adda([vTicks])                  #14
blt('.recheck17')               #15
ld([v6502_P])                   #16 Transfer C to "bit 8"
anda(1)                         #17
adda(127)                       #18
anda(128)                       #19
st([v6502_BI])                  #20 The real 6502 wouldn't use BI for this
ld([v6502_P])                   #21 Transfer bit 0 to C
anda(~1)                        #22
st([v6502_P])                   #23
ld([Y,X])                       #24
anda(1)                         #25
ora([v6502_P])                  #26
st([v6502_P])                   #27
ld('v6502_ror#38')              #28 Shift table lookup
st([vTmp])                      #29
ld([Y,X])                       #30
anda(~1)                        #31
ld(hi('shiftTable'),Y)          #32
jmp(Y,AC)                       #33
bra(255)                        #34 bra shiftTable+255
label('.recheck17')
ld(hi('v6502_check'),Y)         #17 Go back to time check before dispatch
jmp(Y,'v6502_check')            #18
ld(-20/2)                       #19

label('v6502_lsr')
assert v6502_Cflag == 1
ld([v6502_ADH],Y)               #12
ld([v6502_P])                   #13 Transfer bit 0 to C
anda(~1)                        #14
st([v6502_P])                   #15
ld([Y,X])                       #16
anda(1)                         #17
ora([v6502_P])                  #18
st([v6502_P])                   #19
ld('v6502_lsr#30')              #20 Shift table lookup
st([vTmp])                      #21
ld([Y,X])                       #22
anda(~1)                        #23
ld(hi('shiftTable'),Y)          #24
jmp(Y,AC)                       #25
bra(255)                        #26 bra shiftTable+255

label('v6502_rol')
assert v6502_Cflag == 1
ld([v6502_ADH],Y)               #12
ld([Y,X])                       #13
anda(0x80)                      #14
st([v6502_Tmp])                 #15
ld([v6502_P])                   #16
anda(1)                         #17
label('.rol#18')
adda([Y,X])                     #18
adda([Y,X])                     #19
st([Y,X])                       #20
st([v6502_Qz])                  #21 Z flag
st([v6502_Qn])                  #22 N flag
ld([v6502_P])                   #23 C Flag
anda(~1)                        #24
ld([v6502_Tmp],X)               #25
ora([X])                        #26
st([v6502_P])                   #27
ld(hi('v6502_next'),Y)          #28
ld(-32/2)                       #29
jmp(Y,'v6502_next')             #30
#nop()                          #31 Overlap
#
label('v6502_asl')
ld([v6502_ADH],Y)               #12,32
ld([Y,X])                       #13
anda(0x80)                      #14
st([v6502_Tmp])                 #15
bra('.rol#18')                  #16
ld(0)                           #17

label('v6502_jmp1')
nop()                           #12
ld([v6502_ADL])                 #13
st([v6502_PCL])                 #14
ld([v6502_ADH])                 #15
st([v6502_PCH])                 #16
ld(hi('v6502_next'),Y)          #17
jmp(Y,'v6502_next')             #18
ld(-20/2)                       #19

label('v6502_jmp2')
nop()                           #12
ld([v6502_ADH],Y)               #13
ld([Y,X])                       #14
st([Y,Xpp])                     #15 (Just X++) Wrap around: bug compatible with NMOS
st([v6502_PCL])                 #16
ld([Y,X])                       #17
st([v6502_PCH])                 #18
ld(hi('v6502_next'),Y)          #19
jmp(Y,'v6502_next')             #20
ld(-22/2)                       #21

label('v6502_pla')
ld([v6502_S])                   #12
ld(AC,X)                        #13
adda(1)                         #14
st([v6502_S])                   #15
ld([X])                         #16
st([v6502_A])                   #17
st([v6502_Qz])                  #18 Z flag
st([v6502_Qn])                  #19 N flag
ld(hi('v6502_next'),Y)          #20
ld(-24/2)                       #21
jmp(Y,'v6502_next')             #22
#nop()                          #23 Overlap
#
label('v6502_pha')
ld(hi('v6502_next'),Y)          #12,24
ld([v6502_S])                   #13
suba(1)                         #14
st([v6502_S],X)                 #15
ld([v6502_A])                   #16
st([X])                         #17
jmp(Y,'v6502_next')             #18
ld(-20/2)                       #19

label('v6502_brk')
ld(hi('ENTER'))                 #12 Switch to vCPU
st([vCpuSelect])                #13
assert v6502_A == vAC
ld(0)                           #14
st([vAC+1])                     #15
ld(hi('REENTER'),Y)             #16 Switch in the current time slice
ld(-22//2+v6502_adjust)         #17
jmp(Y,'REENTER')                #18
nop()                           #19

# All interpreter entry points must share the same page offset, because
# this offset is hard-coded as immediate operand in the video driver.
# The Gigatron's original vCPU's 'ENTER' label is already at $2ff, so we
# just use $dff for 'v6502_ENTER'. v6502 actually has two entry points.
# The other is 'v6502_RESUME' at $10ff. It is used for instructions
# that were fetched but not yet executed. Allowing the split gives finer
# granulariy, and hopefully more throughput for the simpler instructions.
# (There is no "overhead" for allowing instruction splitting, because
#  both emulation phases must administer [vTicks] anyway.)
while pc()&255 < 255:
  nop()
label('v6502_ENTER')
bra('v6502_next2')              #0 v6502 primary entry point
# --- Page boundary ---
suba(v6502_adjust)              #1,19 Adjust for vCPU/v6520 timing differences

#19 Addressing modes
(   'v6502_mode0'  ); bra('v6502_modeIZX'); bra('v6502_modeIMM'); bra('v6502_modeILL') # $00 xxx000xx
bra('v6502_modeZP');  bra('v6502_modeZP');  bra('v6502_modeZP');  bra('v6502_modeILL') # $04 xxx001xx
bra('v6502_modeIMP'); bra('v6502_modeIMM'); bra('v6502_modeACC'); bra('v6502_modeILL') # $08 xxx010xx
bra('v6502_modeABS'); bra('v6502_modeABS'); bra('v6502_modeABS'); bra('v6502_modeILL') # $0c xxx011xx
bra('v6502_modeREL'); bra('v6502_modeIZY'); bra('v6502_modeIMM'); bra('v6502_modeILL') # $10 xxx100xx
bra('v6502_modeZPX'); bra('v6502_modeZPX'); bra('v6502_modeZPX'); bra('v6502_modeILL') # $14 xxx101xx
bra('v6502_modeIMP'); bra('v6502_modeABY'); bra('v6502_modeIMP'); bra('v6502_modeILL') # $18 xxx110xx
bra('v6502_modeABX'); bra('v6502_modeABX'); bra('v6502_modeABX'); bra('v6502_modeILL') # $1c xxx111xx

# Special encoding cases for emulator:
#     $00 BRK -         but gets mapped to #$DD      handled in v6502_mode0
#     $20 JSR $DDDD     but gets mapped to #$DD      handled in v6502_mode0 and v6502_JSR
#     $40 RTI -         but gets mapped to #$DD      handled in v6502_mode0
#     $60 RTS -         but gets mapped to #$DD      handled in v6502_mode0
#     $6C JMP ($DDDD)   but gets mapped to $DDDD     handled in v6502_JMP2
#     $96 STX $DD,Y     but gets mapped to $DD,X     handled in v6502_STX2
#     $B6 LDX $DD,Y     but gets mapped to $DD,X     handled in v6502_LDX2
#     $BE LDX $DDDD,Y   but gets mapped to $DDDD,X   handled in v6502_modeABX

label('v6502_next')
adda([vTicks])                  #0
blt('v6502_exitBefore')         #1 No more ticks
label('v6502_next2')
st([vTicks])                    #2
#
# Fetch opcode
ld([v6502_PCL],X)               #3
ld([v6502_PCH],Y)               #4
ld([Y,X])                       #5 Fetch IR
st([v6502_IR])                  #6
ld([v6502_PCL])                 #7 PC++
adda(1)                         #8
st([v6502_PCL],X)               #9
beq(pc()+3)                     #10
bra(pc()+3)                     #11
ld(0)                           #12
ld(1)                           #12(!)
adda([v6502_PCH])               #13
st([v6502_PCH],Y)               #14
#
# Get addressing mode and fetch operands
ld([v6502_IR])                  #15 Get addressing mode
anda(31)                        #16
bra(AC)                         #17
bra('.next20')                  #18
# (jump table)                  #19
label('.next20')
ld([Y,X])                       #20 Fetch L
# Most opcodes branch away at this point, but IR & 31 == 0 falls through
#
# Implicit Mode for  BRK JSR RTI RTS (<  0x80) -- 26 cycles
# Immediate Mode for LDY CPY CPX     (>= 0x80) -- 36 cycles
label('v6502_mode0')
ld([v6502_IR])                  #21 'xxx0000'
bmi('.imm24')                   #22
ld([v6502_PCH])                 #23
bra('v6502_check')              #24
ld(-26/2)                       #25

# Resync with video driver. At this point we're returning BEFORE
# fetching and executing the next instruction.
label('v6502_exitBefore')
adda(v6502_maxTicks)            #3 Exit BEFORE fetch
bgt(pc()&255)                   #4 Resync
suba(1)                         #5
ld(hi('v6502_ENTER'))           #6 Set entry point to before 'fetch'
st([vCpuSelect])                #7
ld(hi('vBlankStart'),Y)         #8
jmp(Y,[vReturn])                #9 To video driver
ld(0)                           #10
assert v6502_overhead ==         11

# Immediate Mode: #$FF -- 36 cycles
label('v6502_modeIMM')
nop()                           #21 Wait for v6502_mode0 to join
nop()                           #22
ld([v6502_PCH])                 #23 Copy PC
label('.imm24')
st([v6502_ADH])                 #24
ld([v6502_PCL])                 #25
st([v6502_ADL],X)               #26
adda(1)                         #27 PC++
st([v6502_PCL])                 #28
beq(pc()+3)                     #29
bra(pc()+3)                     #30
ld(0)                           #31
ld(1)                           #31(!)
adda([v6502_PCH])               #32
st([v6502_PCH])                 #33
bra('v6502_check')              #34
ld(-36/2)                       #35

# Accumulator Mode: ROL ROR LSL ASR -- 28 cycles
label('v6502_modeACC')
ld(v6502_A&255)                 #21 Address of AC
st([v6502_ADL],X)               #22
ld(v6502_A>>8)                  #23
st([v6502_ADH])                 #24
ld(-28/2)                       #25
bra('v6502_check')              #26
#nop()                          #27 Overlap
#
# Implied Mode: no operand -- 24 cycles
label('v6502_modeILL')
label('v6502_modeIMP')
nop()                           #21,27
bra('v6502_check')              #22
ld(-24/2)                       #23

# Zero Page Modes: $DD $DD,X $DD,Y -- 36 cycles
label('v6502_modeZPX')
bra('.zp23')                    #21
adda([v6502_X])                 #22
label('v6502_modeZP')
bra('.zp23')                    #21
nop()                           #22
label('.zp23')
st([v6502_ADL],X)               #23
ld(0)                           #24 H=0
st([v6502_ADH])                 #25
ld(1)                           #26 PC++
adda([v6502_PCL])               #27
st([v6502_PCL])                 #28
beq(pc()+3)                     #29
bra(pc()+3)                     #30
ld(0)                           #31
ld(1)                           #31(!)
adda([v6502_PCH])               #32
st([v6502_PCH])                 #33
bra('v6502_check')              #34
ld(-36/2)                       #35

# Possible retry loop for modeABS and modeIZY. Because these need
# more time than the v6502_maxTicks of 38 Gigatron cycles, we may
# have to restart them after the next horizontal pulse.
label('.retry28')
beq(pc()+3)                     #28,37 PC--
bra(pc()+3)                     #29
ld(0)                           #30
ld(-1)                          #30(!)
adda([v6502_PCH])               #31
st([v6502_PCH])                 #32
ld([v6502_PCL])                 #33
suba(1)                         #34
st([v6502_PCL])                 #35
bra('v6502_next')               #36 Retry until sufficient time
ld(-38/2)                       #37

# Absolute Modes: $DDDD $DDDD,X $DDDD,Y -- 64 cycles
label('v6502_modeABS')
bra('.abs23')                   #21
ld(0)                           #22
label('v6502_modeABX')
bra('.abs23')                   #21
label('v6502_modeABY')
ld([v6502_X])                   #21,22
ld([v6502_Y])                   #22
label('.abs23')
st([v6502_ADL])                 #23
ld(-64//2+v6502_maxTicks)       #24 Is there enough time for the excess ticks?
adda([vTicks])                  #25
blt('.retry28')                 #26
ld([v6502_PCL])                 #27
ld([v6502_IR])                  #28 Special case $BE: LDX $DDDD,Y (we got X in ADL)
xora(0xbe)                      #29
beq(pc()+3)                     #30
bra(pc()+3)                     #31
ld([v6502_ADL])                 #32
ld([v6502_Y])                   #32(!)
adda([Y,X])                     #33 Fetch and add L
st([v6502_ADL])                 #34
bmi('.abs37')                   #35 Carry?
suba([Y,X])                     #36 Gets back original operand
bra('.abs39')                   #37
ora([Y,X])                      #38 Carry in bit 7
label('.abs37')
anda([Y,X])                     #37 Carry in bit 7
nop()                           #38
label('.abs39')
anda(0x80,X)                    #39 Move carry to bit 0
ld([X])                         #40
st([v6502_ADH])                 #41
ld([v6502_PCL])                 #42 PC++
adda(1)                         #43
st([v6502_PCL],X)               #44
beq(pc()+3)                     #45
bra(pc()+3)                     #46
ld(0)                           #47
ld(1)                           #47(!)
adda([v6502_PCH])               #48
st([v6502_PCH],Y)               #49
ld([Y,X])                       #50 Fetch H
adda([v6502_ADH])               #51
st([v6502_ADH])                 #52
ld([v6502_PCL])                 #53 PC++
adda(1)                         #54
st([v6502_PCL])                 #55
beq(pc()+3)                     #56
bra(pc()+3)                     #57
ld(0)                           #58
ld(1)                           #58(!)
adda([v6502_PCH])               #59
st([v6502_PCH])                 #60
ld([v6502_ADL],X)               #61
bra('v6502_check')              #62
ld(-64/2)                       #63

# Indirect Indexed Mode: ($DD),Y -- 54 cycles
label('v6502_modeIZY')
ld(AC,X)                        #21 $DD
ld(0,Y)                         #22 $00DD
ld(-54//2+v6502_maxTicks)       #23 Is there enough time for the excess ticks?
adda([vTicks])                  #24
nop()                           #25
blt('.retry28')                 #26
ld([v6502_PCL])                 #27
adda(1)                         #28 PC++
st([v6502_PCL])                 #29
beq(pc()+3)                     #30
bra(pc()+3)                     #31
ld(0)                           #32
ld(1)                           #32(!)
adda([v6502_PCH])               #33
st([v6502_PCH])                 #34
ld([Y,X])                       #35 Read word from zero-page
st([Y,Xpp])                     #36 (Just X++) Wrap-around is correct
st([v6502_ADL])                 #37
ld([Y,X])                       #38
st([v6502_ADH])                 #39
ld([v6502_Y])                   #40 Add Y
adda([v6502_ADL])               #41
st([v6502_ADL])                 #42
bmi('.izy45')                   #43 Carry?
suba([v6502_Y])                 #44 Gets back original operand
bra('.izy47')                   #45
ora([v6502_Y])                  #46 Carry in bit 7
label('.izy45')
anda([v6502_Y])                 #45 Carry in bit 7
nop()                           #46
label('.izy47')
anda(0x80,X)                    #47 Move carry to bit 0
ld([X])                         #48
adda([v6502_ADH])               #49
st([v6502_ADH])                 #50
ld([v6502_ADL],X)               #51
bra('v6502_check')              #52
ld(-54/2)                       #53

# Relative Mode: BEQ BNE BPL BMI BCC BCS BVC BVS -- 36 cycles
label('v6502_modeREL')
st([v6502_ADL],X)               #21 Offset (Only needed for branch)
bmi(pc()+3)                     #22 Sign extend
bra(pc()+3)                     #23
ld(0)                           #24
ld(255)                         #24(!)
st([v6502_ADH])                 #25
ld([v6502_PCL])                 #26 PC++ (Needed for both cases)
adda(1)                         #27
st([v6502_PCL])                 #28
beq(pc()+3)                     #29
bra(pc()+3)                     #30
ld(0)                           #31
ld(1)                           #31(!)
adda([v6502_PCH])               #32
st([v6502_PCH])                 #33
bra('v6502_check')              #34
ld(-36/2)                       #53

# Indexed Indirect Mode: ($DD,X) -- 38 cycles
label('v6502_modeIZX')
adda([v6502_X])                 #21 Add X
st([v6502_Tmp])                 #22
adda(1,X)                       #23 Read word from zero-page
ld([X])                         #24
st([v6502_ADH])                 #25
ld([v6502_Tmp],X)               #26
ld([X])                         #27
st([v6502_ADL],X)               #28
ld([v6502_PCL])                 #29 PC++
adda(1)                         #30
st([v6502_PCL])                 #31
beq(pc()+3)                     #32
bra(pc()+3)                     #33
ld(0)                           #34
ld(1)                           #34(!)
adda([v6502_PCH])               #35
st([v6502_PCH])                 #36
ld(-38/2)                       #37 !!! Fall through to v6502_check !!!
#
# Update elapsed time for the addressing mode processing.
# Then check if we can immediately execute this instruction.
# Otherwise transfer control to the video driver.
label('v6502_check')
adda([vTicks])                  #0
blt('v6502_exitAfter')          #1 No more ticks
st([vTicks])                    #2
ld(hi('v6502_execute'),Y)       #3
jmp(Y,[v6502_IR])               #4
bra(255)                        #5

# Otherwise resync with video driver. At this point we're returning AFTER
# addressing mode decoding, but before executing the instruction.
label('v6502_exitAfter')
adda(v6502_maxTicks)            #3 Exit AFTER fetch
bgt(pc()&255)                   #4 Resync
suba(1)                         #5
ld(hi('v6502_RESUME'))          #6 Set entry point to before 'execute'
st([vCpuSelect])                #7
ld(hi('vBlankStart'),Y)         #8
jmp(Y,[vReturn])                #9 To video driver
ld(0)                           #10
assert v6502_overhead ==         11

align(0x100,size=0x100)
label('v6502_execute')
# This page works as a 255-entry (0..254) jump table for 6502 opcodes.
# Jumping into this page must have 'bra 255' in the branch delay slot
# in order to get out again and dispatch to the right continuation.
# X must hold [v6502_ADL],
# Y will hold hi('v6502_execute'),
# A will be loaded with the code offset (this is skipped at offset $ff)
ld('v6502_BRK'); ld('v6502_ORA'); ld('v6502_ILL'); ld('v6502_ILL') #6 $00
ld('v6502_ILL'); ld('v6502_ORA'); ld('v6502_ASL'); ld('v6502_ILL') #6
ld('v6502_PHP'); ld('v6502_ORA'); ld('v6502_ASL'); ld('v6502_ILL') #6
ld('v6502_ILL'); ld('v6502_ORA'); ld('v6502_ASL'); ld('v6502_ILL') #6
ld('v6502_BPL'); ld('v6502_ORA'); ld('v6502_ILL'); ld('v6502_ILL') #6 $10
ld('v6502_ILL'); ld('v6502_ORA'); ld('v6502_ASL'); ld('v6502_ILL') #6
ld('v6502_CLC'); ld('v6502_ORA'); ld('v6502_ILL'); ld('v6502_ILL') #6
ld('v6502_ILL'); ld('v6502_ORA'); ld('v6502_ASL'); ld('v6502_ILL') #6
ld('v6502_JSR'); ld('v6502_AND'); ld('v6502_ILL'); ld('v6502_ILL') #6 $20
ld('v6502_BIT'); ld('v6502_AND'); ld('v6502_ROL'); ld('v6502_ILL') #6
ld('v6502_PLP'); ld('v6502_AND'); ld('v6502_ROL'); ld('v6502_ILL') #6
ld('v6502_BIT'); ld('v6502_AND'); ld('v6502_ROL'); ld('v6502_ILL') #6
ld('v6502_BMI'); ld('v6502_AND'); ld('v6502_ILL'); ld('v6502_ILL') #6 $30
ld('v6502_ILL'); ld('v6502_AND'); ld('v6502_ROL'); ld('v6502_ILL') #6
ld('v6502_SEC'); ld('v6502_AND'); ld('v6502_ILL'); ld('v6502_ILL') #6
ld('v6502_ILL'); ld('v6502_AND'); ld('v6502_ROL'); ld('v6502_ILL') #6
ld('v6502_RTI'); ld('v6502_EOR'); ld('v6502_ILL'); ld('v6502_ILL') #6 $40
ld('v6502_ILL'); ld('v6502_EOR'); ld('v6502_LSR'); ld('v6502_ILL') #6
ld('v6502_PHA'); ld('v6502_EOR'); ld('v6502_LSR'); ld('v6502_ILL') #6
ld('v6502_JMP1');ld('v6502_EOR'); ld('v6502_LSR'); ld('v6502_ILL') #6
ld('v6502_BVC'); ld('v6502_EOR'); ld('v6502_ILL'); ld('v6502_ILL') #6 $50
ld('v6502_ILL'); ld('v6502_EOR'); ld('v6502_LSR'); ld('v6502_ILL') #6
ld('v6502_CLI'); ld('v6502_EOR'); ld('v6502_ILL'); ld('v6502_ILL') #6
ld('v6502_ILL'); ld('v6502_EOR'); ld('v6502_LSR'); ld('v6502_ILL') #6
ld('v6502_RTS'); ld('v6502_ADC'); ld('v6502_ILL'); ld('v6502_ILL') #6 $60
ld('v6502_ILL'); ld('v6502_ADC'); ld('v6502_ROR'); ld('v6502_ILL') #6
ld('v6502_PLA'); ld('v6502_ADC'); ld('v6502_ROR'); ld('v6502_ILL') #6
ld('v6502_JMP2');ld('v6502_ADC'); ld('v6502_ROR'); ld('v6502_ILL') #6
ld('v6502_BVS'); ld('v6502_ADC'); ld('v6502_ILL'); ld('v6502_ILL') #6 $70
ld('v6502_ILL'); ld('v6502_ADC'); ld('v6502_ROR'); ld('v6502_ILL') #6
ld('v6502_SEI'); ld('v6502_ADC'); ld('v6502_ILL'); ld('v6502_ILL') #6
ld('v6502_ILL'); ld('v6502_ADC'); ld('v6502_ROR'); ld('v6502_ILL') #6
ld('v6502_ILL'); ld('v6502_STA'); ld('v6502_ILL'); ld('v6502_ILL') #6 $80
ld('v6502_STY'); ld('v6502_STA'); ld('v6502_STX'); ld('v6502_ILL') #6
ld('v6502_DEY'); ld('v6502_ILL'); ld('v6502_TXA'); ld('v6502_ILL') #6
ld('v6502_STY'); ld('v6502_STA'); ld('v6502_STX'); ld('v6502_ILL') #6
ld('v6502_BCC'); ld('v6502_STA'); ld('v6502_ILL'); ld('v6502_ILL') #6 $90
ld('v6502_STY'); ld('v6502_STA'); ld('v6502_STX2');ld('v6502_ILL') #6
ld('v6502_TYA'); ld('v6502_STA'); ld('v6502_TXS'); ld('v6502_ILL') #6
ld('v6502_ILL'); ld('v6502_STA'); ld('v6502_ILL'); ld('v6502_ILL') #6
ld('v6502_LDY'); ld('v6502_LDA'); ld('v6502_LDX'); ld('v6502_ILL') #6 $A0
ld('v6502_LDY'); ld('v6502_LDA'); ld('v6502_LDX'); ld('v6502_ILL') #6
ld('v6502_TAY'); ld('v6502_LDA'); ld('v6502_TAX'); ld('v6502_ILL') #6
ld('v6502_LDY'); ld('v6502_LDA'); ld('v6502_LDX'); ld('v6502_ILL') #6
ld('v6502_BCS'); ld('v6502_LDA'); ld('v6502_ILL'); ld('v6502_ILL') #6 $B0
ld('v6502_LDY'); ld('v6502_LDA'); ld('v6502_LDX2');ld('v6502_ILL') #6
ld('v6502_CLV'); ld('v6502_LDA'); ld('v6502_TSX'); ld('v6502_ILL') #6
ld('v6502_LDY'); ld('v6502_LDA'); ld('v6502_LDX'); ld('v6502_ILL') #6
ld('v6502_CPY'); ld('v6502_CMP'); ld('v6502_ILL'); ld('v6502_ILL') #6 $C0
ld('v6502_CPY'); ld('v6502_CMP'); ld('v6502_DEC'); ld('v6502_ILL') #6
ld('v6502_INY'); ld('v6502_CMP'); ld('v6502_DEX'); ld('v6502_ILL') #6
ld('v6502_CPY'); ld('v6502_CMP'); ld('v6502_DEC'); ld('v6502_ILL') #6
ld('v6502_BNE'); ld('v6502_CMP'); ld('v6502_ILL'); ld('v6502_ILL') #6 $D0
ld('v6502_ILL'); ld('v6502_CMP'); ld('v6502_DEC'); ld('v6502_ILL') #6
ld('v6502_CLD'); ld('v6502_CMP'); ld('v6502_ILL'); ld('v6502_ILL') #6
ld('v6502_ILL'); ld('v6502_CMP'); ld('v6502_DEC'); ld('v6502_ILL') #6
ld('v6502_CPX'); ld('v6502_SBC'); ld('v6502_ILL'); ld('v6502_ILL') #6 $E0
ld('v6502_CPX'); ld('v6502_SBC'); ld('v6502_INC'); ld('v6502_ILL') #6
ld('v6502_INX'); ld('v6502_SBC'); ld('v6502_NOP'); ld('v6502_ILL') #6
ld('v6502_CPX'); ld('v6502_SBC'); ld('v6502_INC'); ld('v6502_ILL') #6
ld('v6502_BEQ'); ld('v6502_SBC'); ld('v6502_ILL'); ld('v6502_ILL') #6 $F0
ld('v6502_ILL'); ld('v6502_SBC'); ld('v6502_INC'); ld('v6502_ILL') #6
ld('v6502_SED'); ld('v6502_SBC'); ld('v6502_ILL'); ld('v6502_ILL') #6
ld('v6502_ILL'); ld('v6502_SBC'); ld('v6502_INC')                  #6
bra(AC)                         #6,7 Dispatch into next page
# --- Page boundary ---
align(0x100,size=0x100)
ld(hi('v6502_next'),Y)          #8 Handy for instructions that don't clobber Y

label('v6502_ADC')
assert pc()&255 == 1
assert v6502_Cflag == 1
assert v6502_Vemu == 128
ld([v6502_ADH],Y)               #9 Must be at page offset 1, so A=1
anda([v6502_P])                 #10 Carry in (AC=1 because lo('v6502_ADC')=1)
adda([v6502_A])                 #11 Sum
beq('.adc14')                   #12 Danger zone for dropping a carry
adda([Y,X])                     #13
st([v6502_Qz])                  #14 Z flag, don't overwrite left-hand side (A) yet
st([v6502_Qn])                  #15 N flag
xora([v6502_A])                 #16 V flag, (Q^A) & (B^Q) & 0x80
st([v6502_A])                   #17
ld([Y,X])                       #18
xora([v6502_Qz])                #19
anda([v6502_A])                 #20
anda(0x80)                      #21
st([v6502_Tmp])                 #22
ld([v6502_Qz])                  #23 Update A
st([v6502_A])                   #24
bmi('.adc27')                   #25 C flag
suba([Y,X])                     #26
bra('.adc29')                   #27
ora([Y,X])                      #28
label('.adc27')
anda([Y,X])                     #27
nop()                           #28
label('.adc29')
anda(0x80,X)                    #29
ld([v6502_P])                   #30 Update P
anda(~v6502_Vemu&~v6502_Cflag)  #31
ora([X])                        #32
ora([v6502_Tmp])                #33
st([v6502_P])                   #34
ld(hi('v6502_next'),Y)          #35
jmp(Y,'v6502_next')             #36
ld(-38/2)                       #37
# Cin=1, A=$FF, B=$DD --> Result=$DD, Cout=1, V=0
# Cin=0, A=$00, B=$DD --> Result=$DD, Cout=0, V=0
label('.adc14')
st([v6502_A])                   #14 Special case
st([v6502_Qz])                  #15 Z flag
st([v6502_Qn])                  #16 N flag
ld([v6502_P])                   #17
anda(0x7f)                      #18 V=0, keep C
st([v6502_P])                   #19
ld(hi('v6502_next'),Y)          #20
ld(-24/2)                       #21
jmp(Y,'v6502_next')             #22
#nop()                          #23 Overlap
#
label('v6502_SBC')
# No matter how hard we try, v6502_SBC always comes out a lot clumsier
# than v6502_ADC. And that one already barely fits in 38 cycles and is
# hard to follow. So we use a hack: transmorph our SBC into an ADC with
# inverted operand, and then dispatch again. Simple and effective.
ld([v6502_ADH],Y)               #9,24
ld([Y,X])                       #10
xora(255)                       #11 Invert right-hand side operand
st([v6502_BI])                  #12 Park modified operand for v6502_ADC
ld(v6502_BI&255)                #13 Address of BI
st([v6502_ADL],X)               #14
ld(v6502_BI>>8)                 #15
st([v6502_ADH])                 #16
ld(0x69)                        #17 ADC #$xx (Any ADC opcode will do)
st([v6502_IR])                  #18
ld(hi('v6502_check'),Y)         #20 Go back to time check before dispatch
jmp(Y,'v6502_check')            #20
ld(-22/2)                       #21

# Carry calculation table
#   L7 R7 C7   RX UC SC
#   -- -- -- | -- -- --
#    0  0  0 |  0  0  0
#    0  0  1 |  0  0  0
#    1  0  0 |  0  1 +1
#    1  0  1 |  0  0  0
#    0  1  0 | -1  1  0
#    0  1  1 | -1  0 -1
#    1  1  0 | -1  1  0
#    1  1  1 | -1  1  0
#   -- -- -- | -- -- --
#    ^  ^  ^    ^  ^  ^
#    |  |  |    |  |  `--- Carry of unsigned L + signed R: SC = RX + UC
#    |  |  |    |  `----- Carry of unsigned L + unsigned R: UC = C7 ? L7&R7 : L7|R7
#    |  |  |    `------- Sign extension of signed R
#    |  |  `--------- MSB of unextended L + R
#    |  `----------- MSB of right operand R
#    `------------- MSB of left operand L

label('v6502_CLC')
ld([v6502_P])                   #9
bra('.sec12')                   #10
label('v6502_SEC')
anda(~v6502_Cflag)              #9,11 Overlap
ld([v6502_P])                   #10
ora(v6502_Cflag)                #11
label('.sec12')
st([v6502_P])                   #12
nop()                           #13
label('.next14')
jmp(Y,'v6502_next')             #14
ld(-16/2)                       #15

label('v6502_BPL')
ld([v6502_Qn])                  #9
bmi('.next12')                  #10
bpl('.branch13')                #11
#nop()                          #12 Overlap
label('v6502_BMI')
ld([v6502_Qn])                  #9,12
bpl('.next12')                  #10
bmi('.branch13')                #11
#nop()                          #12 Overlap
label('v6502_BVC')
ld([v6502_P])                   #9,12
anda(v6502_Vemu)                #10
beq('.branch13')                #11
bne('.next14')                  #12
#nop()                          #13 Overlap
label('v6502_BVS')
ld([v6502_P])                   #9,13
anda(v6502_Vemu)                #10
bne('.branch13')                #11
beq('.next14')                  #12
#nop()                          #13 Overlap
label('v6502_BCC')
ld([v6502_P])                   #9,13
anda(v6502_Cflag)               #10
beq('.branch13')                #11
bne('.next14')                  #12
#nop()                          #13 Overlap
label('v6502_BCS')
ld([v6502_P])                   #9,13
anda(v6502_Cflag)               #10
bne('.branch13')                #11
beq('.next14')                  #12
#nop()                          #13 Overlap
label('v6502_BNE')
ld([v6502_Qz])                  #9,13
beq('.next12')                  #10
bne('.branch13')                #11
#nop()                          #12 Overlap
label('v6502_BEQ')
ld([v6502_Qz])                  #9,12
bne('.next12')                  #10
beq('.branch13')                #11
#nop()                          #12 Overlap
label('.branch13')
ld([v6502_ADL])                 #13,12 PC + offset
adda([v6502_PCL])               #14
st([v6502_PCL])                 #15
bmi('.bra0')                    #16 Carry
suba([v6502_ADL])               #17
bra('.bra1')                    #18
ora([v6502_ADL])                #19
label('.bra0')
anda([v6502_ADL])               #18
nop()                           #19
label('.bra1')
anda(0x80,X)                    #20
ld([X])                         #21
adda([v6502_ADH])               #22
adda([v6502_PCH])               #23
st([v6502_PCH])                 #24
nop()                           #25
jmp(Y,'v6502_next')             #26
ld(-28/2)                       #27

label('v6502_INX')
nop()                           #9
ld([v6502_X])                   #10
adda(1)                         #11
st([v6502_X])                   #12
label('.inx13')
st([v6502_Qz])                  #13 Z flag
st([v6502_Qn])                  #14 N flag
ld(-18/2)                       #15
jmp(Y,'v6502_next')             #16
nop()                           #17

label('.next12')
jmp(Y,'v6502_next')             #12
ld(-14/2)                       #13

label('v6502_DEX')
ld([v6502_X])                   #9
suba(1)                         #10
bra('.inx13')                   #11
st([v6502_X])                   #12

label('v6502_INY')
ld([v6502_Y])                   #9
adda(1)                         #10
bra('.inx13')                   #11
st([v6502_Y])                   #12

label('v6502_DEY')
ld([v6502_Y])                   #9
suba(1)                         #10
bra('.inx13')                   #11
st([v6502_Y])                   #12

label('v6502_NOP')
ld(-12/2)                       #9
jmp(Y,'v6502_next')             #10
#nop()                          #11 Overlap
#
label('v6502_AND')
ld([v6502_ADH],Y)               #9,11
ld([v6502_A])                   #10
bra('.eor13')                   #11
anda([Y,X])                     #12

label('v6502_ORA')
ld([v6502_ADH],Y)               #9
ld([v6502_A])                   #10
bra('.eor13')                   #11
label('v6502_EOR')
ora([Y,X])                      #12,9
#
#label('v6502_EOR')
#nop()                          #9 Overlap
ld([v6502_ADH],Y)               #10
ld([v6502_A])                   #11
xora([Y,X])                     #12
label('.eor13')
st([v6502_A])                   #13
st([v6502_Qz])                  #14 Z flag
st([v6502_Qn])                  #15 N flag
ld(hi('v6502_next'),Y)          #16
ld(-20/2)                       #17
jmp(Y,'v6502_next')             #18
#nop()                          #19 Overlap
#
label('v6502_JMP1')
ld(hi('v6502_jmp1'),Y)          #9,19 JMP $DDDD
jmp(Y,'v6502_jmp1')             #10
#nop()                          #11 Overlap
label('v6502_JMP2')
ld(hi('v6502_jmp2'),Y)          #9 JMP ($DDDD)
jmp(Y,'v6502_jmp2')             #10
#nop()                          #11 Overlap
label('v6502_JSR')
ld([v6502_S])                   #9,11
suba(2)                         #10
st([v6502_S],X)                 #11
ld(v6502_Stack>>8,Y)            #12
ld([v6502_PCH])                 #13 Let ADL,ADH point to L operand
st([v6502_ADH])                 #14
ld([v6502_PCL])                 #15
st([v6502_ADL])                 #16
adda(1)                         #17 Push ++PC
st([v6502_PCL])                 #18 Let PCL,PCH point to H operand
st([Y,Xpp])                     #19
beq(pc()+3)                     #20
bra(pc()+3)                     #21
ld(0)                           #22
ld(1)                           #22(!)
adda([v6502_PCH])               #23
st([v6502_PCH])                 #24
st([Y,X])                       #25
ld([v6502_ADL],X)               #26 Fetch L
ld([v6502_ADH],Y)               #27
ld([Y,X])                       #28
ld([v6502_PCL],X)               #29 Fetch H
st([v6502_PCL])                 #30
ld([v6502_PCH],Y)               #31
ld([Y,X])                       #32
st([v6502_PCH])                 #33
ld(hi('v6502_next'),Y)          #34
ld(-38/2)                       #35
jmp(Y,'v6502_next')             #36
#nop()                          #37 Overlap
#
label('v6502_INC')
ld(hi('v6502_inc'),Y)           #9,37
jmp(Y,'v6502_inc')              #10
#nop()                          #11 Overlap
label('v6502_LDA')
ld(hi('v6502_lda'),Y)           #9,11
jmp(Y,'v6502_lda')              #10
#nop()                          #11 Overlap
label('v6502_LDX')
ld(hi('v6502_ldx'),Y)           #9,11
jmp(Y,'v6502_ldx')              #10
#nop()                          #11 Overlap
label('v6502_LDX2')
ld(hi('v6502_ldx2'),Y)          #9,11
jmp(Y,'v6502_ldx2')             #10
#nop()                          #11 Overlap
label('v6502_LDY')
ld(hi('v6502_ldy'),Y)           #9,11
jmp(Y,'v6502_ldy')              #10
#nop()                          #11 Overlap
label('v6502_STA')
ld(hi('v6502_sta'),Y)           #9,11
jmp(Y,'v6502_sta')              #10
#nop()                          #11 Overlap
label('v6502_STX')
ld(hi('v6502_stx'),Y)           #9,11
jmp(Y,'v6502_stx')              #10
#nop()                          #11 Overlap
label('v6502_STX2')
ld(hi('v6502_stx2'),Y)          #9,11
jmp(Y,'v6502_stx2')             #10
#nop()                          #11 Overlap
label('v6502_STY')
ld(hi('v6502_sty'),Y)           #9,11
jmp(Y,'v6502_sty')              #10
#nop()                          #11 Overlap
label('v6502_TAX')
ld(hi('v6502_tax'),Y)           #9,11
jmp(Y,'v6502_tax')              #10
#nop()                          #11 Overlap
label('v6502_TAY')
ld(hi('v6502_tay'),Y)           #9,11
jmp(Y,'v6502_tay')              #10
#nop()                          #11 Overlap
label('v6502_TXA')
ld(hi('v6502_txa'),Y)           #9,11
jmp(Y,'v6502_txa')              #10
#nop()                          #11 Overlap
label('v6502_TYA')
ld(hi('v6502_tya'),Y)           #9,11
jmp(Y,'v6502_tya')              #10
#nop()                          #11 Overlap
label('v6502_CLV')
ld(hi('v6502_clv'),Y)           #9,11
jmp(Y,'v6502_clv')              #10
#nop()                          #11 Overlap
label('v6502_RTI')
ld(hi('v6502_rti'),Y)           #9,11
jmp(Y,'v6502_rti')              #10
#nop()                          #11 Overlap
label('v6502_ROR')
ld(hi('v6502_ror'),Y)           #9,11
jmp(Y,'v6502_ror')              #10
#nop()                          #11 Overlap
label('v6502_LSR')
ld(hi('v6502_lsr'),Y)           #9,11
jmp(Y,'v6502_lsr')              #10
#nop()                          #11 Overlap
label('v6502_PHA')
ld(hi('v6502_pha'),Y)           #9,11
jmp(Y,'v6502_pha')              #10
#nop()                          #11 Overlap
label('v6502_CLI')
ld(hi('v6502_cli'),Y)           #9,11
jmp(Y,'v6502_cli')              #10
#nop()                          #11 Overlap
label('v6502_RTS')
ld(hi('v6502_rts'),Y)           #9,11
jmp(Y,'v6502_rts')              #10
#nop()                          #11 Overlap
label('v6502_PLA')
ld(hi('v6502_pla'),Y)           #9,11
jmp(Y,'v6502_pla')              #10
#nop()                          #11 Overlap
label('v6502_SEI')
ld(hi('v6502_sei'),Y)           #9,11
jmp(Y,'v6502_sei')              #10
#nop()                          #11 Overlap
label('v6502_TXS')
ld(hi('v6502_txs'),Y)           #9,11
jmp(Y,'v6502_txs')              #10
#nop()                          #11 Overlap
label('v6502_TSX')
ld(hi('v6502_tsx'),Y)           #9,11
jmp(Y,'v6502_tsx')              #10
#nop()                          #11 Overlap
label('v6502_CPY')
ld(hi('v6502_cpy'),Y)           #9,11
jmp(Y,'v6502_cpy')              #10
#nop()                          #11 Overlap
label('v6502_CMP')
ld(hi('v6502_cmp'),Y)           #9,11
jmp(Y,'v6502_cmp')              #10
#nop()                          #11 Overlap
label('v6502_DEC')
ld(hi('v6502_dec'),Y)           #9,11
jmp(Y,'v6502_dec')              #10
#nop()                          #11 Overlap
label('v6502_CLD')
ld(hi('v6502_cld'),Y)           #9,11
jmp(Y,'v6502_cld')              #10
#nop()                          #11 Overlap
label('v6502_CPX')
ld(hi('v6502_cpx'),Y)           #9,11
jmp(Y,'v6502_cpx')              #10
#nop()                          #11 Overlap
label('v6502_ASL')
ld(hi('v6502_asl'),Y)           #9,11
jmp(Y,'v6502_asl')              #10
#nop()                          #11 Overlap
label('v6502_PHP')
ld(hi('v6502_php'),Y)           #9,11
jmp(Y,'v6502_php')              #10
#nop()                          #11 Overlap
label('v6502_BIT')
ld(hi('v6502_bit'),Y)           #9
jmp(Y,'v6502_bit')              #10
#nop()                          #11 Overlap
label('v6502_ROL')
ld(hi('v6502_rol'),Y)           #9
jmp(Y,'v6502_rol')              #10
#nop()                          #11 Overlap
label('v6502_PLP')
ld(hi('v6502_plp'),Y)           #9
jmp(Y,'v6502_plp')              #10
#nop()                          #11 Overlap
label('v6502_SED')              # Decimal mode not implemented
ld(hi('v6502_sed'),Y)           #9,11
jmp(Y,'v6502_sed')              #10
#nop()                          #11 Overlap
label('v6502_ILL') # All illegal opcodes map to BRK, except $FF which will crash
label('v6502_BRK')
ld(hi('v6502_brk'),Y)           #9
jmp(Y,'v6502_brk')              #10
#nop()                          #11 Overlap

while pc()&255 < 255:
  nop()

# `v6502_RESUME' is the interpreter's secondary entry point for when
# the opcode and operands were already fetched, just before the last hPulse.
# It must be at $xxff, prefably somewhere in v6502's own code pages.
label('v6502_RESUME')
assert (pc()&255) == 255
suba(v6502_adjust)              #0,11 v6502 secondary entry point
# --- Page boundary ---
align(0x100,size=0x200)
st([vTicks])                    #1
ld([v6502_ADL],X)               #2
ld(hi('v6502_execute'),Y)       #3
jmp(Y,[v6502_IR])               #4
bra(255)                        #5

label('v6502_dec')
ld([v6502_ADH],Y)               #12
ld([Y,X])                       #13
suba(1)                         #14
st([Y,X])                       #15
st([v6502_Qz])                  #16 Z flag
st([v6502_Qn])                  #17 N flag
ld(hi('v6502_next'),Y)          #18
ld(-22/2)                       #19
jmp(Y,'v6502_next')             #20
#nop()                          #21 Overlap
#
label('v6502_inc')
ld([v6502_ADH],Y)               #12,22
ld([Y,X])                       #13
adda(1)                         #14
st([Y,X])                       #15
st([v6502_Qz])                  #16 Z flag
st([v6502_Qn])                  #17 N flag
ld(hi('v6502_next'),Y)          #18
ld(-22/2)                       #19
jmp(Y,'v6502_next')             #20
nop()                           #21

label('v6502_lda')
nop()                           #12
ld([v6502_ADH],Y)               #13
ld([Y,X])                       #14
st([v6502_A])                   #15
label('.lda16')
st([v6502_Qz])                  #16 Z flag
st([v6502_Qn])                  #17 N flag
nop()                           #18
ld(hi('v6502_next'),Y)          #19
jmp(Y,'v6502_next')             #20
ld(-22/2)                       #21

label('v6502_ldx')
ld([v6502_ADH],Y)               #12
ld([Y,X])                       #13
bra('.lda16')                   #14
st([v6502_X])                   #15

label('v6502_ldy')
ld([v6502_ADH],Y)               #12
ld([Y,X])                       #13
bra('.lda16')                   #14
st([v6502_Y])                   #15

label('v6502_ldx2')
ld([v6502_ADL])                 #12 Special case $B6: LDX $DD,Y
suba([v6502_X])                 #13 Undo X offset
adda([v6502_Y],X)               #14 Apply Y instead
ld([X])                         #15
st([v6502_X])                   #16
st([v6502_Qz])                  #17 Z flag
st([v6502_Qn])                  #18 N flag
ld(hi('v6502_next'),Y)          #19
jmp(Y,'v6502_next')             #20
ld(-22/2)                       #21

label('v6502_sta')
ld([v6502_ADH],Y)               #12
ld([v6502_A])                   #13
st([Y,X])                       #14
ld(hi('v6502_next'),Y)          #15
jmp(Y,'v6502_next')             #16
ld(-18/2)                       #17

label('v6502_stx')
ld([v6502_ADH],Y)               #12
ld([v6502_X])                   #13
st([Y,X])                       #14
ld(hi('v6502_next'),Y)          #15
jmp(Y,'v6502_next')             #16
ld(-18/2)                       #17

label('v6502_stx2')
ld([v6502_ADL])                 #12 Special case $96: STX $DD,Y
suba([v6502_X])                 #13 Undo X offset
adda([v6502_Y],X)               #14 Apply Y instead
ld([v6502_X])                   #15
st([X])                         #16
ld(hi('v6502_next'),Y)          #17
jmp(Y,'v6502_next')             #18
ld(-20/2)                       #19

label('v6502_sty')
ld([v6502_ADH],Y)               #12
ld([v6502_Y])                   #13
st([Y,X])                       #14
ld(hi('v6502_next'),Y)          #15
jmp(Y,'v6502_next')             #16
label('v6502_tax')
ld(-18/2)                       #17,12
#
#label('v6502_tax')
#nop()                          #12 Overlap
ld([v6502_A])                   #13
st([v6502_X])                   #14
label('.tax15')
st([v6502_Qz])                  #15 Z flag
st([v6502_Qn])                  #16 N flag
ld(hi('v6502_next'),Y)          #17
jmp(Y,'v6502_next')             #18
label('v6502_tsx')
ld(-20/2)                       #19
#
#label('v6502_tsx')
#nop()                          #12 Overlap
ld([v6502_S])                   #13
suba(1)                         #14 Shift down on export
st([v6502_X])                   #15
label('.tsx16')
st([v6502_Qz])                  #16 Z flag
st([v6502_Qn])                  #17 N flag
nop()                           #18
ld(hi('v6502_next'),Y)          #19
jmp(Y,'v6502_next')             #20
ld(-22/2)                       #21

label('v6502_txs')
ld([v6502_X])                   #12
adda(1)                         #13 Shift up on import
bra('.tsx16')                   #14
st([v6502_S])                   #15

label('v6502_tay')
ld([v6502_A])                   #12
bra('.tax15')                   #13
st([v6502_Y])                   #14

label('v6502_txa')
ld([v6502_X])                   #12
bra('.tax15')                   #13
st([v6502_A])                   #14

label('v6502_tya')
ld([v6502_Y])                   #12
bra('.tax15')                   #13
st([v6502_A])                   #14

label('v6502_cli')
ld([v6502_P])                   #12
bra('.clv15')                   #13
anda(~v6502_Iflag)              #14

label('v6502_sei')
ld([v6502_P])                   #12
bra('.clv15')                   #13
ora(v6502_Iflag)                #14

label('v6502_cld')
ld([v6502_P])                   #12
bra('.clv15')                   #13
anda(~v6502_Dflag)              #14

label('v6502_sed')
ld([v6502_P])                   #12
bra('.clv15')                   #13
label('v6502_clv')
ora(v6502_Dflag)                #14,12 Overlap
#
#label('v6502_clv')
#nop()                          #12
ld([v6502_P])                   #13
anda(~v6502_Vemu)               #14
label('.clv15')
st([v6502_P])                   #15
ld(hi('v6502_next'),Y)          #16
ld(-20/2)                       #17
jmp(Y,'v6502_next')             #18
label('v6502_bit')
nop()                           #19,12
#
#label('v6502_bit')
#nop()                          #12 Overlap
ld([v6502_ADL],X)               #13
ld([v6502_ADH],Y)               #14
ld([Y,X])                       #15
st([v6502_Qn])                  #16 N flag
anda([v6502_A])                 #17 This is a reason we keep N and Z in separate bytes
st([v6502_Qz])                  #18 Z flag
ld([v6502_P])                   #19
anda(~v6502_Vemu)               #20
st([v6502_P])                   #21
ld([Y,X])                       #22
adda(AC)                        #23
anda(v6502_Vemu)                #24
ora([v6502_P])                  #25
st([v6502_P])                   #26 Update V
ld(hi('v6502_next'),Y)          #27
jmp(Y,'v6502_next')             #28
ld(-30/2)                       #29

label('v6502_rts')
ld([v6502_S])                   #12
ld(AC,X)                        #13
adda(2)                         #14
st([v6502_S])                   #15
ld(0,Y)                         #16
ld([Y,X])                       #17
st([Y,Xpp])                     #18 Just X++
adda(1)                         #19
st([v6502_PCL])                 #20
beq(pc()+3)                     #21
bra(pc()+3)                     #22
ld(0)                           #23
ld(1)                           #23(!)
adda([Y,X])                     #24
st([v6502_PCH])                 #25
nop()                           #26
ld(hi('v6502_next'),Y)          #27
jmp(Y,'v6502_next')             #28
ld(-30/2)                       #29

label('v6502_php')
ld([v6502_S])                   #12
suba(1)                         #13
st([v6502_S],X)                 #14
ld([v6502_P])                   #15
anda(~v6502_Vflag&~v6502_Zflag) #16 Keep Vemu,B,D,I,C
bpl(pc()+3)                     #17 V to bit 6 and clear N
bra(pc()+2)                     #18
xora(v6502_Vflag^v6502_Vemu)    #19
st([X])                         #19,20
ld([v6502_Qz])                  #21 Z flag
beq(pc()+3)                     #22
bra(pc()+3)                     #23
ld(0)                           #24
ld(v6502_Zflag)                 #24(!)
ora([X])                        #25
st([X])                         #26
ld([v6502_Qn])                  #27 N flag
anda(0x80)                      #28
ora([X])                        #29
ora(v6502_Uflag)                #30 Unused bit
st([X])                         #31
nop()                           #32
ld(hi('v6502_next'),Y)          #33
jmp(Y,'v6502_next')             #34
ld(-36/2)                       #35

label('v6502_cpx')
bra('.cmp14')                   #12
ld([v6502_X])                   #13

label('v6502_cpy')
bra('.cmp14')                   #12
label('v6502_cmp')
ld([v6502_Y])                   #13,12
#
#label('v6502_cmp')             #12 Overlap
assert v6502_Cflag == 1
ld([v6502_A])                   #13
label('.cmp14')
ld([v6502_ADH],Y)               #14
bmi('.cmp17')                   #15 Carry?
suba([Y,X])                     #16
st([v6502_Qz])                  #17 Z flag
st([v6502_Qn])                  #18 N flag
bra('.cmp21')                   #19
ora([Y,X])                      #20
label('.cmp17')
st([v6502_Qz])                  #17 Z flag
st([v6502_Qn])                  #18 N flag
anda([Y,X])                     #19
nop()                           #20
label('.cmp21')
xora(0x80)                      #21
anda(0x80,X)                    #22 Move carry to bit 0
ld([v6502_P])                   #23 C flag
anda(~1)                        #24
ora([X])                        #25
st([v6502_P])                   #26
ld(hi('v6502_next'),Y)          #27
jmp(Y,'v6502_next')             #28
ld(-30/2)                       #29

label('v6502_plp')
assert v6502_Nflag == 128
assert 2*v6502_Vflag == v6502_Vemu
ld([v6502_S])                   #12
ld(AC,X)                        #13
adda(1)                         #14
st([v6502_S])                   #15
ld([X])                         #16
st([v6502_Qn])                  #17 N flag
anda(v6502_Zflag)               #18
xora(v6502_Zflag)               #19
st([v6502_Qz])                  #20 Z flag
ld([X])                         #21
anda(~v6502_Vemu)               #22 V to bit 7
adda(v6502_Vflag)               #23
st([v6502_P])                   #24 All other flags
ld(hi('v6502_next'),Y)          #25
jmp(Y,'v6502_next')             #26
ld(-28/2)                       #27

label('v6502_rti')
ld([v6502_S])                   #12
ld(AC,X)                        #13
adda(3)                         #14
st([v6502_S])                   #15
ld([X])                         #16
st([v6502_Qn])                  #17 N flag
anda(v6502_Zflag)               #18
xora(v6502_Zflag)               #19
st([v6502_Qz])                  #20 Z flag
ld(0,Y)                         #21
ld([Y,X])                       #22
st([Y,Xpp])                     #23 Just X++
anda(~v6502_Vemu)               #24 V to bit 7
adda(v6502_Vflag)               #25
st([v6502_P])                   #26 All other flags
ld([Y,X])                       #27
st([Y,Xpp])                     #28 Just X++
st([v6502_PCL])                 #29
ld([Y,X])                       #30
st([v6502_PCH])                 #31
nop()                           #32
ld(hi('v6502_next'),Y)          #33
jmp(Y,'v6502_next')             #34
ld(-36/2)                       #35

#-----------------------------------------------------------------------
#       Extended vertical blank logic: interrupts
#-----------------------------------------------------------------------
align(0x100)

# Check if an IRQ handler is defined
label('vBlankFirst#78')
ld([Y,vIRQ_v5])                 #78
ora([Y,vIRQ_v5+1])              #79
bne('vBlankFirst#82')           #80
ld([vPC])                       #81
runVcpu(186-82-extra,           #82 Application cycles (scan line 0)
    '---D line 0 timeout but no irq',
    returnTo='vBlankFirst#186')

label('vBlankFirst#82')
st([0x30])                      #82 Save vPC
ld([vPC+1])                     #83
st([0x31])                      #84
ld([vAC])                       #85 Save vAC
st([0x32])                      #86
ld([vAC+1])                     #87
st([0x33])                      #88
ld([Y,vIRQ_v5])                 #89 Set vPC to vIRQ
suba(2)                         #90
st([vPC])                       #91
ld([Y,vIRQ_v5+1])               #92
st([vPC+1])                     #93
ld([vCpuSelect])                #94 Handler must save this if needed
st([vAC+1])                     #95
ld(0)                           #96
st([vAC])                       #97
ld(hi('ENTER'))                 #98 Set vCpuSelect to ENTER (=regular vCPU)
st([vCpuSelect])                #99
runVcpu(186-100-extra,          #100 Application cycles (scan line 0)
    '---D line 0 timeout with irq',
    returnTo='vBlankFirst#186')

# vIRQ sequence WITH interpreter switch
label('vRTI#18')
ld([X])                         #18
st([vCpuSelect])                #19
ld([0x30])                      #20
st([vPC])                       #21
ld([0x31])                      #22
st([vPC+1])                     #23
ld([0x32])                      #24
st([vAC])                       #25
ld([0x33])                      #26
st([vAC+1])                     #27
nop()                           #0
ld(hi('RESYNC'),Y)              #1
jmp(Y,'RESYNC')                 #2
ld([vTicks])                    #3

# Entered last line of vertical blank (line 40)
label('vBlankLast#34')

#-----------------------------------------------------------------------
#       Extended vertical blank logic: game controller decoding
#-----------------------------------------------------------------------

# Game controller types
# TypeA: Based on 74LS165 shift register (not supported)
# TypeB: Based on CD4021B shift register (standard)
# TypeC: Based on priority encoder
#
# Notes:
# - TypeA was only used during development and first beta test, before ROM v1
# - TypeB appears as type A with negative logic levels
# - TypeB is the game controller type that comes with the original kit and ROM v1
# - TypeB is mimicked by BabelFish / Pluggy McPlugface
# - TypeB requires a prolonged /SER_LATCH, therefore vPulse is 8 scanlines, not 2
# - TypeB and TypeC can be sampled in the same scanline
# - TypeA is 1 scanline shifted as it looks at a different edge (XXX up or down?)
# - TypeC gives incomplete information: lower buttons overshadow higher ones
#
#       TypeC    Alias    Button TypeB
#       00000000  ^@   -> Right  11111110
#       00000001  ^A   -> Left   11111101
#       00000011  ^C   -> Down   11111011
#       00000111  ^G   -> Up     11110111
#       00001111  ^O   -> Start  11101111
#       00011111  ^_   -> Select 11011111
#       00111111  ?    -> B      10111111
#       01111111  DEL  -> A      01111111
#       11111111       -> (None) 11111111
#
#       Conversion formula:
#               f(x) := 254 - x

# Detect controller TypeC codes
ld([serialRaw])                 #34 if serialRaw in [0,1,3,7,15,31,63,127,255]
adda(1)                         #35
anda([serialRaw])               #36
bne('.buttons#39')              #37

# TypeC
ld([serialRaw])                 #38 [TypeC] if serialRaw < serialLast
adda(1)                         #39
anda([serialLast])              #40
bne('.buttons#43')              #41
ld(254)                         #42 then clear the selected bit
nop()                           #43
bra('.buttons#46')              #44
label('.buttons#43')
suba([serialRaw])               #43,45
anda([buttonState])             #44
st([buttonState])               #45
label('.buttons#46')
ld([serialRaw])                 #46 Set the lower bits
ora([buttonState])              #47
label('.buttons#48')
st([buttonState])               #48
ld([serialRaw])                 #49 Update serialLast for next pass
jmp(Y,'vBlankLast#52')          #50
st([serialLast])                #51

# TypeB
# pChange = pNew & ~pOld
# nChange = nNew | ~nOld {DeMorgan}
label('.buttons#39')
ld(255)                         #39 [TypeB] Bitwise edge-filter to detect button presses
xora([serialLast])              #40
ora([serialRaw])                #41 Catch button-press events
anda([buttonState])             #42 Keep active button presses
ora([serialRaw])                #43
nop()                           #44
nop()                           #45
bra('.buttons#48')              #46
nop()                           #47


#-----------------------------------------------------------------------
#       More SYS functions
#-----------------------------------------------------------------------

# SYS_Exec_88 implementation
label('sys_Exec')
st([vPC+1],Y)                   #18 Clear vPCH and Y
ld([vSP])                       #19 Place ROM loader below current stack pointer
suba(53+2)                      #20 (AC -> *+0) One extra word for PUSH
st([vTmp],X)                    #21
adda(-2)                        #22 (AC -> *-2)
st([vPC])                       #23
# Start of manually compiled vCPU section
st('PUSH',    [Y,Xpp])          #24 *+0
st('CALL',    [Y,Xpp])          #25 *+26 Fetch first byte
adda(33-(-2))                   #26 (AC -> *+33)
st(           [Y,Xpp])          #27 *+27
st('ST',      [Y,Xpp])          #28 *+3 Chunk copy loop
st(sysArgs+3, [Y,Xpp])          #29 *+4 High-address comes first
st('CALL',    [Y,Xpp])          #30 *+5
st(           [Y,Xpp])          #31 *+6
st('ST',      [Y,Xpp])          #32 *+7
st(sysArgs+2, [Y,Xpp])          #33 *+8 Then the low address
st('CALL',    [Y,Xpp])          #34 *+9
st(           [Y,Xpp])          #35 *+10
st('ST',      [Y,Xpp])          #36 *+11 Byte copy loop
st(sysArgs+4, [Y,Xpp])          #37 *+12 Byte count (0 means 256)
st('CALL',    [Y,Xpp])          #38 *+13
st(           [Y,Xpp])          #39 *+14
st('POKE',    [Y,Xpp])          #40 *+15
st(sysArgs+2, [Y,Xpp])          #41 *+16
st('INC',     [Y,Xpp])          #42 *+17
st(sysArgs+2, [Y,Xpp])          #43 *+18
st('LD',      [Y,Xpp])          #44 *+19
st(sysArgs+4, [Y,Xpp])          #45 *+20
st('SUBI',    [Y,Xpp])          #46 *+21
st(1,         [Y,Xpp])          #47 *+22
st('BCC',     [Y,Xpp])          #48 *+23
st('NE',      [Y,Xpp])          #49 *+24
adda(11-2-33)                   #50 (AC -> *+9)
st(           [Y,Xpp])          #51 *+25
st('CALL',    [Y,Xpp])          #52 *+26 Go to next block
adda(33-9)                      #53 (AC -> *+33)
st(           [Y,Xpp])          #54 *+27
st('BCC',     [Y,Xpp])          #55 *+28
st('NE',      [Y,Xpp])          #56 *+29
adda(3-2-33)                    #57 (AC -> *+1)
st(           [Y,Xpp])          #58 *+30
st('POP',     [Y,Xpp])          #59 *+31 End
st('RET',     [Y,Xpp])          #60 *+32
# Pointer constant pointing to the routine below (for use by CALL)
adda(35-1)                      #61 (AC -> *+35)
st(           [Y,Xpp])          #62 *+33
st(0,         [Y,Xpp])          #63 *+34
# Routine to read next byte from ROM and advance read pointer
st('LD',      [Y,Xpp])          #64 *+35 Test for end of ROM table
st(sysArgs+0, [Y,Xpp])          #65 *+36
st('XORI',    [Y,Xpp])          #66 *+37
st(251,       [Y,Xpp])          #67 *+38
st('BCC',     [Y,Xpp])          #68 *+39
st('NE',      [Y,Xpp])          #69 *+40
adda(46-2-35)                   #70 (AC -> *+44)
st(           [Y,Xpp])          #71 *+41
st('ST',      [Y,Xpp])          #72 *+42 Wrap to next ROM page
st(sysArgs+0, [Y,Xpp])          #73 *+43
st('INC',     [Y,Xpp])          #74 *+44
st(sysArgs+1, [Y,Xpp])          #75 *+45
st('LDW',     [Y,Xpp])          #76 *+46 Read next byte from ROM table
st(sysArgs+0, [Y,Xpp])          #77 *+47
st('LUP',     [Y,Xpp])          #78 *+48
st(0,         [Y,Xpp])          #79 *+49
st('INC',     [Y,Xpp])          #80 *+50 Increment read pointer
st(sysArgs+0, [Y,Xpp])          #81 *+51
st('RET',     [Y,Xpp])          #82 *+52 Return
# Return to interpreter
ld(hi('REENTER'),Y)             #83
jmp(Y,'REENTER')                #84
ld(-88/2)                       #85

# SYS_VDrawBits_134 implementation
label('sys_VDrawBits')
ld(0)                           #18
label('.sysVdb0')
st([vTmp])                      #19+i*25
adda([sysArgs+5],Y)             #20+i*25 Y=[sysPos+1]+[vTmp]
ld([sysArgs+2])                 #21+i*25 Select color
bmi(pc()+3)                     #22+i*25
bra(pc()+3)                     #23+i*25
ld([sysArgs+0])                 #24+i*25
ld([sysArgs+1])                 #24+i*25(!)
st([Y,X])                       #25+i*25 Draw pixel
ld([sysArgs+2])                 #26+i*25 Shift byte left
adda(AC)                        #27+i*25
st([sysArgs+2])                 #28+i*25
ld([vTmp])                      #29+i*25 Unrolled loop (once)
adda([sysArgs+5])               #31+i*25
adda(1,Y)                       #30+i*25 Y=[sysPos+1]+[vTmp]+1
ld([sysArgs+2])                 #32+i*25 Select color
bmi(pc()+3)                     #33+i*25
bra(pc()+3)                     #34+i*25
ld([sysArgs+0])                 #35+i*25
ld([sysArgs+1])                 #35+i*25(!)
st([Y,X])                       #36+i*25 Draw pixel
ld([sysArgs+2])                 #37+i*25 Shift byte left
adda(AC)                        #38+i*25
st([sysArgs+2])                 #39+i*25
ld([vTmp])                      #40+i*25 Loop counter
suba(6)                         #41+i*25
bne('.sysVdb0')                 #42+i*25
adda(8)                         #43+i*25 Steps of 2
ld(hi('REENTER'),Y)             #119
jmp(Y,'REENTER')                #120
ld(-124/2)                      #121

# SYS_ResetWaveforms_v4_50 implementation
label('sys_ResetWaveforms')
ld([vAC+0])                     #18 X=4i
adda(AC)                        #19
adda(AC,X)                      #20
ld([vAC+0])                     #21
st([Y,Xpp])                     #22 Sawtooth: T[4i+0] = i
anda(0x20)                      #23 Triangle: T[4i+1] = 2i if i<32 else 127-2i
bne(pc()+3)                     #24
ld([vAC+0])                     #25
bra(pc()+3)                     #26
adda([vAC+0])                   #26,27
xora(127)                       #27
st([Y,Xpp])                     #28
ld([vAC+0])                     #29 Pulse: T[4i+2] = 0 if i<32 else 63
anda(0x20)                      #30
bne(pc()+3)                     #31
bra(pc()+3)                     #32
ld(0)                           #33
ld(63)                          #33(!)
st([Y,Xpp])                     #34
ld([vAC+0])                     #35 Sawtooth: T[4i+3] = i
st([Y,X])                       #36
adda(1)                         #37 i += 1
st([vAC+0])                     #38
xora(64)                        #39 For 64 iterations
beq(pc()+3)                     #40
bra(pc()+3)                     #41
ld(-2)                          #42
ld(0)                           #42(!)
adda([vPC])                     #43
st([vPC])                       #44
ld(hi('REENTER'),Y)             #45
jmp(Y,'REENTER')                #46
ld(-50/2)                       #47

# SYS_ShuffleNoise_v4_46 implementation
label('sys_ShuffleNoise')
ld([vAC+0],X)                   #18 tmp = T[4j]
ld([Y,X])                       #19
st([vTmp])                      #20
ld([vAC+1],X)                   #21 T[4j] = T[4i]
ld([Y,X])                       #22
ld([vAC+0],X)                   #23
st([Y,X])                       #24
adda(AC)                        #25 j += T[4i]
adda(AC,)                       #26
adda([vAC+0])                   #27
st([vAC+0])                     #28
ld([vAC+1],X)                   #29 T[4i] = tmp
ld([vTmp])                      #30
st([Y,X])                       #31
ld([vAC+1])                     #32 i += 1
adda(4)                         #33
st([vAC+1])                     #34
beq(pc()+3)                     #35 For 64 iterations
bra(pc()+3)                     #36
ld(-2)                          #37
ld(0)                           #37(!)
adda([vPC])                     #38
st([vPC])                       #39
ld(hi('NEXTY'),Y)               #40
jmp(Y,'NEXTY')                  #41
ld(-44/2)                       #42


#-----------------------------------------------------------------------
#
#  $1300 ROM page 19/20: CopyMemory/CopyMemoryExt/ScanMemory
#
#-----------------------------------------------------------------------

align(0x100, size=0x100)

# SYS_CopyMemory_DEVROM_80 implementation

label('sys_CopyMemory')
ble('.sysCm#20')                     #18   goto burst6
suba(6)                              #19
bge('.sysCm#22')                     #20   goto burst6
ld([sysArgs+3],Y)                    #21
adda(3)                              #22
bge('.sysCm#25')                     #23   goto burst3
ld([sysArgs+2],X)                    #24

adda(2)                              #25   single
st([vAC])                            #26
ld([Y,X])                            #27
ld([sysArgs+1],Y)                    #28
ld([sysArgs+0],X)                    #29
st([Y,X])                            #30
ld([sysArgs+0])                      #31
adda(1)                              #32
st([sysArgs+0])                      #33
ld([sysArgs+2])                      #34
adda(1)                              #35
st([sysArgs+2])                      #36
ld([vAC])                            #37
beq(pc()+3)                          #38
bra(pc()+3)                          #39
ld(-2)                               #40
ld(0)                                #40!
adda([vPC])                          #41
st([vPC])                            #42
ld(hi('REENTER'),Y)                  #43
jmp(Y,'REENTER')                     #44
ld(-48/2)                            #45

label('.sysCm#25')
st([vAC])                            #25   burst3
for i in range(3):
  ld([Y,X])                            #26+3*i
  st([sysArgs+4+i])                    #27+3*i
  st([Y,Xpp]) if i<2 else None         #28+3*i
ld([sysArgs+1],Y)                    #34
ld([sysArgs+0],X)                    #35
for i in range(3):
  ld([sysArgs+4+i])                    #36+2*i
  st([Y,Xpp])                          #37+2*i
ld([sysArgs+0])                      #42
adda(3)                              #43
st([sysArgs+0])                      #44
ld([sysArgs+2])                      #45
adda(3)                              #46
st([sysArgs+2])                      #47
ld([vAC])                            #48
beq(pc()+3)                          #49
bra(pc()+3)                          #50
ld(-2)                               #51
ld(0)                                #51!
adda([vPC])                          #52
st([vPC])                            #53
ld(hi('NEXTY'),Y)                    #54
jmp(Y,'NEXTY')                       #55
ld(-58/2)                            #56

label('.sysCm#20')
nop()                                #20   burst6
ld([sysArgs+3],Y)                    #21
label('.sysCm#22')
st([vAC])                            #22   burst6
ld([sysArgs+2],X)                    #23
for i in range(6):
  ld([Y,X])                            #24+i*3
  st([vLR+i if i<2 else sysArgs+2+i])  #25+i*3
  st([Y,Xpp]) if i<5 else None         #26+i*3 if i<5
ld([sysArgs+1],Y)                    #41
ld([sysArgs+0],X)                    #42
for i in range(6):
  ld([vLR+i if i<2 else sysArgs+2+i])  #43+i*2
  st([Y,Xpp])                          #44+i*2
ld([sysArgs+0])                      #55
adda(6)                              #56
st([sysArgs+0])                      #57
ld([sysArgs+2])                      #58
adda(6)                              #59
st([sysArgs+2])                      #60

ld([vAC])                            #61
bne('.sysCm#64')                     #62
ld(hi('REENTER'),Y)                  #63
jmp(Y,'REENTER')                     #64
ld(-68/2)                            #65

label('.sysCm#64')
ld(-52/2)                            #64
adda([vTicks])                       #13 = 65 - 52
st([vTicks])                         #14
adda(min(0,maxTicks-(26+52)/2))      #15   could probably be min(0,maxTicks-(26+52)/2)
bge('sys_CopyMemory')                #16
ld([vAC])                            #17
ld(-2)                               #18   notime
adda([vPC])                          #19
st([vPC])                            #20
ld(hi('REENTER'),Y)                  #21
jmp(Y,'REENTER')                     #22
ld(-26/2)                            #23

# SYS_CopyMemoryExt_DEVROM_100 implementation

label('sys_CopyMemoryExt')

ld([Y,ctrlBits])                     #18
bne('.sysCme#21')                    #19
st([vTmp])                           #20
ld(255)                              #21 no expansion present
st([vAC])                            #22
st([vAC+1])                          #23
ld(hi('NEXTY'),Y)                    #24
jmp(Y,'NEXTY')                       #25
ld(-28/2)                            #26

label('.sysCme#21')
ld([vAC+1])                          #21
adda([vAC+1])                        #22
st([vLR])                            #23
adda([vLR])                          #24
xora([vTmp])                         #25
anda(0xc3)                           #26 Sanitize [$1f8] as well
xora([vTmp])                         #27
st([vTmp])                           #28 [vTmp] = source ctrl value
xora([vAC+1])                        #29
anda(0xc0)                           #30
xora([vTmp])                         #31
st([vLR])                            #32 [vLR] = destination ctrl value (nonono)

ld([vAC])                            #33
label('.sysCme#34')
ble('.sysCme#36')                    #34   goto burst5
suba(5)                              #35
bge('.sysCme#38')                    #36   goto burst5
ld([sysArgs+3],Y)                    #37
adda(4)                              #38

st([vAC])                            #39   single
ld([vTmp],X)                         #40
ctrl(X)                              #41
ld([sysArgs+2],X)                    #42
ld([Y,X])                            #43
ld([vLR],X)                          #44
ctrl(X)                              #45
ld([sysArgs+1],Y)                    #46
ld([sysArgs+0],X)                    #47
st([Y,X])                            #48
ld(hi(ctrlBits), Y)                  #49
ld([Y,ctrlBits])                     #50
ld(AC,X)                             #51
ctrl(X)                              #52
ld([sysArgs+0])                      #53
adda(1)                              #54
st([sysArgs+0])                      #55
ld([sysArgs+2])                      #56
adda(1)                              #57
st([sysArgs+2])                      #58
ld([vAC])                            #59
beq(pc()+3)                          #60
bra(pc()+3)                          #61
ld(-2)                               #62
ld(0)                                #62!
adda([vPC])                          #63
st([vPC])                            #64
ld(hi('REENTER'),Y)                  #65
jmp(Y,'REENTER')                     #66
ld(-70/2)                            #67

label('.sysCme#36')
nop()                                #36   burst5
ld([sysArgs+3],Y)                    #37
label('.sysCme#38')
st([vAC])                            #38   burst5
ld([vTmp],X)                         #39
ctrl(X)                              #40
ld([sysArgs+2],X)                    #41
for i in range(5):
  ld([Y,X])                            #42+i*3
  st([vLR+1 if i<1 else sysArgs+3+i])  #43+i*3
  st([Y,Xpp]) if i<4 else None         #44+i*3 if i<5
ld([vLR],X)                          #56
ctrl(X)                              #57
ld([sysArgs+1],Y)                    #58
ld([sysArgs+0],X)                    #59
for i in range(5):
  ld([vLR+1 if i<1 else sysArgs+3+i])  #60+i*2
  st([Y,Xpp])                          #61+i*2
ld([sysArgs+0])                      #70
adda(5)                              #71
st([sysArgs+0])                      #72
ld([sysArgs+2])                      #73
adda(5)                              #74
st([sysArgs+2])                      #75

ld([vAC])                            #76
bne('.sysCme#79')                    #77
ld(hi(ctrlBits), Y)                  #78
ld([Y,ctrlBits])                     #79
anda(0xfc,X)                         #80
ctrl(X)                              #81
ld([vTmp])                           #82 ALWAYS LOAD AFTER CTRL
ld(hi('REENTER'),Y)                  #83
jmp(Y,'REENTER')                     #84
ld(-88/2)                            #85

label('.sysCme#79')
nop()                                #79
ld(-52/2)                            #80
adda([vTicks])                       #29 = 81 - 52
st([vTicks])                         #30
adda(min(0,maxTicks-(46+52)/2))      #31  note use of maxTicks
bge('.sysCme#34')                    #32
ld([vAC])                            #33
ld(-2)                               #34  notime
adda([vPC])                          #35
st([vPC])                            #36
ld(hi(ctrlBits), Y)                  #37
ld([Y,ctrlBits])                     #38
anda(0xfc,X)                         #39
ctrl(X)                              #40
ld([vTmp])                           #41 ALWAYS LOAD AFTER CTRL
ld(hi('NEXTY'),Y)                    #42
jmp(Y,'NEXTY')                       #43
ld(-46/2)                            #44  max: 46 + 52 = 98 cycles



align(0x100, size=0x100)

# SYS_ScanMemory_DEVROM_50 implementation

label('sys_ScanMemory')
ld([sysArgs+0],X)                    #18
ld([Y,X])                            #19
label('.sysSme#20')
xora([sysArgs+2])                    #20
beq('.sysSme#23')                    #21
ld([Y,X])                            #22
xora([sysArgs+3])                    #23
beq('.sysSme#26')                    #24
ld([sysArgs+0])                      #25
adda(1);                             #26
st([sysArgs+0],X)                    #27
ld([vAC])                            #28
suba(1)                              #29
beq('.sysSme#32')                    #30 return zero
st([vAC])                            #31
ld(-18/2)                            #14 = 32 - 18
adda([vTicks])                       #15
st([vTicks])                         #16
adda(min(0,maxTicks -(28+18)/2))     #17
bge('.sysSme#20')                    #18
ld([Y,X])                            #19
ld(-2)                               #20 restart
adda([vPC])                          #21
st([vPC])                            #22
ld(hi('REENTER'),Y)                  #23
jmp(Y,'REENTER')                     #24
ld(-28/2)                            #25

label('.sysSme#32')
st([vAC+1])                          #32 return zero
ld(hi('REENTER'),Y)                  #33
jmp(Y,'REENTER')                     #34
ld(-38/2)                            #35

label('.sysSme#23')
nop()                                #23 success
nop()                                #24
ld([sysArgs+0])                      #25
label('.sysSme#26')
st([vAC])                            #26 success
ld([sysArgs+1])                      #27
st([vAC+1])                          #28
ld(hi('REENTER'),Y)                  #29
jmp(Y,'REENTER')                     #30
ld(-34/2)                            #31


# SYS_OsCall_DEVROM_80 implementation

label('sys_OsCall')
ld([Y,ctrlBits])                     #18 check ctrlBits!=0
beq('.sysOsErr')                     #20
ld(-30/2)                            #21
ld([Y,ctrlBits])                     #22
anda(0xfc)                           #23
ora(0xc0,X)                          #24
ld(0x80,Y)                           #25
ctrl(X)                              #26 set bank 3
ld([Y,0])                            #27 check 'GTOS' signature in 0x8000
xora(0x47)                           #28 'G'
st([vTmp])                           #29
ld([Y,1])                            #30
xora(0x54)                           #31 'T'
ora([vTmp])                          #32
st([vTmp])                           #33
ld([Y,2])                            #34
xora(0x4f)                           #35 'O'
ora([vTmp])                          #36
st([vTmp])                           #37
ld([Y,3])                            #38
xora(0x53)                           #39 'S'
ora([vTmp])                          #40
st([vTmp])                           #41
bne('.sysOsErrCtrl')                 #42
ld(-58/2)                            #43
ld([vPC])                            #44
adda(2)                              #45
st([vLR])                            #46 save return address
ld([vPC+1])                          #47
st([vLR+1])                          #48
ld([vAC],X)                          #49 load vector index
ld([Y,X])                            #50 low byte
st([Y,Xpp])                          #51 just increment
st([vTmp])                           #52
ld([Y,X])                            #53
bpl('.sysOsErrCtrl')                 #54
ld(-70/2)                            #55
ld([Y,X])                            #56
st([vPC+1])                          #57
ld([vTmp])                           #58
suba(2)                              #59
st([vPC])                            #60 warning: calling os routine
ld(hi('REENTER'),Y)                  #61   with [ctrlBits] matching the
jmp(Y,'REENTER')                     #62   previous bank, not bank 2.
ld(-66/2)                            #63

label('.sysOsErrCtrl')
st([vTmp])                          #T-14 vAC must contain (-T/2)
ld(hi(ctrlBits),Y)                  #T-13 reset bank
ld([Y, ctrlBits])                   #T-12
anda(0xfc, X)                       #T-11
ctrl(X)                             #T-10
ld([vTmp])                          #T-9
label('.sysOsErr')
st([vTmp])                          #T-8 vAC must contain (-T/2)
ld(0x80)                            #T-7
st([vAC])                           #T-6
st([vAC+1])                         #T-5
ld(hi('NEXTY'),Y)                   #T-4
jmp(Y,'NEXTY')                      #T-3
ld([vTmp])                          #T-2


#-----------------------------------------------------------------------
#
#  End of core
#
#-----------------------------------------------------------------------

align(0x100)

disableListing()

#-----------------------------------------------------------------------
#
#  Start of storage area
#
#-----------------------------------------------------------------------

# Export some zero page variables to GCL
# These constants were already loaded from interface.json.
# We're redefining them here to get a consistency check.
define('memSize',    memSize)
for i in range(3):
  define('entropy%d' % i, entropy+i)
define('videoY',     videoY)
define('frameCount', frameCount)
define('serialRaw',  serialRaw)
define('buttonState',buttonState)
define('xoutMask',   xoutMask)
define('vPC',        vPC)
define('vAC',        vAC)
define('vACH',       vAC+1)
define('vLR',        vLR)
define('vSP',        vSP)
define('vTmp',       vTmp)      # Not in interface.json
define('romType',    romType)
define('sysFn',      sysFn)
for i in range(8):
  define('sysArgs%d' % i, sysArgs+i)
define('soundTimer', soundTimer)
define('ledTimer',   ledTimer)
define('ledState_v2',ledState_v2)
define('ledTempo',   ledTempo)
define('userVars',   userVars)
define('videoTable', videoTable)
define('vIRQ_v5',    vIRQ_v5)
define('videoTop_v5',videoTop_v5)
define('userCode',   userCode)
define('soundTable', soundTable)
define('screenMemory',screenMemory)
define('vReset',     vReset)
define('wavA',       wavA)
define('wavX',       wavX)
define('keyL',       keyL)
define('keyH',       keyH)
define('oscL',       oscL)
define('oscH',       oscH)
define('maxTicks',   maxTicks)
define('v6502_PC',   v6502_PC)
define('v6502_PCL',  v6502_PCL)
define('v6502_PCH',  v6502_PCH)
define('v6502_A',    v6502_A)
define('v6502_X',    v6502_X)
define('v6502_Y',    v6502_Y)
define('qqVgaWidth', qqVgaWidth)
define('qqVgaHeight',qqVgaHeight)
define('buttonRight',buttonRight)
define('buttonLeft', buttonLeft)
define('buttonDown', buttonDown)
define('buttonUp',   buttonUp)
define('buttonStart',buttonStart)
define('buttonSelect',buttonSelect)
define('buttonB',    buttonB)
define('buttonA',    buttonA)

# XXX This is a hack (trampoline() is probably in the wrong module):
define('vPC+1',      vPC+1)

#-----------------------------------------------------------------------
#       Embedded programs -- import and convert programs and data
#-----------------------------------------------------------------------

def basicLine(address, number, text):
  """Helper to encode lines for TinyBASIC"""
  head = [] if number is None else [number&255, number>>8]
  body = [] if text is None else [ord(c) for c in text] + [0]
  s = head + body
  assert len(s) > 0
  for i, byte in enumerate([address>>8, address&255, len(s)]+s):
    comment = repr(chr(byte)) if i >= 3+len(head) else None
    program.putInRomTable(byte, comment=comment)

#-----------------------------------------------------------------------

lastRomFile = ''

def insertRomDir(name):
  global lastRomFile
  if name[0] != '_':                    # Mechanism for hiding files
    if pc()&255 >= 251-14:              # Prevent page crossing
      trampoline()
    s = lastRomFile[0:8].ljust(8,'\0')  # Cropping and padding
    if len(lastRomFile) == 0:
      lastRomFile = 0
    for i in range(8):
      st(ord(s[i]), [Y,Xpp])            #25-32
      C(repr(s[i]))
    ld(lo(lastRomFile))                 #33
    st([vAC])                           #34
    ld(hi(lastRomFile))                 #35
    ld(hi('.sysDir#39'),Y)              #36
    jmp(Y,'.sysDir#39')                 #37
    st([vAC+1])                         #38
    lastRomFile = name

#-----------------------------------------------------------------------
#       Embedded programs must be given on the command line
#-----------------------------------------------------------------------

if pc()&255 >= 251:                     # Don't start in a trampoline region
  align(0x100)

for application in argv[1:]:
  print()

  # Determine label
  if '=' in application:
    # Explicit label given as 'label=filename'
    name, application = application.split('=', 1)
  else:
    # Label derived from filename itself
    name = application.rsplit('.', 1)[0] # Remove extension
    name = name.rsplit('/', 1)[-1]       # Remove path
  print('Processing file %s label %s' % (application, name))

  C('+-----------------------------------+')
  C('| %-33s |' % application)
  C('+-----------------------------------+')

  # Pre-compiled GT1 files
  if application.endswith(('.gt1', '.gt1x')):
    print('Load type .gt1 at $%04x' % pc())
    with open(application, 'rb') as f:
      raw = bytearray(f.read())
    insertRomDir(name)
    label(name)
    raw = raw[:-2] # Drop start address
    if raw[0] == 0 and raw[1] + raw[2] > 0xc0:
      highlight('Warning: zero-page conflict with ROM loader (SYS_Exec_88)')
    program = gcl.Program(None)
    for byte in raw:
      program.putInRomTable(byte)
    program.end()

  # GCL files
  #----------------------------------------------------------------
  #  !!! GCL programs using *labels* "_L=xx" must be cautious !!!
  # Those labels end up in the same symbol table as the ROM build,
  # and name clashes cause havoc. It's safer to precompile such
  # applications into .gt1/.gt1x files. (This warning doesn't apply
  # to ordinary GCL variable names "xx A=".)
  #----------------------------------------------------------------
  elif application.endswith('.gcl'):
    print('Compile type .gcl at $%04x' % pc())
    insertRomDir(name)
    label(name)
    program = gcl.Program(name)
    program.org(userCode)
    zpReset(userVars)
    for line in open(application).readlines():
      program.line(line)
    program.end()

  # Application-specific SYS extensions
  elif application.endswith('.py'):
    print('Include type .py at $%04x' % pc())
    label(name)
    importlib.import_module(name)

  # GTB files
  elif application.endswith('.gtb'):
    print('Link type .gtb at $%04x' % pc())
    zpReset(userVars)
    label(name)
    program = gcl.Program(name)
    # BasicProgram comes from TinyBASIC.gcl
    address = symbol('BasicProgram')
    if not has(address):
      highlight('Error: TinyBASIC must be compiled-in first')
    program.org(address)
    i = 0
    for line in open(application):
      i += 1
      line = line.rstrip()[0:25]
      number, text = '', ''
      for c in line:
        if c.isdigit() and len(text) == 0:
          number += c
        else:
          text += c
      basicLine(address, int(number), text)
      address += 32
      if address & 255 == 0:
        address += 160
    basicLine(address+2, None, 'RUN')           # Startup command
    # Buffer comes from TinyBASIC.gcl
    basicLine(symbol('Buffer'), address, None)  # End of program
    program.putInRomTable(0)
    program.end()
    print(' Lines', i)

  # Simple sequential RGB file (for Racer horizon image)
  elif application.endswith('-256x16.rgb'):
    width, height = 256, 16
    print('Convert type .rgb/sequential at $%04x' % pc())
    f = open(application, 'rb')
    raw = bytearray(f.read())
    f.close()
    insertRomDir(name)
    label(name)
    packed, quartet = [], []
    for i in range(0, len(raw), 3):
      R, G, B = raw[i+0], raw[i+1], raw[i+2]
      quartet.append((R//85) + 4*(G//85) + 16*(B//85))
      if len(quartet) == 4:
        # Pack 4 pixels in 3 bytes
        packed.append( ((quartet[0]&0b111111)>>0) + ((quartet[1]&0b000011)<<6) )
        packed.append( ((quartet[1]&0b111100)>>2) + ((quartet[2]&0b001111)<<4) )
        packed.append( ((quartet[2]&0b110000)>>4) + ((quartet[3]&0b111111)<<2) )
        quartet = []
    for i in range(len(packed)):
      ld(packed[i])
      if pc()&255 == 251:
        trampoline()
    print(' Pixels %dx%d' % (width, height))

  # Random access RGB files (for Pictures application)
  elif application.endswith('-160x120.rgb'):
    if pc()&255 > 0:
      trampoline()
    print('Convert type .rgb/parallel at $%04x' % pc())
    f = open(application, 'rb')
    raw = f.read()
    f.close()
    label(name)
    for y in range(0, qqVgaHeight, 2):
      for j in range(2):
        comment = 'Pixels for %s line %s' % (name, y+j)
        for x in range(0, qqVgaWidth, 4):
          bytes = []
          for i in range(4):
            R = raw[3 * ((y + j) * qqVgaWidth + x + i) + 0]
            G = raw[3 * ((y + j) * qqVgaWidth + x + i) + 1]
            B = raw[3 * ((y + j) * qqVgaWidth + x + i) + 2]
            bytes.append( (R//85) + 4*(G//85) + 16*(B//85) )
          # Pack 4 pixels in 3 bytes
          ld( ((bytes[0]&0b111111)>>0) + ((bytes[1]&0b000011)<<6) ); comment = C(comment)
          ld( ((bytes[1]&0b111100)>>2) + ((bytes[2]&0b001111)<<4) )
          ld( ((bytes[2]&0b110000)>>4) + ((bytes[3]&0b111111)<<2) )
        if j==0:
          trampoline3a()
        else:
          trampoline3b()
    print(' Pixels %dx%d' % (width, height))

  # XXX Provisionally bring ROMv1 egg back as placeholder for Pictures
  elif application.endswith(('/gigatron.rgb', '/packedPictures.rgb')):
    print(('Convert type gigatron.rgb at $%04x' % pc()))
    f = open(application, 'rb')
    raw = bytearray(f.read())
    f.close()
    label(name)
    for i in range(len(raw)):
      if i&255 < 251:
        ld(raw[i])
      elif pc()&255 == 251:
        trampoline()

  else:
    assert False

  C('End of %s, size %d' % (application, pc() - symbol(name)))
  print(' Size %s' % (pc() - symbol(name)))

#-----------------------------------------------------------------------
# ROM directory
#-----------------------------------------------------------------------

# SYS_ReadRomDir implementation

if pc()&255 > 251 - 28:         # Prevent page crossing
  trampoline()
label('sys_ReadRomDir')
beq('.sysDir#20')               #18
ld(lo(sysArgs),X)               #19
ld(AC,Y)                        #20 Follow chain to next entry
ld([vAC])                       #21
suba(14)                        #22
jmp(Y,AC)                       #23
#ld(hi(sysArgs),Y)              #24 Overlap
#
label('.sysDir#20')
ld(hi(sysArgs),Y)               #20,24 Dummy
ld(lo('.sysDir#25'))            #21 Go to first entry in chain
ld(hi('.sysDir#25'),Y)          #22
jmp(Y,AC)                       #23
ld(hi(sysArgs),Y)               #24
label('.sysDir#25')
insertRomDir(lastRomFile)       #25-38 Start of chain
label('.sysDir#39')
ld(hi('REENTER'),Y)             #39 Return
jmp(Y,'REENTER')                #40
ld(-44/2)                       #41

print()

#-----------------------------------------------------------------------
# End of embedded applications
#-----------------------------------------------------------------------

if pc()&255 > 0:
  trampoline()

#-----------------------------------------------------------------------
# Finish assembly
#-----------------------------------------------------------------------
end()
writeRomFiles(argv[0])
