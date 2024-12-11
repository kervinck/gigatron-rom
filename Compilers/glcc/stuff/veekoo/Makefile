ROM=dev7
RUNROM=
MAP=32k
B=../../build/
CC=${B}glcc
GTSIM=${B}gtsim -rom ${or ${RUNROM},../../gigatron/roms/${ROM}.rom}
GTPROF=${B}/../gigatron/mapsim/gtprof

PGMS=ascbrot.gt1

all:${PGMS}


%.gt1: %.c
	${CC} -rom=${ROM} -map=${MAP} $< -o $@


%.prof: %.c
	${CC} -rom=${ROM} -map=sim,hionly $< -o $*-sim.gt1 --frags > $*-sim.frg
	-${GTSIM} -prof $*-sim.prf $*-sim.gt1
	${GTPROF} $*-sim.prf $*-sim.frg | sort -nr > $@


clean: FORCE
	-rm *.gt1 *.prf *.frg *.prof

FORCE:
