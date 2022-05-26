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
GTSIM_CFLAGS=-g -O3
GLCC=${B}glcc
E=

MAPDIR=mapsim
PYFILES=${wildcard *.py}
PYTARGETS=${patsubst %.py,${B}${MAPDIR}/%.py,${PYFILES}}

LIBNAME=libsim
SFILES=$(wildcard ${LIBNAME}/*.s)
CFILES=$(wildcard ${LIBNAME}/*.c)
O4FILES=${patsubst %.c,${B}%_4.o,${CFILES}}
O5FILES=${patsubst %.c,${B}%_5.o,${CFILES}}
O6FILES=${patsubst %.c,${B}%_6.o,${CFILES}}
OFILES=${O4FILES} ${O5FILES} ${O6FILES}

all: ${B}gtsim${E} ${PYTARGETS} ${B}${MAPDIR}/${LIBNAME}.a

${B}gtsim${E}: gtsim.c
	${CC} ${GTSIM_CFLAGS} -o $@ gtsim.c -lm

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

clean: FORCE
	-rm "${B}gtsim"
	-rm -rf ${PYTARGETS} ${B}${MAPDIR}/${LIBNAME}.a
	-rm ${OFILES}
	-rmdir ${B}${LIBNAME}
	-rmdir ${B}${MAPDIR}

install: FORCE
	-${INSTALL} -d "${libdir}/${MAPDIR}"
	${INSTALL} -m 0644 "${B}${MAPDIR}/${LIBNAME}.a" "${libdir}/${MAPDIR}/"
	${INSTALL} -m 0644 ${PYTARGETS} "${libdir}/${MAPDIR}/"
	${INSTALL} -m 0755 "${B}gtsim${E}" "${libdir}/gtsim${E}"
ifndef MSYSTEM
	-${INSTALL} -d "${bindir}"
	${LN_S} ${libdir}/gtsim${E} ${bindir}/gtsim${E}
endif

test: FORCE

FORCE: .PHONY

.PHONY: 

