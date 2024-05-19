#-----------------------------------------------------------------------
#
#	Makefile for Gigatron ROM and tooling
#
#-----------------------------------------------------------------------

# Latest development version as default target
DEV:=games.rom

# Emulators open gigatron.rom as their ROM image
gigatron.rom: $(DEV)
 ifdef OS # Windows
	copy $< $@
 else
	ln -sf "$<" "$@"
 endif

dev: $(DEV)

TOPDIR=../..

# Allow application-specific SYS extensions to live in Apps/
export PYTHONPATH:=$(TOPDIR)/Apps/Loader:$(TOPDIR)/Apps/Racer:$(TOPDIR)/Core
export PYTHONDONTWRITEBYTECODE:=please

ifdef OS # Windows
 export PYTHONPATH:=$(subst :,;,$(PYTHONPATH))
endif

CFLAGS:=-std=c11 -O3 -Wall

# Stripped down ROMs for FPGA version of Gigatron ROM v6

games.rom: $(TOPDIR)/Core/* $(TOPDIR)/Apps/*/* Makefile interface.json
	python3 Core/fpga_games.asm.py\
		Snake=$(TOPDIR)/Apps/Snake/Snake_v3.gcl\
		zippedRacerHorizon=$(TOPDIR)/Apps/Racer/Horizon-256x16.rgb\
		SYS_Racer_v1.py\
		Racer=$(TOPDIR)/Apps/Racer/Racer_v3.gcl\
		Tetronis=$(TOPDIR)/Apps/Tetronis/Tetronis_v2.gt1\
		GtMine=$(TOPDIR)/Apps/GtMine/gtmine_v1.gt1\
		Bricks=$(TOPDIR)/Apps/Bricks/Bricks_v2.gt1\
		TinyBASIC=$(TOPDIR)/Apps/TinyBASIC/TinyBASIC_v6.gcl\
		TicTac=$(TOPDIR)/Apps/TicTac/LoadTicTac_v1.gcl\
		TicTacGtb=$(TOPDIR)/Apps/TicTac/TicTac_v2.gtb\
		$(TOPDIR)/Apps/Loader/SYS_Loader_v5.py\
		Loader=$(TOPDIR)/Apps/Loader/Loader_v4.gcl\
		Main=Apps/MainMenu/MainMenu_games.gcl\
		Reset=$(TOPDIR)/Core/Reset_v6.gcl

games.run: 
	DISPLAY=0:0 $(TOPDIR)/../sim/emulator/gtemu ./fpga_games.rom


#-----------------------------------------------------------------------
#	Development
#-----------------------------------------------------------------------

dev.rom: $(TOPDIR)/Core/* $(TOPDIR)/Apps/*/* Makefile interface.json
	python3 Core/fpga_dev.asm.py\
		dummy=$(TOPDIR)/Apps/Horizon/Horizon_c_v2.gt1\
		gigatronRaw=$(TOPDIR)/Apps/Screen/gigatron.rgb\
		Screen=$(TOPDIR)/Apps/Screen/Screen_v1.gcl\
		$(TOPDIR)/Apps/Loader/SYS_Loader_v5.py\
		Loader=$(TOPDIR)/Apps/Loader/Loader_v4.gcl\
		Main=Apps/MainMenu/MainMenu_dev.gcl\
		Reset=$(TOPDIR)/Core/Reset_v6.gcl
# Main=Apps/Credits/Credits.gt1\

dev.run: 
	DISPLAY=0:0 ../at67/gtemuAT67


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
#	Official Gigatron ROM version
#-----------------------------------------------------------------------

# ROMv6 freezes lots of change since ROMv5a,
# including improved support for RAM&IO Expansion boards, a new
# application GtMine, a faster Mandelbrot, and lots of smaller
# improvements and bug fixes.
ROMv6.rom: Core/* Apps/*/* Makefile interface.json
	python3 Core/ROMv6.asm.py\
		packedPictures=Apps/Pictures/packedPictures.rgb\
		Snake=Apps/Snake/Snake_v3.gcl\
		SYS_Racer_v1.py\
		zippedRacerHorizon=Apps/Racer/Horizon-256x16.rgb\
		Racer=Apps/Racer/Racer_v3.gcl\
		Mandelbrot=Apps/Mandelbrot/Mandelbrot_v2.gt1\
		SYS_Loader_v5.py\
		Pictures=Apps/Pictures/Pictures_v3.gcl\
		Bricks=Apps/Bricks/Bricks_v2.gt1\
		GtMine=Apps/GtMine/gtmine_v1.gt1\
		TinyBASIC=Apps/TinyBASIC/TinyBASIC_v6.gcl\
		TicTac=Apps/TicTac/LoadTicTac_v1.gcl\
		TicTacGtb=Apps/TicTac/TicTac_v2.gtb\
		WozMon=Apps/WozMon/WozMon_v2.gcl\
		Apple1=Apps/Apple-1/Apple-1_v3.gt1\
		MSBASIC=Apps/MSBASIC/MSBASIC_v1.gt1\
		Loader=Apps/Loader/Loader_v4.gcl\
		Credits=Apps/Credits/Credits_v4.gt1\
		Egg=Apps/Horizon/Horizon_c_v2.gt1\
		Boot=Apps/CardBoot/CardBoot_v2.gt1\
		Main=Apps/MainMenu/MainMenu_v6.gcl\
		Reset=Core/Reset_v6.gcl

burnv6: ROMv6.rom
	minipro -p 'AT27C1024 @DIP40' -w "$<" -y -s


#-----------------------------------------------------------------------
#	Generic rules
#-----------------------------------------------------------------------

%.gt1: %.gcl
	Core/compilegcl.py "$<" `dirname "./$@"`

%.gt1x: %.gcl
	Core/compilegcl.py -x "$<" `dirname "./$@"`

%.gt1: %.vasm.py
	env PYTHONPATH=Core python3 "$<"
	mv out.gt1 "$@"

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
	python3 "$<" > "$@"

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

%.gt1x: %.o $(libObjects)
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
