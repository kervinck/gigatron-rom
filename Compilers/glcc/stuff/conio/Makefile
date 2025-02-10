ROM=v5a
MAP=32k,nochan
GLCC=../../build/glcc
CFLAGS=-rom=$(ROM) -map=$(MAP) 

PROGS=TSTkbgeta.gt1 TSTkbgetb.gt1 TSTkbgetc.gt1 TSTsnake.gt1

all: $(PROGS)

TSTkbgeta.gt1: TSTkbget.c
	${GLCC} ${CFLAGS} --option=KBGET_SIMPLE TSTkbget.c -o $@

TSTkbgetb.gt1: TSTkbget.c
	${GLCC} ${CFLAGS} --option=KBGET_AUTOBTN TSTkbget.c -o $@

TSTkbgetc.gt1: TSTkbget.c
	${GLCC} ${CFLAGS} --option=KBGET_AUTOREPEAT TSTkbget.c -o $@

TSTsnake.gt1: TSTsnake.c
	${GLCC} ${CFLAGS} TSTsnake.c -o $@

snake: TSTsnake.c unix/conio.h unix/conio.c
	gcc -o $@ -Iunix TSTsnake.c unix/conio.c

clean: FORCE
	-rm -rf $(PROGS) snake

FORCE:
