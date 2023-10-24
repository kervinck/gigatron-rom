# GNUmake required

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
BL=${TOP}${BUILDDIR}/libc/
INC=${TOP}include/gigatron/
G=${TOP}gigatron/
GLCC=${B}glcc
GTSIM=${B}gtsim
TFLAGS=
E=

SFILES=$(wildcard *.s)
CFILES=$(wildcard *.c)
O4FILES=${patsubst %.c,${BL}%_4.o,${CFILES}}
O5FILES=${patsubst %.c,${BL}%_5.o,${CFILES}}
O6FILES=${patsubst %.c,${BL}%_6.o,${CFILES}}
O7FILES=${patsubst %.c,${BL}%_7.o,${CFILES}}
OFILES=${O4FILES} ${O5FILES} ${O6FILES} ${O7FILES}
RFILES=$(wildcard ../runtime/*.s)

all: ${B}cpu4/libc.a ${B}cpu5/libc.a ${B}cpu6/libc.a ${B}cpu7/libc.a

${B}cpu4/libc.a: ${SFILES} ${O4FILES} ${RFILES}
	-@mkdir -p ${B}cpu4
	cat ${SFILES} ${O4FILES} ${RFILES} > ${B}/cpu4/libc.a

${B}cpu5/libc.a: ${SFILES} ${O5FILES} ${RFILES}
	-@mkdir -p ${B}cpu5
	cat ${SFILES} ${O5FILES} ${RFILES} > ${B}/cpu5/libc.a

${B}cpu6/libc.a: ${SFILES} ${O6FILES} ${RFILES}
	-@mkdir -p ${B}cpu6
	cat ${SFILES} ${O6FILES} ${RFILES} > ${B}/cpu6/libc.a

${B}cpu7/libc.a: ${SFILES} ${O7FILES} ${RFILES}
	-@mkdir -p ${B}cpu7
	cat ${SFILES} ${O7FILES} ${RFILES} > ${B}/cpu7/libc.a

# this is incomplete but better than nothing
DEPS=_stdio.h ${INC}stdio.h ${INC}gigatron/libc.h ${B}rcc${E}

${BL}%_4.o: %.c ${DEPS}
	-@mkdir -p ${BL}
	${GLCC} -c -cpu=4 -o $@ $<

${BL}%_5.o: %.c ${DEPS}
	-@mkdir -p ${BL}
	${GLCC} -c -cpu=5 -o $@ $<

${BL}%_6.o: %.c ${DEPS}
	-@mkdir -p ${BL}
	${GLCC} -c -cpu=6 -o $@  $<

${BL}%_7.o: %.c ${DEPS}
	-@mkdir -p ${BL}
	${GLCC} -c -cpu=7 -o $@  $<

clean: FORCE
	-rm ${OFILES}
	-for cpu in 4 5 6; do \
	   rm ${B}cpu$$cpu/libc.a ; \
	   rmdir ${B}cpu$$cpu ; \
	 done
	-rmdir ${BL}

install: FORCE
	for cpu in 4 5 6 7; do if test -d ${B}cpu$$cpu; then \
	  ${INSTALL} -d "${libdir}/cpu$$cpu/" ; \
	  ${INSTALL} -m 644 "${B}cpu$$cpu/libc.a" "${libdir}/cpu$$cpu/libc.a" ; \
	fi; done


TSTCFILES=$(wildcard tst/TST*.c)
TSTXFILES=${patsubst tst/%.c,${B}tst/%.gt1,${TSTCFILES}}
TSTOFILES=${patsubst tst/%.c,${B}tst/%.xx1,${TSTCFILES}}

test: ${TSTOFILES}

${B}tst/%.xx1: ${B}tst/%.gt1
	-${GTSIM} -rom ${G}roms/${ROM}.rom $< > $@
	cmp $@ tst/$(*F).out

${B}tst/%.gt1: tst/%.c FORCE
	@test -d ${B}tst || mkdir ${B}tst
	${GLCC} -map=sim -rom=${ROM} ${TFLAGS} -o $@ $<

.PRECIOUS: ${B}tst/%.gt1

FORCE: .PHONY

.PHONY: 

