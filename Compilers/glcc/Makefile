SHELL=/bin/sh
TOP=
PREFIX=/usr/local
DESTDIR=
BUILDDIR=build
bindir=${DESTDIR}${PREFIX}/bin
libdir=${DESTDIR}${PREFIX}/lib/gigatron-lcc
INSTALL=${TOP}gigatron/install-sh
LN_S=ln -sf
B=${TOP}${BUILDDIR}/
G=${TOP}gigatron/
TARGET=gigatron
CFLAGS=-g -Wno-abi
LDFLAGS=-g
EXTRALIBS=-lm
PYTHON=python3
HOSTFILE=${TOP}etc/gigatron-lcc.c
GLCC=${B}glcc
GTSIM=${B}gtsim
TFLAGS=

SUBDIRS=${G}runtime ${G}libc \
        ${G}map32k ${G}map64k ${G}mapsim ${G}mapconx \
        ${G}map128k ${G}map512k
GFILES=${B}glcc ${B}glink ${B}gt1dump \
       ${B}glink.py ${B}glccver.py ${B}interface.json \
       ${B}interface-dev.json ${B}roms.json ${GFILES_W}
ROMFILES=${wildcard ${G}roms/*.rom}
ROMS=${patsubst ${G}roms/%.rom,%,${ROMFILES}}
RUNROM=

ifdef COMSPEC
E=.exe
GFILES_W=${B}glink.cmd ${B}glcc.cmd
else
E=
GFILES_W=
endif

default: all

all: build-dir lcc-all gigatron-all
	${MAKE} subdirs-all
	@echo "+----------------------------------+"
	@echo "|   Compilation ran successfully!  |"
	@echo "+----------------------------------+"


clean: lcc-clean gigatron-clean subdirs-clean build-dir-clean

install: all gigatron-install subdirs-install
ifdef MSYSTEM
	echo "Calling Mingw post-installation script"
	PREFIX="${PREFIX}" INSTALL="${INSTALL}" \
	  bindir="${bindir}" libdir="${libdir}" "${G}mingw-install-sh"
endif

test: all
	@test -d ${B}tst || mkdir ${B}tst
	@for rom in ${ROMS}; do \
	    printf "+----------------------------------+\n"; \
	    printf "|  Compiling for rom: %-8s     |\n" $$rom; \
	    printf "+----------------------------------+\n"; \
	    ${MAKE} ROM=$$rom RUNROM=${RUNROM} glcc-test subdirs-test || exit; \
	    wc -c ${B}tst/*.gt1 > ${B}tst/sizes-$$rom.txt; \
	 done
	@echo "+----------------------------------+"
	@echo "|  Test sequence ran successfully! |"
	@echo "+----------------------------------+"

build-dir: FORCE
	-mkdir -p ${BUILDDIR}

build-dir-clean: FORCE
	-rm -rf ${B}

lcc-%: FORCE
	@${MAKE} -f makefile.lcc \
		"PREFIX=${PREFIX}" \
		"BUILDDIR=${BUILDDIR}" \
		"HOSTFILE=${HOSTFILE}" \
                "TARGET=${TARGET}" \
		"CFLAGS=${CFLAGS}" \
		"LDFLAGS=${LDFLAGS}" \
		"EXTRALIBS=${EXTRALIBS}" \
		"E=${E}" \
		`echo $@ | sed -e 's/^lcc-//'`

subdirs-%: FORCE
	@for n in ${SUBDIRS} ; do \
	   ${MAKE} -C $$n \
		"PREFIX=${PREFIX}" \
		"BUILDDIR=${BUILDDIR}" \
		"DESTDIR=${DESTDIR}" \
		"CFLAGS=${CFLAGS}" \
		"LDFLAGS=${LDFLAGS}" \
		"E=${E}" \
		"ROM=${ROM}" \
		"RUNROM=${RUNROM}" \
		`echo $@ | sed -e 's/^subdirs-//'` || exit; \
	   done

gigatron-all: gigatron-include ${GFILES} 

gigatron-clean: FORCE
	-rm -rf ${GFILES} ${B}include
	-rm -rf ${B}tst[0-9]

gigatron-install: FORCE
	${INSTALL} -d "${libdir}"
	${INSTALL} -m 755 "${B}cpp${E}" "${libdir}/cpp${E}"
	${INSTALL} -m 755 "${B}rcc${E}" "${libdir}/rcc${E}"
	${INSTALL} -m 755 "${B}lcc${E}" "${libdir}/lcc${E}"
	for n in ${GFILES}; do \
	    mode=644; test -x "$$n" && mode=755 ; \
	    ${INSTALL} -m $$mode "$$n" "${libdir}/" ; done
	test -r ${B}glink.pyc && \
	    ${INSTALL} -m 644 ${B}glink.pyc "${libdir}/"
	-${INSTALL} -d "${libdir}/include"
	for n in "${B}include/"*.h ; do \
	    ${INSTALL} -m 0644 "$$n" "${libdir}/include/" ; done
	-${INSTALL} -d "${libdir}/include/gigatron"
	for n in "${B}include/gigatron/"*.h ; do \
	    ${INSTALL} -m 0644 "$$n" "${libdir}/include/gigatron/" ; done
ifndef MSYSTEM
	-${INSTALL} -d "${bindir}"
	${LN_S} "${libdir}/glcc" "${bindir}/glcc"
	${LN_S} "${libdir}/glink" "${bindir}/glink"
	${LN_S} "${libdir}/gt1dump" "${bindir}/gt1dump"
endif

gigatron-include: FORCE
	-mkdir -p ${B}include
	cp -r ${TOP}include/gigatron/* ${B}/include/

${B}glink: ${G}glink
	cp ${G}glink ${B}glink
	chmod a+x ${B}glink

${B}glink.py: ${G}glink.py
	cp ${G}glink.py ${B}glink.py
	${PYTHON} -m compileall -b ${B}glink.py

${B}glcc: ${G}glcc
	cp ${G}glcc ${B}glcc
	chmod a+x ${B}glcc

${B}glcc.cmd ${B}glink.cmd: FORCE
	echo '@py -3 "%~dp0\%~n0" %*' > $@

${B}glccver.py: FORCE
	id=`( test -d .git && which git >/dev/null && \
              git describe --tags 2>/dev/null || echo no )` ; \
	if test "$${id}" != no ; then \
	    echo 'ver="'"$${id}"'"' > "$@" ; \
	else \
	    cp "${G}glccver.py" "$@" ; \
	fi

${B}%: ${G}%
	cp $< $@


GTSIMR=${GTSIM} -rom ${or ${RUNROM},${G}roms/${ROM}.rom}
TSTBK1FILES=$(wildcard ${G}tst/*.1bk)
TSTBK2FILES=$(wildcard ${G}tst/*.2bk)
TSTX=${patsubst ${G}tst/%.1bk,${B}tst/%.gt1, ${TSTBK1FILES}}
TSTO=${patsubst ${G}tst/%.1bk,${B}tst/%.xx1, ${TSTBK1FILES}}

ifeq (${ROM},dev7)
TSTS=${patsubst ${G}tst/%.2bk,${B}tst/%.s, ${TSTBK2FILES}}
endif

glcc-test: ${TSTS} ${TSTO}

${B}tst/%.s: tst/%.c FORCE
	@test -d ${B}tst || mkdir ${B}tst
	-${GLCC} -S -rom=${ROM} -o $@  $< 2>"${B}tst/$(*F).ss2"
	cmp "${B}tst/$(*F).ss2" "${G}tst/$(*F).2bk"
	[ ! -r "${G}tst/$(*F).sbk" ] || cmp $@ "${G}tst/$(*F).sbk"

${B}tst/%.s: ${G}tst/%.c FORCE
	@test -d ${B}tst || mkdir ${B}tst
	-${GLCC} -S -rom=${ROM} -o $@  $< 2>"${B}tst/$(*F).ss2"
	cmp "${B}tst/$(*F).ss2" "${G}tst/$(*F).2bk"
	[ ! -r "${G}tst/$(*F).sbk" ] || cmp $@ "${G}tst/$(*F).sbk"

${B}tst/%.gt1: tst/%.c FORCE
	@test -d ${B}tst || mkdir ${B}tst
	${GLCC} -map=sim,allout -rom=${ROM} ${TFLAGS} -o $@ $< 2>"${B}tst/$(*F).xx2"

${B}tst/%.xx1: ${B}tst/%.gt1 FORCE
	${GTSIMR} $< > "$@" < "tst/$(*F).0"
	cmp $@ ${G}tst/$(*F).1bk

FORCE:

.PRECIOUS: ${B}tst/%.gt1

.PHONY: FORCE



