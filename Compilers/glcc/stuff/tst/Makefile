CC=../../build/glcc
ROM=v5a
MAP=32k
CFLAGS=-map=${MAP} -rom=${ROM}

PGMS=TSTconsole.gt1 TSTio.gt1 TSTcontroller.gt1 TSTmemcpyext.gt1 TSTmem.gt1

all: $(PGMS)

clean:
	-rm $(PGMS)

TSTconsole.gt1: TSTconsole.c
	${CC} ${CFLAGS} TSTconsole.c -o $@

TSTio.gt1: TSTio.c
	${CC} ${CFLAGS} TSTio.c -o $@

TSTcontroller.gt1: TSTcontroller.c
	${CC} ${CFLAGS} TSTcontroller.c -o $@

TSTmemcpyext.gt1: TSTmemcpyext.c
	${CC} -map=32k -rom=dev TSTmemcpyext.c -o $@

TSTmem.gt1: TSTmem.c
	${CC} -map=32k -rom=${ROM} TSTmem.c -o $@
