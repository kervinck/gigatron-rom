
CC=../../build/glcc
ROM=v5a
MAP=64k,hionly
DEFS=
CFLAGS=-rom=$(ROM) -map=$(MAP) -Dprintf=mincprintf ${DEFS}

PGMS=sieve0.gt1 sieve1.gt1 sieve2.gt1

all: $(PGMS)

sieve0.gt1: sieve0.c
	${CC} ${CFLAGS} sieve0.c -o $@

sieve1.gt1: sieve1.c
	${CC} ${CFLAGS} sieve1.c -o $@

sieve2.gt1: sieve2.c
	${CC} ${CFLAGS} sieve2.c -o $@

clean:
	-rm ${PGMS}

