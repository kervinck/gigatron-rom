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

MAPDIR=map128k
PYFILES=${wildcard *.py}
PYTARGETS=${patsubst %.py,${B}${MAPDIR}/%.py,${PYFILES}}
SUBDIRS= libcon1

all: ${PYTARGETS} subdirs-all

${B}${MAPDIR}/%.py: %.py
	-mkdir -p ${B}${MAPDIR}
	cp $+ $@

clean: FORCE subdirs-clean
	-rm -rf ${PYTARGETS}
	-rmdir ${B}${MAPDIR}

install: FORCE subdirs-install
	-${INSTALL} -d "${libdir}/${MAPDIR}"
	${INSTALL} -m 0644 ${PYTARGETS} "${libdir}/${MAPDIR}/"

test: FORCE

subdirs-%: FORCE
	@for n in ${SUBDIRS} ; do \
	   ${MAKE} -C $$n \
		"PREFIX=${PREFIX}" \
		"BUILDDIR=${BUILDDIR}" \
		"DESTDIR=${DESTDIR}" \
		"CFLAGS=${CFLAGS}" \
		"LDFLAGS=${LDFLAGS}" \
		"MAPDIR=${MAPDIR}" \
		"E=${E}" \
		"ROM=${ROM}" \
		`echo $@ | sed -e 's/^subdirs-//'` || exit; \
	   done

FORCE: .PHONY

.PHONY: 

