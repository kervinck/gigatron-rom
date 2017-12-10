CFLAGS:=-std=c11 -O3 -Wall

run: gtemu theloop.2.rom
	./gtemu

test: gtemu theloop.2.rom
	# Check for hSync errors in first ~30 seconds of emulation
	./gtemu | head -999999 | grep \~

theloop.2.rom: theloop.py
	./theloop.py

burn: theloop.2.rom
	minipro -p 'AT27C1024 @DIP40' -w theloop.2.rom -y -s
