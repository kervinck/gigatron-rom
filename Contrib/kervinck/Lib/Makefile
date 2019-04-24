CC:=build/lcc -I.

all: DemoLib.gt1

DemoLib.gt1: gt/ClearScreen.o gt/Newline.o stdio/putc.o stdio/puts.o DemoLib.o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) -c $< -o $@

clean:
	rm -f */*.o *.o

# vi: noexpandtab
