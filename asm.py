
# SYNOPSIS: from asm import *

from os.path import basename, splitext
from sys import argv

# Module variables because I don't feel like making a class
_romSize, _zpSize, _symbols, _refsL, _refsH = 0, 0, {}, [], []
_rom0, _rom1 = [], []

# Bus access
busD   = 0
busRAM = 1
busAC  = 2
busIN  = 3

# Addressing modes
#
# How addresses into RAM are composed.
# In a sufficiently large RAM system there must be both immediate
# (absolute) addresses and computed addresses.
#
# A reasonable EAU could comprise of a 16-bit data pointer, DP, to which
# an immediate 8-bit offset is optionally added, with the option to disable
# the DP (so we have zero page addressing). Such a unit requires 4 TTL
# adders + 6 TTL AND chips = 10 TTL chips.
#
# Our EA unit is a "poor man's" EAU that supports some workable combinations.
# There is no addition, so no linear address space, just selecting which
# part go into which half. It uses 4 TTL chips. The data pointer DP is
# replaced with 8-bit X and 8-bit Y.
#
# Register and addressing modes are compacted to 8 combinations
#
ea0DregAC    = 0 << 2
ea0XregAC    = 1 << 2
eaYDregAC    = 2 << 2
eaYXregAC    = 3 << 2
ea0DregX     = 4 << 2
ea0DregY     = 5 << 2
ea0DregOUT   = 6 << 2
eaYXregOUTIX = 7 << 2 # post-increment of X

# Store instructions
def ea0D(v): return ea0DregAC | d(v)
ea0X    = ea0XregAC
def eaYD(v): return eaYDregAC | d(v)
eaYX    = eaYXregAC
eaYXinc = eaYXregOUTIX

# Load/exec instructions (without memory)
regAC  = ea0DregAC
regX   = ea0DregX
regY   = ea0DregY
regOUT = ea0DregOUT

# Immediate value
#
# Immediate means that the value used is encoded within the program stream,
# instead of, for example, coming from a register or memory location.
#
def val(v): return busD | d(v)

def d(v): return ((v & 255) << 8)

def ram(ea): return busRAM | ea

# General instruction layout
_maskOp   = 0b11100000
_maskMode = 0b00011100
_maskCc   = 0b00011100
_maskBus  = 0b00000011

# Operations
_opLD  = 0 << 5
_opAND = 1 << 5
_opOR  = 2 << 5
_opXOR = 3 << 5
_opADD = 4 << 5
_opSUB = 5 << 5
_opST  = 6 << 5
_opJ   = 7 << 5

# No operation
_nops = [_opLD  | regAC | busAC,
         _opAND | regAC | busAC,
         _opOR  | regAC | busAC ]
_clrs = [_opXOR | regAC | busAC,
         _opSUB | regAC | busAC ]

# Jump conditions
#
# During jump, the ALU is wired to calculate "-AC".
# Only the overflow flag is looked at. The negation result itself is not used and discarded.
# Only in case of all zero bits, -AC overflows the ALU. So the overflow acts as a zero flag (Z).
# If we look at bit 7 of AC, we know if AC is negative or positive.
# What does the combination of two signals tell us:
#
#  Z bit7
#  0  0  | AC>0  |  0  1  0  1  0  1  0  1   Instruction bit 2
#  0  1  | AC<0  |  0  0  1  1  0  0  1  1   Instruction bit 3
#  1  0  | AC=0  |  0  0  0  0  1  1  1  1   Instruction bit 4
#  1  1  | n/a    -------------------------
#                   F GT LT NE EQ GE LE  T   Condition code ("F" is repurposed for long jumps)
jL  = 0 << 2
jGT = 1 << 2
jLT = 2 << 2
jNE = 3 << 2
jEQ = 4 << 2
jGE = 5 << 2
jLE = 6 << 2
jS  = 7 << 2

_mnemonics = [ 'ld', 'anda', 'ora', 'xora', 'adda', 'suba', 'st', 'j' ]

def _hexString(val):
  return '$%02x' % val

def label(name):
  define(name, _romSize)

def define(name, value):
  global _romSize, _symbols
  _symbols[name] = value

def lo(name):
  global _refsL
  _refsL.append((name, _romSize))
  return 0 # placeholder

def hi(name):
  global _refsH
  _refsH.append((name, _romSize))
  return 0 # placeholder

