CFLAGS:=-std=c11 -O3 -Wall

theloop.2.rom: *.py *.gcl Images/*.rgb
	./theloop.py

run: gtemu theloop.2.rom
	./gtemu

test: gtemu theloop.2.rom
	# Check for hSync errors in first ~30 seconds of emulation
	./gtemu | head -999999 | grep \~

burn: theloop.2.rom
	minipro -p 'AT27C1024 @DIP40' -w theloop.2.rom -y -s

%.rgb: %.png
	# Uses ImageMagick
	convert "$<" "$@"

todo:
	@git ls-files | sed 's/ /\\ /g' | xargs grep -I -i -E '(todo|xxx)'
