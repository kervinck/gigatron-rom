CFLAGS:=-std=c11 -Ofast -Wall -ggdb3
LDFLAGS=-lSDL2 -ggdb3

romType=0x1c # ROMv1 gets 0x1c. Further numbers to be decided.

theloop.2.rom: Core/* Apps/* Images/* Makefile
	env romType="$(romType)"\
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
