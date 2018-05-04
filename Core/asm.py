
# SYNOPSIS: from asm import *

from os.path import basename, splitext
from sys import argv
import pickle

# Module variables because I don't feel like making a class
_romSize, _maxRomSize, _zpSize = 0, 0, 1
_symbols, _refsL, _refsH = {}, [], []
_labels = {} # Inverse of _symbols, but only when made with label(). For disassembler
_comments = {}
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
  address = _romSize
  define(name, address)
  if address not in _labels:
    _labels[address] = [] # There can be more than one
  _labels[_romSize].append(name)

def C(line):
  """Insert comment to print in disassembly"""
  if line:
    address = max(0, _romSize-1)
    if address not in _comments:
      _comments[address] = []
    _comments[address].append(line)
  return None

def define(name, value):
  global _symbols
  _symbols[name] = value

def symbol(name):
  return _symbols[name] if name in _symbols else None

def lo(name):
  global _refsL
  _refsL.append((name, _romSize))
  return 0 # placeholder

def hi(name):
  global _refsH
  _refsH.append((name, _romSize))
  return 0 # placeholder

def disassemble(opcode, operand, address=None):
  text = _mnemonics[opcode >> 5] # (74LS155)
  isStore = (opcode & 0xe0) == _opST

  # Decode addressing and register mode (74LS138)
  if text != 'j':
    if opcode & _maskMode == ea0DregAC:    ea, reg = '[%s]' % _hexString(operand), 'ac'
    if opcode & _maskMode == ea0XregAC:    ea, reg = '[x]', 'ac'
    if opcode & _maskMode == eaYDregAC:    ea, reg = '[y,%s]' % _hexString(operand), 'ac'
    if opcode & _maskMode == eaYXregAC:    ea, reg = '[y,x]', 'ac'
    if opcode & _maskMode == ea0DregX:     ea, reg = '[%s]' % _hexString(operand), 'x'
    if opcode & _maskMode == ea0DregY:     ea, reg = '[%s]' % _hexString(operand), 'y'
    if opcode & _maskMode == ea0DregOUT:   ea, reg = '[%s]' % _hexString(operand), 'out'
    if opcode & _maskMode == eaYXregOUTIX: ea, reg = '[y,x++]', 'out'
  else:
    ea = '[%s]' % _hexString(operand)

  # Decode bus mode (74LS139)
  if opcode & _maskBus == busD:   bus = _hexString(operand)
  if opcode & _maskBus == busRAM: bus = '$??' if isStore else ea
  if opcode & _maskBus == busAC:  bus = 'ac'
  if opcode & _maskBus == busIN:  bus = 'in'

  if text == 'j':
    # Decode jumping mode (74LS153)
    if opcode & _maskCc == jL:  text = 'jmp  y,'
    if opcode & _maskCc == jS:  text = 'bra  '
    if opcode & _maskCc == jEQ: text = 'beq  '
    if opcode & _maskCc == jNE: text = 'bne  '
    if opcode & _maskCc == jGT: text = 'bgt  '
    if opcode & _maskCc == jGE: text = 'bge  '
    if opcode & _maskCc == jLT: text = 'blt  '
    if opcode & _maskCc == jLE: text = 'ble  '
    if address is not None and opcode & _maskCc != jL and opcode & _maskBus == busD:
      # We can calculate the destination address
      # XXX Except when the previous instruction is a far jump (jmp y,...)
      lo, hi = address&255, address>>8
      if lo == 255: # When branching from $xxFF, we still end up in the next page
        hi = (hi + 1) & 255
      destination = (hi << 8) + operand
      if destination in _labels:
        bus = _labels[destination][-1]
      else:
        bus = '$%04x' % destination
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
  global _rom0, _rom1, _romSize, _maxRomSize
  opcode, operand = ins & 255, ins >> 8
  if _romSize >= _maxRomSize:
      disassembly = disassemble(opcode, operand)
      print '%04x %02x%02x  %s' % (_romSize, opcode, operand, disassembly)
      print 'Error: Program size limit exceeded'
      _maxRomSize = 0x10000 # Extend to full address space to prevent more of the same errors
  _rom0.append(opcode)
  _rom1.append(operand)
  _romSize += 1

  # Warning for conditional branches with a target address from RAM. The (unverified) danger is
  # that the ALU is calculating `-A' (for the condition decoder) as L+R+1, with L=0 and R=~A. But B
  # is also an input to R and comes from memory. The addressing mode is [D], which requires high
  # EH and EL, and this is slower when the diodes are forward biased from the previous instruction.
  # Therefore R might momentarily glitch while B changes value and the AND/OR layers in the 74153
  # multiplexer resettles. Such a glitch then potentially ripples all the way through two 74283
  # adders and the control unit's 74153. This all depends on the previous instruction's addressing
  # mode and the values of AC and [D], which we can't know with static analysis.
  if opcode & _maskOp == _opJ and\
    opcode & _maskBus == busRAM and\
    opcode & _maskCc in [ jGT, jLT, jNE, jEQ, jGE, jLE ]:
    disassembly = disassemble(opcode, operand)
    print '%04x %02x%02x  %s' % (_romSize, opcode, operand, disassembly)
    print 'Warning: large propagation delay (conditional branch with RAM on bus)'

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

