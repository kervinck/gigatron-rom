
# SYNOPSIS: from vasm import *
#
# This is the (back end for) a minimalistic vCPU assembler in Python.
# Use Python itself as the front end.

import sys

_gt1, _symbols = [], {}

def L(name):
  _symbols[name] = _gt1[-2] + len(_gt1[-1])

def ORG(addr): _gt1.extend((addr,[]))

def LDWI(op):  _gt1[-1].extend((0x11, op & 255, op >> 8))
def LD(op):    _gt1[-1].extend((0x1a, op))
def LDW(op):   _gt1[-1].extend((0x21, op))
def STW(op):   _gt1[-1].extend((0x2b, op))
def BEQ(op):   _gt1[-1].extend((0x35, 0x3f, op))
def BGT(op):   _gt1[-1].extend((0x35, 0x4d, op))
def BLT(op):   _gt1[-1].extend((0x35, 0x50, op))
def BGE(op):   _gt1[-1].extend((0x35, 0x53, op))
def BLE(op):   _gt1[-1].extend((0x35, 0x56, op))
def LDI(op):   _gt1[-1].extend((0x59, op))
def ST(op):    _gt1[-1].extend((0x5e, op))
def POP():     _gt1[-1].extend((0x63))
def BNE(op):   _gt1[-1].extend((0x35, 0x72, op))
def PUSH():    _gt1[-1].extend((0x75))
def LUP(op):   _gt1[-1].extend((0x7f, op))
def ANDI(op):  _gt1[-1].extend((0x82, op))
def ORI(op):   _gt1[-1].extend((0x88, op))
def XORI(op):  _gt1[-1].extend((0x8c, op))
def BRA(op):   _gt1[-1].extend((0x90, (op,-2)))
def INC(op):   _gt1[-1].extend((0x93, op))
def ADDW(op):  _gt1[-1].extend((0x99, op))
def PEEK(op):  _gt1[-1].extend((0xad, op))
def SYS(op):   _gt1[-1].extend((0xb4, op))
def SUBW(op):  _gt1[-1].extend((0xb8, op))
def DEF(op):   _gt1[-1].extend((0xcd, op))
def CALL(op):  _gt1[-1].extend((0xcf, op))
def ALLOC(op): _gt1[-1].extend((0xdf, op))
def ADDI(op):  _gt1[-1].extend((0xe3, op))
def SUBI(op):  _gt1[-1].extend((0xe6, op))
def LSLW(op):  _gt1[-1].extend((0xe9, op))
def STLW(op):  _gt1[-1].extend((0xec, op))
def LDLW(op):  _gt1[-1].extend((0xee, op))
def POKE(op):  _gt1[-1].extend((0xf0, op))
def DOKE(op):  _gt1[-1].extend((0xf3, op))
def DEEK(op):  _gt1[-1].extend((0xf6, op))
def ANDW(op):  _gt1[-1].extend((0xf8, op))
def ORW(op):   _gt1[-1].extend((0xfa, op))
def XORW(op):  _gt1[-1].extend((0xfc, op))
def RET():     _gt1[-1].extend((0xff))

def END(start):
  with open('out.gt1', 'wb') as fp:
    for ix in range(0, len(_gt1), 2):
      addr, seg = _gt1[ix:ix+2]
      if len(seg) == 0:
        continue
      if addr + len(seg) > (addr | 255) + 1:
        print('Error: page overrun in segment 0x%04X' % addr)
        sys.exit(1)
      seg = [_resolve(i) for i in seg]
      seg = [addr>>8, addr&255, len(seg) & 255] + seg
      fp.write(bytes(seg))
    fp.write(bytes([0, start>>8, start&255]))

def _resolve(op):
  offset = 0
  if isinstance(op, tuple):
    op, offset = op
  if isinstance(op, str):
    op = _symbols[op]
  return (op + offset) & 255

