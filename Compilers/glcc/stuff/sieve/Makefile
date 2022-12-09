
CC=../../build/glcc
ROM=dev7
MAP=64k,hionly
DEFS=
CFLAGS=-rom=$(ROM) -map=$(MAP) ${DEFS} -Dprintf=mincprintf

PGMS=sieve0.gt1 sieve1.gt1

all: $(PGMS)

sieve0.gt1: sieve0.c
	${CC} ${CFLAGS} sieve0.c -o $@

sieve1.gt1: sieve1.c
	${CC} ${CFLAGS} sieve1.c -o $@

clean:
	-rm *.gt1 *.s *.frg *.prf *-prof.txt


sieve0-sim.gt1: sieve0.c Makefile
	${CC} -rom=${ROM} -map=sim,hionly ${DEFA} $< -o $@ --frags > sieve0-sim.frg

sieve1-sim.gt1: sieve1.c Makefile
	${CC} -rom=${ROM} -map=sim,hionly ${DEFS} $< -o $@ --frags > sieve1-sim.frg

sieve0-prof.txt: sieve0-sim.gt1
	gtsim -rom ../../gigatron/roms/${ROM}.rom -vmode 1975 -prof sieve0-sim.prf sieve0-sim.gt1
	gtprof sieve0-sim.prf sieve0-sim.frg > sieve0-prof.txt

sieve1-prof.txt: sieve1-sim.gt1
	gtsim -rom ../../gigatron/roms/${ROM}.rom -vmode 1975 -prof sieve1-sim.prf sieve1-sim.gt1
	gtprof sieve1-sim.prf sieve1-sim.frg > sieve1-prof.txt
