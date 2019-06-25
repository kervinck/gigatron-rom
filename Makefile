CFLAGS:=-std=c11 -O3 -Wall
DEV:=ROMv3x

# Latest released version as default target:
# Integrates BASIC, WozMon, Tetronis, Bricks, TicTacToe
# vPulse modulation (for SAVE in BASIC), sprite acceleration
# Note: ROM builder still directly incudes TicTac_v1.gtb
ROMv3.rom: Core/* Apps/* Images/* Makefile interface.json
	python Core/ROMv3.py\
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
		Apps/Sprites_v1.gt1\
		Apps/Main_v3.gcl\
		Core/Reset_v3.gcl

# Development towards ROM v4
dev: $(DEV).rom
$(DEV).rom: Core/* Apps/* Images/* Makefile interface.json
	python Core/$(DEV).py\
		packedParrot=Images/Parrot-160x120.rgb\
		packedJupiter=Images/Jupiter-160x120.rgb\
		Apps/Snake_v2.gcl\
		zippedRacerHorizon=Images/RacerHorizon-256x16.rgb\
		Apps/Racer_v1.gcl\
		Apps/Mandelbrot_v1.gcl\
		Apps/Pictures_v2.gcl\
		Apps/Credits_v2.gcl\
		Apps/Loader_v2.gcl\
		Apps/Tetronis_v1.gt1\
		Apps/Bricks_v1.gt1\
		Apps/TinyBASIC_v2.gcl\
		Apps/TicTac_v1.gtb\
		Apps/WozMon_v2.gt1\
		Apps/Sprites_v1.gt1\
		Apps/Main_v3.gcl\
		Core/Reset_v4.gcl

# Test ROM for v6502 testing
mos: v6502.rom
v6502.rom: Core/* Apps/* Images/* Makefile interface.json
	rm -f ROMv3x.rom ROMv3x.asm
	python Core/ROMv3x.py\
		Main=Apps/Apple1.gcl\
		Core/Reset_v4.gcl
	mv ROMv3x.rom v6502.rom
	mv ROMv3x.asm v6502.asm
	open http://127.0.0.1:8000/src

# Experimental revision, based on ROM v3, for overclocked systems at 12.5 MHz.
# Adds vCPU slices on scanlines to get 400 cycles per scanline
ROMv3y.rom: Core/* Apps/* Images/* Makefile interface.json
	python Core/ROMv3y.py\
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
		Apps/Sprites_v1.gt1\
		Apps/Main_v3.gcl\
		Core/Reset_v3y.gcl

# ROM v2 minor changes only
ROMv2.rom: Core/* Apps/* Images/* Makefile interface.json
	python Core/ROMv2.py\
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
	python Core/ROMv1.py\
		Apps/Snake_v1.gcl\
		Apps/Racer_v1.gcl\
		Apps/Mandelbrot_v1.gcl\
		Apps/Pictures_v1.gcl\
		Apps/Credits_v1.gcl\
		Apps/Loader_v1.gcl\
		Apps/Screen_v1.gcl\
		Apps/Main_v1.gcl\
		Core/Reset_v1.gcl

run: Docs/gtemu $(DEV).rom
	Docs/gtemu $(DEV).rom

test: Docs/gtemu $(DEV).rom
	# Check for hSync errors in first ~30 seconds of emulation
	Docs/gtemu $(DEV).rom | head -999999 | grep \~

Utils/BabelFish/tinyfont.h: Utils/BabelFish/tinyfont.py
	python "$<" > "$@"

compiletest: Apps/*.gcl
	# Test compilation
	# (Use 'git diff' afterwards to detect unwanted changes)
	for GCL in Apps/*.gcl; do Core/compilegcl.py "$${GCL}" Apps; done

time: Docs/gtemu $(DEV).rom
	# Run emulation until first sound
	Docs/gtemu $(DEV).rom | grep -m 1 'xout [^0]'

burnv3: ROMv3.rom
	minipro -p 'AT27C1024 @DIP40' -w "$<" -y -s

burn: $(DEV).rom
	minipro -p 'AT27C1024 @DIP40' -w "$<" -y -s

burnmos: v6502.rom
	minipro -p 'AT27C1024 @DIP40' -w "$<" -y -s

burn85:
	# Set to 8 MHz
	minipro -p attiny85 -w Utils/BabelFish/BabelFish.ATtiny85_fuses.txt -c config
	# ROM image
	minipro -p attiny85 -w Utils/BabelFish/BabelFish.ATtiny85.bin -s

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

todo:
	@git ls-files | sed 's/ /\\ /g' | xargs grep -I -E '(TODO|XXX)'

# Show simplified git log
log:
	git log --oneline --decorate --graph --all

# vi: noexpandtab
