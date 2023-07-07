SHELL=/bin/sh
TOP=../../
PREFIX=/usr/local
DESTDIR=
BUILDDIR=build
bindir=${DESTDIR}${PREFIX}/bin
libdir=${DESTDIR}${PREFIX}/lib/gigatron-lcc
INSTALL=${TOP}/gigatron/install-sh
LN_S=ln -s
B=${TOP}${BUILDDIR}/
G=${TOP}gigatron/
GLCC=${B}glcc
E=

MAPDIR=mapconx
PYFILES=${wildcard *.py}
PYTARGETS=${patsubst %.py,${B}${MAPDIR}/%.py,${PYFILES}}

LIBNAME=libconx
SFILES=$(wildcard ${LIBNAME}/*.s)
CFILES=$(wildcard ${LIBNAME}/*.c)
O4FILES=${patsubst %.c,${B}%_4.o,${CFILES}}
O5FILES=${patsubst %.c,${B}%_5.o,${CFILES}}
O6FILES=${patsubst %.c,${B}%_6.o,${CFILES}}
O7FILES=${patsubst %.c,${B}%_7.o,${CFILES}}
OFILES=${O4FILES} ${O5FILES} ${O7FILES}

all: ${PYTARGETS} ${B}${MAPDIR}/${LIBNAME}.a

${B}${MAPDIR}/%.py: %.py
	-mkdir -p ${B}${MAPDIR}
	cp $+ $@

${B}${MAPDIR}/${LIBNAME}.a: ${SFILES} ${O4FILES} ${O5FILES} ${O6FILES}
	-mkdir -p ${B}${MAPDIR}
	cat $+ > $@

${B}${LIBNAME}/%_4.o: ${LIBNAME}/%.c ${B}rcc${E}
	-@mkdir -p ${B}${LIBNAME}
	${GLCC} -c -cpu=4 -o $@ $<

${B}${LIBNAME}/%_5.o: ${LIBNAME}/%.c ${B}rcc${E}
	-@mkdir -p ${B}${LIBNAME}
	${GLCC} -c -cpu=5 -o $@ $<

${B}${LIBNAME}/%_6.o: ${LIBNAME}/%.c ${B}rcc${E}
	-@mkdir -p ${B}${LIBNAME}
	${GLCC} -c -cpu=6 -o $@  $<

${B}${LIBNAME}/%_7.o: ${LIBNAME}/%.c ${B}rcc${E}
	-@mkdir -p ${B}${LIBNAME}
	${GLCC} -c -cpu=7 -o $@  $<

clean: FORCE
	-rm -rf ${PYTARGETS} ${B}${MAPDIR}/${LIBNAME}.a
	-rm ${OFILES}
	-rmdir ${B}${LIBNAME}
	-rmdir ${B}${MAPDIR}

install: FORCE
	-${INSTALL} -d "${libdir}/${MAPDIR}"
	${INSTALL} -m 0644 "${B}${MAPDIR}/${LIBNAME}.a" "${libdir}/${MAPDIR}/"
	${INSTALL} -m 0644 ${PYTARGETS} "${libdir}/${MAPDIR}/"

test: FORCE

FORCE: .PHONY

.PHONY: 

