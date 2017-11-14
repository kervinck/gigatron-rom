CFLAGS:=-std=c11 -O3 -Wall

gtemu:

findColors: findColors.c
	$(CC) $(CFLAGS) findColors.c -o findColors -lm
