# Please build this from the Makefile
from vasm import *

p=0x30                          # User variables start from $30 in zero page

ORG(0x200)                      # User programs can start from $200
-               LDWI(0x800)     # Top left pixel
-               STW(p)          # Pointer variable p
L('Loop');      POKE(p)         # Update pixel
-               ADDI(1)         # Increment vAC
-               BRA('Loop')     # Loop forever

END(0x200)                      # Execution start address
