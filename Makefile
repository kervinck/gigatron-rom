CFLAGS:=-std=c11 -O3 -Wall

findColors: findColors.c
	$(CC) $(CFLAGS) findColors.c -o findColors -lm
