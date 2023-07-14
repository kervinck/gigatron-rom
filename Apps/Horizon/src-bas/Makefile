
GTBASIC=../../../Contrib/at67/tools/gtbasic/gtbasic
GTASM=../../../Contrib/at67/tools/gtasm/gtasm
TARGET=../Horizon_at67.gt1


$(TARGET): Horizon.gt1
	cp Horizon.gt1 $@

%.gt1: %.gasm
	$(GTASM) $<

%.gasm: %.gbas
	$(GTBASIC) $<

clean:
	-rm $(TARGET) Horizon.gt1 Horizon.gasm

