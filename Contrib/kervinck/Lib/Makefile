CC:=build/lcc -I.

# For Marcel's provisional LCC setup
export LCCDIR=build

all: DemoLib.gt1

DemoLib.gt1: sys/ClearScreen.o sys/Newline.o sys/Random.o stdio/putchar.o stdio/puts.o DemoLib.o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) -c $< -o $@

clean:
	rm -f */*.o *.o

# vi: noexpandtab
