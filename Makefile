#-----------------------------------------------------------------------
#
#	Makefile for Gigatron ROM and tooling
#
#-----------------------------------------------------------------------

# Latest development version as default target
gigatron.rom: dev.rom
	ln -sf "$<" "$@"

# Allow application-specific SYS extensions to live in Apps/
export PYTHONPATH=Apps

CFLAGS:=-std=c11 -O3 -Wall

#-----------------------------------------------------------------------
#	Development
#-----------------------------------------------------------------------

# Development towards "ROM v5"
dev: dev.rom
dev.rom: Core/* Apps/* Images/* Makefile interface.json
	python -B Core/dev.py\
		packedParrot=Images/Parrot-160x120.rgb\
		packedJupiter=Images/Jupiter-160x120.rgb\
		SYS_Racer_v1.py\
		SYS_Loader_v2.py\
		Snake=Apps/Snake.gcl\
		zippedRacerHorizon=Images/RacerHorizon-256x16.rgb\
		Racer=Apps/Racer.gcl\
		Mandelbrot=Apps/Mandelbrot_v1.gcl\
		Pictures=Apps/Pictures_v2.gcl\
		Credits=Apps/Credits_v3.gcl\
		Loader=Apps/Loader_v2.gcl\
		Tetronis=Apps/Tetronis_v1.gt1\
		Bricks=Apps/Bricks_v1.gt1\
		TinyBASIC=Apps/TinyBASIC.gcl\
		TicTac=Apps/TicTac_v2.gtb\
		WozMon=Apps/WozMon_v2.gt1\
		Egg=Apps/Apple1.gt1\
		Main=Apps/Main.gcl\
		Reset=Core/Reset.gcl

# ROM v4b has many small changes, but no new applications
ROMv4b.rom: Core/* Apps/* Images/* Makefile interface.json
	python -B Core/ROMv4b.py\
		packedParrot=Images/Parrot-160x120.rgb\
		packedJupiter=Images/Jupiter-160x120.rgb\
		SYS_Racer_v1.py\
		SYS_Loader_v2.py\
		Snake=Apps/Snake.gcl\
		zippedRacerHorizon=Images/RacerHorizon-256x16.rgb\
		Racer=Apps/Racer.gcl\
		Mandelbrot=Apps/Mandelbrot_v1.gcl\
		Pictures=Apps/Pictures_v2.gcl\
		Credits=Apps/Credits_v3.gcl\
		Loader=Apps/Loader_v2.gcl\
		Tetronis=Apps/Tetronis_v1.gt1\
		Bricks=Apps/Bricks_v1.gt1\
		TinyBASIC=Apps/TinyBASIC.gcl\
		TicTac=Apps/TicTac_v2.gtb\
		WozMon=Apps/WozMon_v2.gt1\
		Egg=Apps/Apple1.gt1\
		Main=Apps/Main.gcl\
		Reset=Core/Reset_v4.gcl

burnv4b: ROMv4b.rom
	minipro -p 'AT27C1024 @DIP40' -w "$<" -y -s

run: Docs/gtemu dev.rom
	# Run ROM in reference emulator on console
	# Pipe though less(1) to hop from frame to frame with 'n' (next)
	# !!! Set terminal width to >225 chars !!!
	Docs/gtemu dev.rom | less -p 'line 0'

export jsEmu=Contrib/PhilThomas/src
runjs: dev.rom
	# Run ROM in javascript emulator in web browser (macOS)
	cd "$(jsEmu)" && ln -sf ../../../dev.rom gigatron.rom
	(sleep 1 && open http://127.0.0.1:8000/src) &
	cd "$(jsEmu)" && npm start

test: Docs/gtemu dev.rom
	# Check for hSync errors in first ~30 seconds of emulation
	Docs/gtemu dev.rom | head -999999 | grep \~

compiletest: Apps/*.gcl
	# Test compilation
	# (Use 'git diff' afterwards to detect unwanted changes)
	for GCL in Apps/*.gcl; do Core/compilegcl.py "$${GCL}" Apps; done

time: Docs/gtemu dev.rom
	# Run emulation until first sound, typically for benchmarking
	Docs/gtemu dev.rom | grep -m 1 'xout [^0]'

burn: dev.rom
	# Program 27C1024 EEPROM with ROM image
	minipro -p 'AT27C1024 @DIP40' -w "$<" -y -s

burn85:
	# Program Pluggy McPlugface
	# Set to 8 MHz
	minipro -p attiny85 -w Utils/BabelFish/BabelFish.ATtiny85_fuses.txt -c config
	# ROM image
	minipro -p attiny85 -w Utils/BabelFish/BabelFish.ATtiny85.bin -s

#-----------------------------------------------------------------------
#	Released ROM versions
#-----------------------------------------------------------------------

# ROM v3 integrates BASIC, WozMon, Tetronis, Bricks, TicTacToe
# vPulse modulation (for SAVE in BASIC), sprite acceleration
# Note: ROM builder still directly incudes TicTac_v1.gtb
ROMv3.rom: Core/* Apps/* Images/* Makefile interface.json
	python -B Core/ROMv3.py\
		Apps/Snake_v2.gcl\
		Apps/Racer_v1.gcl\
		Apps/Mandelbrot_v1.gcl\
		Apps/Pictures_v2.gcl\
		Apps/Credits_v2.gcl\
		Apps/Loader_v2.gcl\
		Apps/Tetronis_v1.gt1\
		Apps/Bricks_v1.gt1\
		Apps/TinyBASIC_v2.gcl\
		Apps/WozMon_v2.gt1\
		Egg=Apps/Sprites_v1.gt1\
		Apps/Main_v3.gcl\
		Core/Reset_v3.gcl

burnv3: ROMv3.rom
	minipro -p 'AT27C1024 @DIP40' -w "$<" -y -s

# ROM v2 minor changes only
ROMv2.rom: Core/* Apps/* Images/* Makefile interface.json
	python -B Core/ROMv2.py\
		Apps/Snake_v2.gcl\
		Apps/Racer_v1.gcl\
		Apps/Mandelbrot_v1.gcl\
		Apps/Pictures_v1.gcl\
		Apps/Credits_v1.gcl\
		Apps/Loader_v1.gcl\
		Apps/TinyBASIC_v1.gcl\
		Apps/WozMon_v1.gcl\
		Apps/Main_v2.gcl\
		Core/Reset_v2.gcl

# ROM v1 as shipped with first batches of kits
ROMv1.rom: Core/* Apps/* Images/* Makefile interface.json
	python -B Core/ROMv1.py\
		Apps/Snake_v1.gcl\
		Apps/Racer_v1.gcl\
		Apps/Mandelbrot_v1.gcl\
		Apps/Pictures_v1.gcl\
		Apps/Credits_v1.gcl\
		Apps/Loader_v1.gcl\
		Apps/Screen_v1.gcl\
		Apps/Main_v1.gcl\
		Core/Reset_v1.gcl

#-----------------------------------------------------------------------
#	Experimental and temporary (to be removed)
#-----------------------------------------------------------------------

# Test ROM for v6502 testing
mos: v6502.rom
v6502.rom: Core/* Apps/* Images/* Makefile interface.json
	rm -f dev.rom dev.asm
	python -B Core/dev.py\
		Main=Apps/Apple1.gcl\
		Core/Reset.gcl
	mv dev.rom v6502.rom
	mv dev.asm v6502.asm

burnmos: v6502.rom
	minipro -p 'AT27C1024 @DIP40' -w "$<" -y -s

# Adds vCPU slices on scanlines to get 400 cycles per scanline
ROMv3y.rom: Core/* Apps/* Images/* Makefile interface.json
	python -B Core/ROMv3y.py\
		Apps/Snake_v2.gcl\
		Apps/Racer_v1.gcl\
		Apps/Mandelbrot_v1.gcl\
		Apps/Pictures_v2.gcl\
		Apps/Credits_v2.gcl\
		Apps/Loader_v2.gcl\
		Apps/Tetronis_v1.gt1\
		Apps/Bricks_v1.gt1\
		Apps/TinyBASIC_v2.gcl\
		Apps/WozMon_v2.gt1\
		Egg=Apps/Sprites_v1.gt1\
		Apps/Main_v3.gcl\
		Core/Reset_v3y.gcl

#-----------------------------------------------------------------------
#	Generic rules
#-----------------------------------------------------------------------

%.gt1: %.gcl
	Core/compilegcl.py "$<" `dirname "./$@"`

%.gt1x: %.gt1 %.gcl
	# Non-compliant files in .gt1 format (see Docs/GT1-files.txt)
	mv "$<" "$@"

%.h: %.gt1
	# Convert GT1 file into header for including as PROGMEM data
	od -t x1 -v < "$<" |\
	awk 'BEGIN {print "// Converted from $< by Makefile"}\
	     {for (i=2; i<=NF; i++) printf "0x%s,\n", $$i}' > "$@"

%.rgb: %.png
	# Uses ImageMagick
	convert "$<" "$@"

Utils/BabelFish/tinyfont.h: Utils/BabelFish/tinyfont.py
	python "$<" > "$@"

todo:
	@git ls-files | sed 's/ /\\ /g' | xargs grep -I -E '(TODO|XXX)'

# Show simplified git log
log:
	git log --oneline --decorate --graph --all

#-----------------------------------------------------------------------
#	C compiler (LCC retargeted for vCPU)
#-----------------------------------------------------------------------

LCCDIR:=Utils/lcc/build
export LCCDIR
LCC:=$(LCCDIR)/lcc
LCCFLAGS:=-ILibs
#LCCFLAGS:=-ILibs -Wf-d -Wa-d

lcc:
	mkdir -p "$(LCCDIR)"
	mkdir -p "$(LCCDIR)/tst"
	cd Utils/lcc && env HOSTFILE=etc/gt1h.c make all gttest

%.o: %.c $(wildcard Libs/*.h)
	$(LCC) $(LCCFLAGS) -c "$<" -o "$@"

libSources:=$(wildcard Libs/*/*.c)
libObjects:=$(libSources:.c=.o)

.SECONDARY: # Instructs 'make' not to delete intermeditate .o files
%.gt1: %.o $(libObjects)
	$(LCC) $(LCCFLAGS) "$^" -o "$@"

ctest: Libs/Example.gt1

cclean:
	rm -f Libs/Example.gt1 Libs/*.o Libs/*/*.o

# Moon shot for C compiler: MSCP 1.4 (Marcel's Simple Chess Program)
# Doesn't work yet. Use as guinea pig to help mature our standard C library
mscp: Contrib/kervinck/mscp.gt1
Contrib/kervinck/mscp.o: Contrib/kervinck/mscp.c $(wildcard Libs/*.h)
	$(LCC) $(LCCFLAGS) -N -P -A -v -c "$<" -o "$@"

#-----------------------------------------------------------------------
#
#-----------------------------------------------------------------------
# vi: noexpandtab
