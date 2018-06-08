
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

# Simplified notation
X       = ea0DregX
Y       = ea0DregY
OUT     = ea0DregOUT
YX      = eaYXregAC
YX_incX = eaYXregOUTIX
def zp(d):    return ((busRAM | ea0DregAC) << 8) | (d & 255)
def ramX():   return  (busRAM | ea0XregAC) << 8
def ramY(d):  return ((busRAM | eaYDregAC) << 8) | (d & 255)
def ram():    return  (busRAM | eaYXregAC) << 8
AC      = busAC << 8 # Use only as input, only as first argument to instruction

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
  _symbols[name] = value

def symbol(name):
  return _symbols[name] if name in _symbols else None

def lo(name):
  _refsL.append((name, _romSize))
  return 0 # placeholder

def hi(name):
  _refsH.append((name, _romSize))
  return 0 # placeholder

def H(word):
  return word >> 8

def L(word):
   return word & 255

def disassemble(opcode, operand, address=None):
  text = _mnemonics[opcode >> 5] # (74LS155)
  isStore = (opcode & _maskOp) == _opST

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
      lo, hi = L(address), H(address)
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
      if opcode == _opLD | busAC: text = 'nop'

  # Emit as text
  return text

def _emit(a, b):
  opcode, operand = a & 255, b & 255

  if b >= 256:
    opcode &= ~_maskBus
    opcode |= b >> 8

  # The addressing mode helpers all set busRAM, but these also useful for 'st'.
  # So catch that usage and change their bus mode to busAC
  if opcode & _maskOp == _opST and opcode & _maskBus == busRAM:
    opcode = (opcode & ~_maskBus) | busAC

  global _romSize, _maxRomSize
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

# Mnemonics for native instruction set
def nop ():                       _emit(_opLD  | 0    | busAC, 0)
def ld  (d=0, mode=0, bus=busD):  _emit(_opLD  | mode | bus, d)
def anda(d=0, mode=0, bus=busD):  _emit(_opAND | mode | bus, d)
def ora (d=0, mode=0, bus=busD):  _emit(_opOR  | mode | bus, d)
def xora(d=0, mode=0, bus=busD):  _emit(_opXOR | mode | bus, d)
def adda(d=0, mode=0, bus=busD):  _emit(_opADD | mode | bus, d)
def suba(d=0, mode=0, bus=busD):  _emit(_opSUB | mode | bus, d)
def st  (d=0, mode=0, bus=busAC): _emit(_opST  | mode | bus, d)
def bra (d=0, bus=busD):          _emit(_opJ   | jS   | bus, d)
def beq (d=0, bus=busD):          _emit(_opJ   | jEQ  | bus, d)
def bne (d=0, bus=busD):          _emit(_opJ   | jNE  | bus, d)
def bgt (d=0, bus=busD):          _emit(_opJ   | jGT  | bus, d)
def blt (d=0, bus=busD):          _emit(_opJ   | jLT  | bus, d)
def bge (d=0, bus=busD):          _emit(_opJ   | jGE  | bus, d)
def ble (d=0, bus=busD):          _emit(_opJ   | jLE  | bus, d)
def jmpy(d=0, bus=busD):          _emit(_opJ   | jL   | bus, d)

bpl = bge # Alias
bmi = blt # Alias

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
    ld(n/2 - 1)
    comment = C(comment)
    bne(_romSize & 255)
    suba(1)
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
  while L(pc()) < 256-5:
    nop()
  bra(AC)                       #13
  """
     It is possible to make this section 2 bytes shorter
     and 1 cycle faster by entering directly wih "jmp y,ac"
     instead of "jmp y,251". However, this will cost two
     words at 'LUP' in vCPU and space is expensive there.
  """
  C('+-----------------------------------+')
  bra(253)                      #14
  C('|                                   |')
  ld(hi('lupReturn'), Y)        #15
  C('| Trampoline for page $%04x lookups |' % (pc()&~255))
  jmpy(lo('lupReturn'))         #17
  C('|                                   |')
  st(lo('vAC'))                 #18
  C('+-----------------------------------+')

def loadBindings(symfile):
  global _symbols
  with open(symfile, 'rb') as file:
    _symbols = pickle.load(file)

def end():
  errors = 0

  for name, where in _refsL:
    if name in _symbols:
      _rom1[where] ^= _symbols[name] & 255 # xor allows some label tricks
    else:
      print 'Error: Undefined symbol %s' % repr(name)
      errors += 1

  for name, where in _refsH:
    if name in _symbols:
      _rom1[where] += _symbols[name] >> 8
    else:
      print 'Error: Undefined symbol %s' % repr(name)
      errors += 1

  if errors:
    print '%d error(s)' % errors
    exit()

def getRom1():
  return ''.join(chr(byte) for byte in _rom1)

def writeRomFiles(sourceFile):

  # Determine stem for file names
  stem, _ = splitext(sourceFile)
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
  # XXX Remove when compiler can work with "bindings.json"
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

