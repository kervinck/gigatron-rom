CC:=build/lcc -I.

# For Marcel's provisional LCC setup
export LCCDIR=build

all: Example.gt1

Example.gt1: sys/ClearScreen.o sys/Newline.o sys/Random.o stdio/putchar.o stdio/puts.o Example.o
	$(CC) $^ -o $@

%.o: %.c
	$(CC) -c $< -o $@

clean:
	rm -f */*.o *.o

# vi: noexpandtab
