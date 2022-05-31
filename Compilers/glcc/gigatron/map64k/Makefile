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

MAPDIR=map64k
PYFILES=${wildcard *.py}
PYTARGETS=${patsubst %.py,${B}${MAPDIR}/%.py,${PYFILES}}

all: ${PYTARGETS}

${B}${MAPDIR}/%.py: %.py
	-mkdir -p ${B}${MAPDIR}
	cp $+ $@

clean: FORCE
	-rm -rf ${PYTARGETS} ${B}${MAPDIR}/${LIBNAME}.a
	-rmdir ${B}${MAPDIR}

install: FORCE
	-${INSTALL} -d "${libdir}/${MAPDIR}"
	${INSTALL} -m 0644 ${PYTARGETS} "${libdir}/${MAPDIR}/"

test: FORCE

FORCE: .PHONY

.PHONY: 

