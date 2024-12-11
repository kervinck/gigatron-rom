
CC=../../build/glcc
ROM=dev7
MAP=32k
CFLAGS=-rom=$(ROM) -map=$(MAP)

PGMS=threads_test.gt1

all: $(PGMS)

threads_test.gt1: threads_test.c threads.s threads.h
	${CC} ${CFLAGS} threads_test.c threads.s -o $@

clean:
	-rm ${PGMS}