def disassemble(opcode, operand):
  text = _mnemonics[opcode >> 5] # (74LS155)
  isStore = (opcode & 0xe0) == _opST

  # Decode addressing and register mode (74LS138)
  if text != 'j':
    if opcode & (7 << 2) == ea0DregAC:    ea, reg = '[%s]' % _hexString(operand), 'ac'
    if opcode & (7 << 2) == ea0XregAC:    ea, reg = '[x]', 'ac'
    if opcode & (7 << 2) == eaYDregAC:    ea, reg = '[y,%s]' % _hexString(operand), 'ac'
    if opcode & (7 << 2) == eaYXregAC:    ea, reg = '[y,x]', 'ac'
    if opcode & (7 << 2) == ea0DregX:     ea, reg = '[%s]' % _hexString(operand), 'x'
    if opcode & (7 << 2) == ea0DregY:     ea, reg = '[%s]' % _hexString(operand), 'y'
    if opcode & (7 << 2) == ea0DregOUT:   ea, reg = '[%s]' % _hexString(operand), 'out'
    if opcode & (7 << 2) == eaYXregOUTIX: ea, reg = '[y,x++]', 'out'
  else:
    ea = '[%s]' % _hexString(operand)

  # Decode bus mode (74LS139)
  if opcode & 3 == busD:   bus = _hexString(operand)
  if opcode & 3 == busRAM: bus = '$??' if isStore else ea
  if opcode & 3 == busAC:  bus = 'ac'
  if opcode & 3 == busIN:  bus = 'in'

  if text == 'j':
    # Decode jumping mode (74LS153)
    if opcode & (7 << 2) == jL:  text = 'jmp  y,'
    if opcode & (7 << 2) == jS:  text = 'bra  '
    if opcode & (7 << 2) == jEQ: text = 'beq  '
    if opcode & (7 << 2) == jNE: text = 'bne  '
    if opcode & (7 << 2) == jGT: text = 'bgt  '
    if opcode & (7 << 2) == jGE: text = 'bge  '
    if opcode & (7 << 2) == jLT: text = 'blt  '
    if opcode & (7 << 2) == jLE: text = 'ble  '
    text += bus
  else:
    # Compose string
    if isStore:
      if bus == 'ac':
        text = '%-4s %s' % (text, ea)
      else:
        text = '%-4s %s,%s' % (text, bus, ea)
      if reg != 'ac' and reg != 'out': # X and Y are not muted
        text += ',' + reg
    else:
      if reg == 'ac':
        text = '%-4s %s' % (text, bus)
      else:
        text = '%-4s %s,%s' % (text, bus, reg)
      # Specials
      if opcode in _nops: text = 'nop'
      if opcode in _clrs: text = 'clr'

  # Emit as text
  return text

def _emit(ins):
  global _rom0, _rom1, _romSize
  opcode, operand = ins & 255, ins >> 8
  _rom0.append(opcode)
  _rom1.append(operand)
  _romSize += 1

  # Warning for time critical instruction combination
  if _romSize >= 2 and _rom0[_romSize-2] & _maskOp == _opJ and\
    opcode & _maskBus == busRAM and\
    opcode & _maskMode not in [ea0DregAC, ea0DregX, ea0DregY, ea0DregOUT] and\
    opcode & _maskOp in [_opADD, _opSUB]:
    for address in range(_romSize-2, _romSize):
      opcode, operand = _rom0[address], _rom1[address]
      # XXX disassembly at this point isn't correct until phase 2 has resolved all symbols
      disassembly = disassemble(opcode, operand)
      print '%04x %02x%02x  %s' % (_romSize, opcode, operand, disassembly)
    print 'Warning: large propagation delay (artihmetic on RAM operand after jump and mode change)'

