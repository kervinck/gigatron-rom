SHELL=/bin/sh
TOP=../../../
PREFIX=/usr/local
DESTDIR=
BUILDDIR=build
bindir=${DESTDIR}${PREFIX}/bin
libdir=${DESTDIR}${PREFIX}/lib/gigatron-lcc
INSTALL=${TOP}/gigatron/install-sh
LN_S=ln -s
B=${TOP}${BUILDDIR}/
G=${TOP}gigatron/
GTSIM_CFLAGS=-g -O3
GLCC=${B}glcc
E=

LIBNAME=libcon_h
SFILES=$(wildcard *.s)
CFILES=$(wildcard *.c)
O5FILES=${patsubst %.c,${B}${LIBNAME}/%_5.o,${CFILES}}
O6FILES=${patsubst %.c,${B}${LIBNAME}/%_6.o,${CFILES}}
OFILES=${O5FILES} ${O6FILES}

all: ${B}${MAPDIR}/${LIBNAME}.a ${B}${LIBNAME}/test.gt1

${B}${MAPDIR}/${LIBNAME}.a: ${SFILES} ${O4FILES} ${O5FILES} ${O6FILES}
	-@mkdir -p ${B}${MAPDIR}
	cat $+ > $@

${B}${LIBNAME}/%_5.o: %.c ${B}rcc${E}
	-@mkdir -p ${B}${LIBNAME}
	${GLCC} -c -cpu=5 -o $@ $<

${B}${LIBNAME}/%_6.o: %.c ${B}rcc${E}
	-@mkdir -p ${B}${LIBNAME}
	${GLCC} -c -cpu=6 -o $@  $<


${B}${LIBNAME}/test.gt1: test.c ${B}${MAPDIR}/${LIBNAME}.a
	${GLCC} -rom=v5a -o $@ test.c ${B}${MAPDIR}/${LIBNAME}.a

clean: FORCE
	-rm 2>/dev/null ${B}${MAPDIR}/${LIBNAME}.a ${OFILES} ${B}${LIBNAME}/test.gt1
	-rmdir 2>/dev/null ${B}${LIBNAME}

install: all FORCE
	-${INSTALL} -d "${libdir}/${MAPDIR}"
	${INSTALL} -m 0644 "${B}${MAPDIR}/${LIBNAME}.a" "${libdir}/${MAPDIR}/"



.PHONY: FORCE
