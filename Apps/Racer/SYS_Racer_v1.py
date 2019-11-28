# -*- coding: utf-8 -*-
#-----------------------------------------------------------------------
#
#       Racer-specific SYS extensions
#
#-----------------------------------------------------------------------
from __future__ import (
    absolute_import,
    division,
    print_function,
    unicode_literals,
)

from asm import *

# Peek into the ROM's symbol table
sysArgs = symbol('sysArgs0')
vTmp = symbol('vTmp')
vPC = symbol('vPC')
vAC = symbol('vAC')

#-----------------------------------------------------------------------
#       SYS_RacerUpdateVideoX_40
#-----------------------------------------------------------------------

# Updates the odd videoTable entries for rendering the road curvature.
# This SYS call self-repeats for a total of 47 times and is equivalent to:
#       [do
#         q, X- p. p 4- p=
#         q, X= q<++
#         q<, if<>0loop]
#

label('SYS_RacerUpdateVideoX_40')
ld([sysArgs+2],X)               #15 q,
ld([sysArgs+3],Y)               #16
ld([Y,X])                       #17
st([vTmp])                      #18
suba([sysArgs+4])               #19 X-
ld([sysArgs+0],X)               #20 p.
ld([sysArgs+1],Y)               #21
st([Y,X])                       #22
ld([sysArgs+0])                 #23 p 4- p=
suba(4)                         #24
st([sysArgs+0])                 #25
ld([vTmp])                      #26 q,
st([sysArgs+4])                 #27 X=
ld([sysArgs+2])                 #28 <q++
adda(1)                         #29
st([sysArgs+2])                 #30
bne('.sysRacer0')               #31 Self-repeat by adjusting vPC
ld([vPC])                       #32
bra('.sysRacer1')               #33
nop()                           #34
label('.sysRacer0')
suba(2)                         #33
st([vPC])                       #34
label('.sysRacer1')
ld(hi('REENTER'),Y)             #35
jmp(Y,'REENTER')                #36
ld(-40//2)                      #37

#-----------------------------------------------------------------------
#       SYS_RacerUpdateVideoY_40
#-----------------------------------------------------------------------

# Updates an even videoTable entry for rendering grass and curbstone color.
# Native Gigatron accelaration for inner loop body:
#
#       8& [if<>0 1] tmp=
#       SegmentY 254& tmp+ p.
#       p<++ p<++
#       SegmentY<++
#       p<, 238^

label('SYS_RacerUpdateVideoY_40')
ld([sysArgs+3])                 #15 8&
anda(8)                         #16
bne(pc()+3)                     #17 [if<>0 1]
bra(pc()+3)                     #18
ld(0)                           #19
ld(1)                           #19(!)
st([vTmp])                      #20 tmp=
ld([sysArgs+1],Y)               #21
ld([sysArgs+0])                 #22 <p++ <p++
adda(2)                         #23
st([sysArgs+0],X)               #24
xora(238)                       #25 238^
st([vAC])                       #26
st([vAC+1])                     #27
ld([sysArgs+2])                 #28 SegmentY
anda(254)                       #29 254&
adda([vTmp])                    #30 tmp+
st([Y,X])                       #31
ld([sysArgs+2])                 #32 <SegmentY++
adda(1)                         #33
st([sysArgs+2])                 #34
ld(hi('REENTER'),Y)             #35
jmp(Y,'REENTER')                #36
ld(-40//2)                      #37

#-----------------------------------------------------------------------
#
#-----------------------------------------------------------------------

