CFLAGS:=-std=c11 -O3 -Wall

theloop.2.rom: Core/* Apps/* Images/* Makefile
	# ROMv1 gets 0x1c. Further numbers to be decided.
	env romType="0x1c"\
	    PYTHONPATH="Core:$(PYTHONPATH)"\
	    python Core/theloop.py\
		Apps/Snake.gcl\
		Apps/Racer.gcl\
		Apps/Mandelbrot.gcl\
		Apps/Pictures.gcl\
		Apps/Credits.gcl\
		Apps/Loader.gcl\
		Apps/Screen.gcl\
		Apps/Main.gcl\
		Core/Reset.gcl

run: gtemu theloop.2.rom
	./gtemu

test: gtemu theloop.2.rom
	# Check for hSync errors in first ~30 seconds of emulation
	./gtemu | head -999999 | grep \~

compiletest:
	# Test compilation
	Core/compilegcl.py Apps/HelloWorld.gcl
	Core/compilegcl.py Apps/Snake.gcl
	Core/compilegcl.py Apps/Mandelbrot.gcl
	Core/compilegcl.py Apps/Credits.gcl

time: gtemu theloop.2.rom
	# Run emulation until first sound
	./gtemu | grep -m 1 'xout [^0]'

burn: theloop.2.rom
	minipro -p 'AT27C1024 @DIP40' -w theloop.2.rom -y -s

%.rgb: %.png
	# Uses ImageMagick
	convert "$<" "$@"

todo:
	@git ls-files | sed 's/ /\\ /g' | xargs grep -I -E '(TODO|XXX)'

# vi: noexpandtab
