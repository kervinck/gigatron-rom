CFLAGS:=-std=c11 -O3 -Wall
DEV:=ROMv1x

# Latest released version as default target
ROMv1.rom: Core/* Apps/* Images/* Makefile interface.json
	# ROMv1 gets 0x1c. Further numbers to be decided.
	env romType="0x1c"\
	    PYTHONPATH="Core:$(PYTHONPATH)"\
	    python Core/ROMv1.py\
		Apps/Snake.gcl\
		Apps/Racer.gcl\
		Apps/Mandelbrot.gcl\
		Apps/Pictures.gcl\
		Apps/Credits.gcl\
		Apps/Loader.gcl\
		Apps/Screen.gcl\
		Apps/Main.gcl\
		Core/Reset.gcl

# Work in progress
dev: $(DEV).rom
$(DEV).rom: Core/* Apps/* Images/* Makefile interface-0x20.json
	# ROMv1x is development towards ROMv2 (minor changes only)
	env romType="0x20"\
	    PYTHONPATH="Core:$(PYTHONPATH)"\
	    python Core/ROMv1x.py\
		Apps/Snake_v1x.gcl\
		Apps/Racer.gcl\
		Apps/Mandelbrot.gcl\
		Apps/Pictures.gcl\
		Apps/Credits.gcl\
		Apps/Loader.gcl\
		Apps/TinyBASIC.gcl\
		Apps/WozMon.gcl\
		Apps/Main_v1x.gcl\
		Core/Reset_v1x.gcl

run: Docs/gtemu $(DEV).rom
	Docs/gtemu $(DEV).rom

test: Docs/gtemu $(DEV).rom
	# Check for hSync errors in first ~30 seconds of emulation
	Docs/gtemu $(DEV).rom | head -999999 | grep \~

compiletest:
	# Test compilation
	Core/compilegcl.py Apps/HelloWorld.gcl
	Core/compilegcl.py Apps/Snake.gcl
	Core/compilegcl.py Apps/Mandelbrot.gcl
	Core/compilegcl.py Apps/Credits.gcl

time: Docs/gtemu $(DEV).rom
	# Run emulation until first sound
	Docs/gtemu $(DEV).rom | grep -m 1 'xout [^0]'

burn: $(DEV).rom
	minipro -p 'AT27C1024 @DIP40' -w "$<" -y -s

%.h: %.gt1
	# Convert GT1 file into header for including as PROGMEM data
	od -t x1 < "$<" |\
	awk 'BEGIN {print "// Converted from $< by Makefile"}\
	     {for (i=2; i<=NF; i++) printf "0x%s,\n", $$i}' > "$@"

%.rgb: %.png
	# Uses ImageMagick
	convert "$<" "$@"

todo:
	@git ls-files | sed 's/ /\\ /g' | xargs grep -I -E '(TODO|XXX)'

# Show simplified git log
log:
	git log --oneline --decorate --graph --all

# vi: noexpandtab
