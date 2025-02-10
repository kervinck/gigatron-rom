CC=../../build/glcc
ROM=dev7
MAP=32k
CFLAGS=-map=${MAP} -rom=${ROM}

PGMS=TSTconsole.gt1 TSTio.gt1 TSTcontroller.gt1 TSTmemcpyext.gt1 TSTmem.gt1 TSTmaze.gt1

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
	${CC} -map=32k -rom=${ROM} TSTmemcpyext.c -o $@

TSTmem.gt1: TSTmem.c
	${CC} -map=32k -rom=${ROM} TSTmem.c -o $@

TSTmaze.gt1: TSTmaze.c
	${CC} -map=32k -rom=${ROM} TSTmaze.c -o $@ --no-runtime-bss
