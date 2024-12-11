
ROM=dev7
GLCC=glcc -map=64k -rom=${ROM}
GLCCFLAGS=--no-runtime-bss-initialization

CC=cc
CFLAGS=


all: rotator_${ROM}_64k.gt1

rotator_${ROM}_64k.gt1: rotator.c table.s Makefile
	${GLCC} ${GLCCFLAGS} -o $@ rotator.c table.s


table.s: tablegen
	./tablegen > table.s

tablegen: tablegen.c Makefile
	${CC} ${CFLAGS} -o $@ tablegen.c

clean: FORCE
	-rm tablegen table.s *.gt1

tar: FORCE
	tar cvfz rotator.tar.gz Makefile tablegen.c rotator.c

FORCE:

.PHONY: FORCE
