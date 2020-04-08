#!/usr/bin/env python3

from vasm import *

p=0x30
(
                ORG(0x300),

                LDWI(0x800),
                STW(p),
L('Loop'),      POKE(p),
                ADDI(1),
                BRA('Loop'),

                END(0x300)      # Execution address
)
