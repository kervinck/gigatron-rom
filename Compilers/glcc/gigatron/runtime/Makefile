# GNUmake required

SHELL=/bin/sh
TOP=../../
PREFIX=/usr/local
DESTDIR=
BUILDDIR=build
B=${TOP}${BUILDDIR}/
G=${TOP}gigatron/
GLCC=${B}glcc
GTSIM=${B}gtsim

all: FORCE

clean: FORCE

install: FORCE

TSTCFILES=$(wildcard tst/TST*.c)
TSTXFILES=${patsubst tst/%.c,${B}tst/%.gt1,${TSTCFILES}}
TSTOFILES=${patsubst tst/%.c,${B}tst/%.xx1,${TSTCFILES}}

test: ${TSTOFILES}

${B}tst/%.xx1: ${B}tst/%.gt1
	-${GTSIM} -rom ${G}roms/${ROM}.rom $< > $@
	cmp $@ tst/$(*F).out

${B}tst/%.gt1: tst/%.c FORCE
	@test -d ${B}tst || mkdir ${B}tst
	${GLCC} -map=sim -rom=${ROM} -DROM=${ROM} -o $@ $<

.PRECIOUS: ${B}tst/%.gt1

FORCE: .PHONY

.PHONY: 