def ld  (base, reg=regAC, flags=0): _emit(_opLD  | base | reg | flags)
def anda(base, reg=regAC, flags=0): _emit(_opAND | base | reg | flags)
def ora (base, reg=regAC, flags=0): _emit(_opOR  | base | reg | flags)
def xora(base, reg=regAC, flags=0): _emit(_opXOR | base | reg | flags)
def adda(base, reg=regAC, flags=0): _emit(_opADD | base | reg | flags)
def suba(base, reg=regAC, flags=0): _emit(_opSUB | base | reg | flags)
def jmpy(base): _emit(_opJ | jL | base)
def bra (base): _emit(_opJ | jS | base)
def beq (base): _emit(_opJ | jEQ | base)
def bne (base): _emit(_opJ | jNE | base)
def bgt (base): _emit(_opJ | jGT | base)
def blt (base): _emit(_opJ | jLT | base)
def bge (base): _emit(_opJ | jGE | base)
def ble (base): _emit(_opJ | jLE | base)
bpl = bge # Alias
bmi = blt # Alias
def nop ():     _emit(_nops[0])
def clr ():     _emit(_clrs[0])
def st  (base1, base2=busAC): _emit(_opST | base1 | base2)
def out (base=busAC): _emit(_opLD | base | regOUT)


def ldzp (base): _emit(_opLD | busRAM | ea0DregAC | base)
def ldzpx(base): _emit(_opLD | busRAM | ea0DregX  | base)
def ldzpy(base): _emit(_opLD | busRAM | ea0DregY  | base)

def align(n):
  while _romSize % n > 0:
    _emit(0)

def wait(n):
  assert n >= 0
  if n > 4:
    n -= 1
    ld(val(n/2 - 1))
    bne(d(_romSize & 255))
    suba(val(1))
    n = n % 2 
  while n > 0:
    nop()
    n -= 1

def pc():
  return _romSize

def zpByte(len=1):
  global _zpSize
  s = _zpSize
  _zpSize += len
  return s

def end():
  errors = 0

  global _rom0, _rom1, _romSize
  global _refsL
  for name, where in _refsL:
    if name in _symbols:
      _rom1[where] = _symbols[name] & 255
    else:
      print 'Error: Undefined symbol %s' % repr(name)
      errors += 1

  global _refsH
  for name, where in _refsH:
    if name in _symbols:
      _rom1[where] += _symbols[name] >> 8
    else:
      print 'Error: Undefined symbol %s' % repr(name)
      errors += 1

  if errors:
    print '%d error(s)' % errors
    exit()

  # Determine stem for file names
  stem, ext = splitext(argv[0])
  stem = basename(stem)
  if stem == '': stem = 'out'

  # Disassemble for readability
  filename = stem + '.asm'
  print 'Creating:', filename
  with open(filename, 'w') as file:
    file.write('address\n'
               '|    encoding\n'
               '|    |     instruction\n'
               '|    |     |    operands\n'
               '|    |     |    |\n'
               'V    V     V    V\n')
    address = 0
    repeats, previous, postponed = 0, None, None
    maxRepeat = 3
    for instruction in zip(_rom0, _rom1):

      if instruction != previous:
        repeats, previous = 0, instruction
        if postponed:
          file.write(postponed)
          postponed = None
      else:
        repeats += 1

      if repeats <= maxRepeat:
        opcode, operand = instruction
        disassembly = disassemble(opcode, operand)
        line = '%04x %02x%02x  %s\n' % (address, opcode, operand, disassembly)

      if repeats < maxRepeat:
        file.write(line) # always write first N
      if repeats == maxRepeat:
        postponed = line # if this turns out to  be the last repeat, emit the line
      if repeats > maxRepeat: # now it makes sense to abbreviate the output
        postponed = '* %d times\n' % (1+repeats)

      address += 1

    if postponed:
      file.write(postponed)
    file.write('%04x\n' % address)
    assert(len(_rom0) == _romSize)
    assert(len(_rom1) == _romSize)

  # Write ROM files
  filename = stem + '.0.rom'
  print 'Creating:', filename
  with open(filename, 'wb') as file:
    file.write(''.join([chr(byte) for byte in _rom0]))

  filename = stem + '.1.rom'
  print 'Creating:', filename
  with open(filename, 'wb') as file:
    file.write(''.join([chr(byte) for byte in _rom1]))

  # 16-bit version for 27C1024, little endian
  filename = stem + '.2.rom'
  print 'Creating:', filename
  _rom2 = []
  for x, y in zip(_rom0, _rom1):
    _rom2.append(x)
    _rom2.append(y)
  _rom2 += (65536 - len(_rom2)) * [0] # Pad to 32K
  _rom2 += _rom2                      # And for now, duplicate the upper half
  with open(filename, 'wb') as file:
    file.write(''.join([chr(byte) for byte in _rom2]))

  print 'OK: %d words' % _romSize

