# compile c programs with GLCC
CC=../../build/glcc

# use 32k map modified by overlay horizon.ovl
CFLAGS=-map=32k,./horizon.ovl

# avoid runtime bss initialization to save code.
LDFLAGS=--no-runtime-bss-init


PGMS=horizon-v4.gt1 horizon-v5a.gt1 horizon-dev7.gt1

all: $(PGMS) 

clean:
	-rm $(PGMS)

horizon-v4.gt1: horizon.c horizon.ovl
	${CC} ${CFLAGS} ${LDFLAGS} -rom=v4 horizon.c -o $@

horizon-v5a.gt1: horizon.c horizon.ovl
	${CC} ${CFLAGS} ${LDFLAGS} -rom=v5a horizon.c -o $@

horizon-dev7.gt1: horizon.c horizon.ovl
	${CC} ${CFLAGS} ${LDFLAGS} -rom=dev7 horizon.c -o $@
