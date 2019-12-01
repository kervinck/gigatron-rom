#-----------------------------------------------------------------------
#
#	Makefile for Gigatron ROM and tooling
#
#-----------------------------------------------------------------------

# Latest development version as default target
DEV:=dev.rom

# Emulators open gigatron.rom as their ROM image
gigatron.rom: $(DEV)
 ifdef OS # Windows
	copy $< $@
 else
	ln -sf "$<" "$@"
 endif

dev: $(DEV)

# Allow application-specific SYS extensions to live in Apps/
export PYTHONPATH:=Apps/Loader:Apps/Racer
export PYTHONDONTWRITEBYTECODE:=please

ifdef OS # Windows
 export PYTHONPATH:=$(subst :,;,$(PYTHONPATH))
endif

CFLAGS:=-std=c11 -O3 -Wall

#-----------------------------------------------------------------------
#	Development
#-----------------------------------------------------------------------

# Development towards "ROM v5"
dev.rom: Core/* Apps/*/* Makefile interface.json
	python Core/dev.py\
		gigatronRaw=Apps/Screen/gigatron.rgb\
		Snake=Apps/Snake/Snake_v3.gcl\
		SYS_Racer_v1.py\
		SYS_Loader.py\
		zippedRacerHorizon=Apps/Racer/Horizon-256x16.rgb\
		Racer=Apps/Racer/Racer_v2.gcl\
		Mandelbrot=Apps/Mandelbrot/Mandelbrot_v1.gcl\
		Pictures=Apps/Screen/Screen_v1.gcl\
		Credits=Apps/Credits/Credits_v3.gcl\
		Loader=Apps/Loader/Loader_v3.gcl\
		Tetronis=Apps/Tetronis/Tetronis.gt1\
		Bricks=Apps/Bricks/Bricks.gt1\
		TinyBASIC=Apps/TinyBASIC/TinyBASIC_v3.gcl\
		TicTac=Apps/TicTac/TicTac.gcl\
		TicTacGtb=Apps/TicTac/TicTac_v2.gtb\
		WozMon=Apps/WozMon/WozMon_v2.gcl\
		Apple1=Apps/Apple1/Apple1.gt1\
		MSBASIC=Apps/MSBASIC/MSBASIC.gt1\
		Egg=Apps/Horizon/Horizon.gt1\
		Main=Apps/MainMenu/MainMenu.gcl\
		Reset=Core/Reset.gcl

run: Docs/gtemu $(DEV)
	# Run ROM in reference emulator on console
	# Pipe though less(1) to hop from frame to frame with 'n' (next)
	# !!! Set terminal width to >225 chars !!!
	Docs/gtemu $(DEV) | less -p 'line 0'

