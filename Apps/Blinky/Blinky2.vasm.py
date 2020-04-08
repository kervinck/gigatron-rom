# Please build this from the Makefile
from vasm import *

p=0x30                          # User variables start from $30 in zero page

ORG(0x300)                      # User programs can start from $300
-               LDWI(0x800)     # Top left pixel
-               STW(p)          # Pointer variable p
L('Loop');      POKE(p)         # Update pixel
-               ADDI(1)         # Increment vAC
-               BRA('Loop')     # Loop forever

END(0x300)                      # Execution start address