def align(n, chunkSize=0x10000):
  global _romSize, _maxRomSize
  _maxRomSize = 0x10000
  while _romSize % n > 0:
    nop()
  _maxRomSize = min(_maxRomSize, _romSize + chunkSize)

def wait(n):
  comment = 'Wait %s cycle%s' % (n, '' if n==1 else 's')
  assert n >= 0
  if n > 4:
    n -= 1
    ld(val(n/2 - 1))
    comment = C(comment)
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
  if s <= 0x80 and 0x80 < s + len:
   s = 0x81 # Keep 0x80 reserved
  _zpSize = s+len
  assert _zpSize <= 0x100
  return s

def zpReset(startFrom=1):
  global _zpSize
  _zpSize = startFrom

def trampoline():
  """Read 1 byte from ROM page"""
  while pc()&255 < 256-5:
    nop()
  bra(busAC);                   #13
  """
     It is possible to make this section 2 bytes shorter
     and 1 cycle faster by entering directly wih "jmp y,ac"
     instead of "jmp y,251". However, this will cost two
     words at 'LUP' in vCPU and space is expensive there.
  """
  C('+-----------------------------------+')
  bra(val(253))                 #14
  C('|                                   |')
  ld(d(hi('lupReturn')),regY)   #15
  C('| Trampoline for page $%04x lookups |' % (pc()&~255))
  jmpy(d(lo('lupReturn')))      #17
  C('|                                   |')
  st(d(lo('vAC')))              #18
  C('+-----------------------------------+')

def link(symfile):
  global _symbols
  with open(symfile, 'rb') as file:
    _symbols = pickle.load(file)

def end(stem=None):
  errors = 0

  global _rom0, _rom1, _romSize
  global _refsL
  for name, where in _refsL:
    if name in _symbols:
      _rom1[where] ^= _symbols[name] & 255 # xor allows some label tricks
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
  if stem is None:
    stem, _ = splitext(argv[0])
    stem = basename(stem)
  if stem == '': stem = 'out'

  # Disassemble for readability
  filename = stem + '.asm'
  print 'Create file', filename
  with open(filename, 'w') as file:
    file.write('              address\n'
               '              |    encoding\n'
               '              |    |     instruction\n'
               '              |    |     |    operands\n'
               '              |    |     |    |\n'
               '              V    V     V    V\n')
    address = 0
    repeats, previous, postponed = 0, None, None
    maxRepeat = 3

    for instruction in zip(_rom0, _rom1):
      # Check if there is a label defined for this address
      label = _labels[address][-1] + ':' if address in _labels else ''
      comment = _comments[address][0] if address in _comments else ''

      if instruction != previous or label or comment:
        repeats, previous = 0, instruction
        if postponed:
          file.write(postponed)
          postponed = None
        if label:
          for extra in _labels[address][:-1]:
            file.write(extra+':\n') # Extra labels get their own line
          if len(label) > 13:
            label += '\n' + (13 * ' ')
      else:
        repeats += 1

      if repeats <= maxRepeat:
        opcode, operand = instruction
        disassembly = disassemble(opcode, operand, address)
        if comment:
          line = '%-13s %04x %02x%02x  %-16s ;%s\n' % (label, address, opcode, operand, disassembly, comment)
        else:
          line = '%-13s %04x %02x%02x  %s\n' % (label, address, opcode, operand, disassembly)

      if repeats < maxRepeat:
        file.write(line) # always write first N
        if comment:
          for extra in _comments[address][1:]:
            file.write(42*' ' + ';%s\n' % extra)
      if repeats == maxRepeat:
        postponed = line # if this turns out to  be the last repeat, emit the line
      if repeats > maxRepeat: # now it makes sense to abbreviate the output
        postponed = 14*' '+'* %d times\n' % (1+repeats)

      address += 1

    if postponed:
      file.write(postponed)
    file.write(14*' '+'%04x\n' % address)
    assert(len(_rom0) == _romSize)
    assert(len(_rom1) == _romSize)

  # Write symbol file
  filename = stem + '.sym'
  print 'Create file', filename
  with open(filename, 'wb') as file:
    pickle.dump(_symbols, file)

  # Write ROM files
  filename = stem + '.0.rom'
  print 'Create file', filename
  with open(filename, 'wb') as file:
    file.write(''.join([chr(byte) for byte in _rom0]))

  filename = stem + '.1.rom'
  print 'Create file', filename
  with open(filename, 'wb') as file:
    file.write(''.join([chr(byte) for byte in _rom1]))

  # 16-bit version for 27C1024, little endian
  filename = stem + '.2.rom'
  print 'Create file', filename
  _rom2 = []
  for x, y in zip(_rom0, _rom1):
    _rom2.append(x)
    _rom2.append(y)
  # Padding
  while len(_rom2) < 2*_maxRomSize:
    _rom2.append(ord('Gigatron!'[ (len(_rom2)-2*_maxRomSize) % 9 ]))
  # Write ROM file
  with open(filename, 'wb') as file:
    file.write(''.join([chr(byte) for byte in _rom2]))

  print 'OK used %d free %d size %d' % (_romSize, _maxRomSize-_romSize, len(_rom2))