export jsEmu=Contrib/PhilThomas/src
runjs: $(DEV)
	# Run ROM in javascript emulator in web browser (macOS)
	cd "$(jsEmu)" && ln -sf ../../../$(DEV) gigatron.rom
	(sleep 1 && open http://127.0.0.1:8000/src) &
	cd "$(jsEmu)" && npm start

test: Docs/gtemu $(DEV)
	# Check for hSync errors in first ~30 seconds of emulation
	Docs/gtemu $(DEV) | head -999999 | grep \~

compiletest: Apps/*/*.gcl
	# Test compilation
	# (Use 'git diff' afterwards to detect unwanted changes)
	for GCL in Apps/*/*.gcl; do Core/compilegcl.py "$${GCL}" `dirname "./$${GCL}"` ; done
	@echo "Use 'git diff' to inspect result (no .gt1 file should have changed)"

time: Docs/gtemu $(DEV)
	# Run emulation until first sound, typically for benchmarking
	Docs/gtemu $(DEV) | grep -m 1 'xout [^0]'

burn: $(DEV)
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

# ROM v4 support `TypeC' game controller signals. There are
# many small changes under the hood, but no new applications.
ROMv4.rom: Core/* Apps/*/* Makefile interface.json
	python Core/ROMv4.py\
		packedParrot=Apps/Pictures/Parrot-160x120.rgb\
		packedJupiter=Apps/Pictures/Jupiter-160x120.rgb\
		Apps/Racer/SYS_Racer_v1.py\
		Apps/Loader/SYS_Loader_v3.py\
		Snake=Apps/Snake/Snake_v3.gcl\
		zippedRacerHorizon=Apps/Racer/Horizon-256x16.rgb\
		Racer=Apps/Racer/Racer_v2.gcl\
		Mandelbrot=Apps/Mandelbrot/Mandelbrot_v1.gcl\
		Pictures=Apps/Pictures/Pictures_v2.gcl\
		Credits=Apps/Credits/Credits_v3.gcl\
		Loader=Apps/Loader/Loader_v3.gcl\
		Tetronis=Apps/Tetronis/Tetronis_v1.gt1\
		Bricks=Apps/Bricks/Bricks_v1.gt1\
		TinyBASIC=Apps/TinyBASIC/TinyBASIC_v3.gcl\
		TicTac=Apps/TicTac/TicTac_v2.gtb\
		WozMon=Apps/WozMon/WozMon_v2.gt1\
		Egg=Apps/Apple1/Apple1_v1.gt1\
		Main=Apps/MainMenu/MainMenu_v4.gcl\
		Reset=Core/Reset_v4.gcl

burnv4: ROMv4.rom
	minipro -p 'AT27C1024 @DIP40' -w "$<" -y -s

# ROM v3 integrates BASIC, WozMon, Tetronis, Bricks, TicTacToe
# vPulse modulation (for SAVE in BASIC), sprite acceleration
# Note: ROM builder still directly incudes TicTac_v1.gtb
ROMv3.rom: Core/* Apps/*/* Makefile interface.json
	python Core/ROMv3.py\
		Apps/Snake/Snake_v2.gcl\
		Apps/Racer/Racer_v1.gcl\
		Apps/Mandelbrot/Mandelbrot_v1.gcl\
		Apps/Pictures/Pictures_v2.gcl\
		Apps/Credits/Credits_v2.gcl\
		Apps/Loader/Loader_v2.gcl\
		Apps/Tetronis/Tetronis_v1.gt1\
		Apps/Bricks/Bricks_v1.gt1\
		Apps/TinyBASIC/TinyBASIC_v2.gcl\
		Apps/WozMon/WozMon_v2.gt1\
		Egg=Apps/Sprites/Sprites_v1.gt1\
		Main=Apps/MainMenu/MainMenu_v3.gcl\
		Core/Reset_v3.gcl

burnv3: ROMv3.rom
	minipro -p 'AT27C1024 @DIP40' -w "$<" -y -s

# ROM v2 minor changes only
ROMv2.rom: Core/* Apps/*/* Makefile interface.json
	python Core/ROMv2.py\
		Apps/Snake/Snake_v2.gcl\
		Apps/Racer/Racer_v1.gcl\
		Apps/Mandelbrot/Mandelbrot_v1.gcl\
		Apps/Pictures/Pictures_v1.gcl\
		Apps/Credits/Credits_v1.gcl\
		Apps/Loader/Loader_v1.gcl\
		Apps/TinyBASIC/TinyBASIC_v1.gcl\
		Apps/WozMon/WozMon_v1.gcl\
		Apps/MainMenu/Main_v2.gcl\
		Core/Reset_v2.gcl

# ROM v1 as shipped with first batches of kits
ROMv1.rom: Core/* Apps/*/* Makefile interface.json
	python Core/ROMv1.py\
		Apps/Snake/Snake_v1.gcl\
		Apps/Racer/Racer_v1.gcl\
		Apps/Mandelbrot/Mandelbrot_v1.gcl\
		Apps/Pictures/Pictures_v1.gcl\
		Apps/Credits/Credits_v1.gcl\
		Apps/Loader/Loader_v1.gcl\
		Apps/Screen/Screen_v1.gcl\
		Apps/MainMenu/Main_v1.gcl\
		Core/Reset_v1.gcl

#-----------------------------------------------------------------------
#	Experimental and temporary (to be removed)
#-----------------------------------------------------------------------

# Adds vCPU slices on scanlines to get 400 cycles per scanline
ROMv3y.rom: Core/* Apps/*/* Makefile interface.json
	python Core/ROMv3y.py\
		Apps/Snake/Snake_v2.gcl\
		Apps/Racer/Racer_v1.gcl\
		Apps/Mandelbrot/Mandelbrot_v1.gcl\
		Apps/Pictures/Pictures_v2.gcl\
		Apps/Credits/Credits_v2.gcl\
		Apps/Loader/Loader_v2.gcl\
		Apps/Tetronis/Tetronis_v1.gt1\
		Apps/Bricks/Bricks_v1.gt1\
		Apps/TinyBASIC/TinyBASIC_v2.gcl\
		Apps/WozMon/WozMon_v2.gt1\
		Egg=Apps/Sprites/Sprites_v1.gt1\
		Main=Apps/MainMenu/MainMenu_v3.gcl\
		Core/Reset_v3y.gcl

#-----------------------------------------------------------------------
#	Generic rules
#-----------------------------------------------------------------------

%.gt1: %.gcl
	Core/compilegcl.py "$<" `dirname "./$@"`

%.gt1x: %.gcl
	Core/compilegcl.py -x "$<" `dirname "./$@"`

%.h: %.gt1
	# Convert GT1 file into header for including as PROGMEM data
	od -t x1 -v < "$<" |\
	awk 'BEGIN {print "// Converted from $< by Makefile"}\
	     {for (i=2; i<=NF; i++) printf "0x%s,\n", $$i}' > "$@"

%.rgb: %.png
	# Uses ImageMagick
	convert "$<" "$@"

# 64tass assembler
%.obj: %.asm
	64tass -b "$<" -o "$@" -L "$*.lst"
	od -An -t x1 -v < "$@" |\
	awk '{for(i=1;i<=NF;i++)print" #$$" $$i}' |\
	fmt -w 80 > "$*.hex"

# as65 assembler (from cc65)
%.obj: %.s
	ca65 "$<" -o "$*.o.tmp" -l "$*.lst"
	ld65 -t none -o "$*.obj" "$*.o.tmp"
	rm -f "$*.o.tmp"

# Hex dump from object file
%.hex: %.obj
	od -An -t x1 -v < "$<" |\
	awk '{for(i=1;i<=NF;i++)print" #$$" $$i}' |\
	fmt -w 80 > "$@"

Utils/BabelFish/tinyfont.h: Utils/BabelFish/tinyfont.py
	python "$<" > "$@"

todo:
	@git ls-files | sed 's/ /\\ /g' | xargs grep -I -E '(TODO|XXX)'

# Show simplified git log
log:
	git log --oneline --decorate --graph --all

# Get updates from GitHub while preserving work in progress
pull:
	git stash push
	git pull https://github.com/kervinck/gigatron-rom
	git stash pop

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
	$(LCC) $(LCCFLAGS) $^ -o "$@"

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
