TOP=../../..
TOOLS=${TOP}/tools
HW=${TOP}/hw
GTBASIC=${TOOLS}/gtbasic/gtbasic
GTHEXDUMP=python3 ${TOOLS}/python/gtHexDump.py

all: Browser_SDCard_ROMv2_v1.gt1 Browser_SDCard_ROMv2_v1.h \
     Browser_SDCard_ROMv5a_v1.gt1 Browser_SDCard_ROMv5a_v1.h
ifdef OS
	copy Browser_SDCard_ROMv2_v1.h ${HW}/SDCard_NO_ROM/BabelFish
	copy Browser_SDCard_ROMv2_v1.gt1 ${HW}/SDCard_NO_ROM/BabelFish
	copy Browser_SDCard_ROMv5a_v1.gt1  ${HW}/SDCard_ROM
else
	cp Browser_SDCard_ROMv2_v1.h ${HW}/SDCard_NO_ROM/BabelFish
	cp Browser_SDCard_ROMv2_v1.gt1 ${HW}/SDCard_NO_ROM/BabelFish
	cp Browser_SDCard_ROMv5a_v1.gt1  ${HW}/SDCard_ROM
endif

%.gt1: %.gbas
	${GTBASIC} $<

%.h: %.gt1
	${GTHEXDUMP} $< > $@

clean: FORCE
ifdef OS
	-del *.h *.gt1 *.gasm
else
	-rm 2>/dev/null  *.h *.gt1 *.gasm
endif

FORCE:

.PHONY: FORCE
