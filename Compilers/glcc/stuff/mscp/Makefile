ROM=dev7
MAP=128k
GLCC=../../build/glcc
PYTHON=python3
PGMS=mscp.gt1
OBJS=mscp.o core.o
ASMS=onload.s

all: ${PGMS}

mscp.gt1: ${OBJS} ${ASMS} mscp.ovl
	${GLCC} -rom=${ROM} -map=${MAP},./mscp.ovl ${OBJS} ${ASMS} -o $@
	${PYTHON} addbook.py $@ book.bin

mscp.o: mscp.c core.h
	${GLCC} -rom=${ROM} -c $<

core.o: core.c core.h
	${GLCC} -rom=${ROM} -c $<

dumpbook: mscp.c
	${CC} -DSAVE_BOOK_BIN=1 -DSUBTRACTIVE_RND=1 mscp.c -o dumpbook

book.bin: dumpbook book.txt mscp.c Makefile
	./dumpbook

clean:
	-rm ${OBJS} *.gt1 *.prf *.frg prof.txt dumpbook mscp

# Profiling support

GTSIM=gtsim -rom ../../../gigatron-rom/dev128k7.rom -vmode 1975
GTPROF=gtprof
CFILES=${OBJS:.o=.c}

prof.txt: mscp-sim.prf
	${GTPROF} mscp-sim.prf mscp-sim.frg | sort -nr > $@

mscp-sim.gt1: ${CFILES} ${ASMS} mscp.ovl
	${GLCC} -rom=${ROM} -map=sim,allout,./mscp.ovl \
	    -DREPEATABLE_RND=1 ${CFILES} ${ASMS} -o $@ --frags > mscp-sim.frg
	${PYTHON} addbook.py $@ book.bin

mscp-sim.prf: mscp-sim.gt1 prof.in
	${GTSIM} -prof $@ mscp-sim.gt1 < prof.in


# Old version

mscp0.gt1: mscp0.c
	${GLCC} -rom=${ROM} -map=sim,allout -DAVOID_SCANF $< -o $@


