ROM=v6
GLCC=../../../Compilers/glcc/build/glcc
D=../

default: ${D}gtmine.gt1

all: ${D}gtmine.gt1 ${D}gtmine64.gt1

${D}gtmine.gt1: gtmine.c
	${GLCC} -o $@ gtmine.c -DMEM32=1 -map=32k,./gtmine32.ovl -rom=${ROM}

${D}gtmine64.gt1: gtmine.c
	${GLCC} -o $@ gtmine.c -DMEM32=0 -map=64k -rom=${ROM}

clean: FORCE
	-rm ${D}gtmine.gt1 ${D}gtmine64.gt1

FORCE:
