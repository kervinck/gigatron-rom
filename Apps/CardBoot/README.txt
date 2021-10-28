This directory used to contain a collection of files related 
to Marcel's initial plans for a GTOS.  These files have been 
moved into the "old" directory.

The only remaining file now is the CardBoot program that the
Gigatron Reset program executes when it senses the presence of
a SD card attached to the SPI0 channel of a RAM & IO expansion
board. The only function of this lightweight program is to load 
and execute file "SYSTEM.GT1" found in the main directory
of a FAT32 formatted SD card.

