
# SYNOPSIS: from asm import *

from os.path import basename, splitext
import json

#------------------------------------------------------------------------
#       Public interface
#------------------------------------------------------------------------

# These definitions allow the use of Python notation as an assembler.
# The syntax is close to assembly, but by using Python we get a
# powerful macro assembler for free.
#
# Some examples:
#       Python                  Equivalent assembly
#       ------                  -------------------
#       nop()                           nop
#       ld(1)                           ld   1
#       ld([0x30])                      ld   [$30]
#       ld([0x30], X)                   ld   [$30],x
#       ld(AC, Y)                       ld   ac,y
#       ld([Y,Xpp], OUT)                ld   [y,x++],out
#       adda(2)                         adda 2
#       adda(AC)                        adda ac
#       xora([0x30])                    xora [$30]
#       anda(0x2a)                      anda $2a
#       st([0x30])                      st   [$30]
#       st([0x30], Y)                   st   [$30],y
#       st([X])                         st   [x]
#       st([Y,1])                       st   [y,1]
#       st([Y,X])                       st   [y,x]
#       st([Y,Xpp])                     st   [y,x++]
#       st(0x2a, [Y,Xpp])               st   $2a,[y,x++]
#       label('loop')           loop:
#       bne('loop')                     bne  loop
#       bra([AC])                       bra  [ac]
#       ctrl(0x30)                      ctrl $30
#       ctrl(X)                         ctrl x
#       ctrl(Y, 0x30)                   ctrl y,$30
#       ctrl(Y, X)                      ctrl y,x
#       ctrl(Y, Xpp)                    ctrl y,x++
#       ctrl($30, X)                    ctrl $30,x      ; Also copies AC into X
#       ctrl($30, Y)                    ctrl $30,y      ; Also copies AC into Y

X   = '__x__'
Y   = '__y__'
Xpp = '__x++__'
OUT = '__out__'
IN  = '__in__'
AC  = '__ac__'

# Mnemonics for Gigatron native 8-bit instruction set
def nop (dummy=None):     _assemble(_opLD, AC)
def ld  (a, b=AC):        _assemble(_opLD,  a, b)
def anda(a, b=AC):        _assemble(_opAND, a, b)
def ora (a, b=AC):        _assemble(_opOR,  a, b)
def xora(a, b=AC):        _assemble(_opXOR, a, b)
def adda(a, b=AC):        _assemble(_opADD, a, b)
def suba(a, b=AC):        _assemble(_opSUB, a, b)
def _jmpy(a):             _assemble(_opJ|_jL,  a)
def bgt (a):              _assemble(_opJ|_jGT, a)
def blt (a):              _assemble(_opJ|_jLT, a)
def bne (a):              _assemble(_opJ|_jNE, a)
def beq (a):              _assemble(_opJ|_jEQ, a)
def bge (a):              _assemble(_opJ|_jGE, a)
def ble (a):              _assemble(_opJ|_jLE, a)
def bra (a):              _assemble(_opJ|_jS,  a)
def st  (a, b=None, c=None):
  if isinstance(a, list): _assemble(_opST, AC, b, a)
  else:                   _assemble(_opST, a,  c, b)
def ctrl(a, b=None):
  if a in [X, Y] and b:   _assemble(_opST|_busRAM, [a, b], None)
  else:                   _assemble(_opST|_busRAM, [a], b)
def jmp(a, b):
  assert a is Y
  _jmpy(b)

bpl = bge # Alias
bmi = blt # Alias

def label(name):
  """Label the current address"""
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

def define(name, newValue):
  if name in _symbols:
    oldValue =  _symbols[name]
    if newValue != oldValue:
      print('Warning: redefining {!s} (old {!s} new {!s})'.format(name, oldValue, newValue))
  _symbols[name] = newValue

def symbol(name):
  """Lookup a symbol, return None if not defined"""
  return _symbols[name] if name in _symbols else None

def has(x):
  """Useful primitive"""
  return x is not None

def lo(name):
  if isinstance(name, int):
    return name & 255
  else:
    _refsL.append((name, _romSize))
    return 0 # placeholder

def hi(name):
  if isinstance(name, int):
    return (name >> 8) & 255
  else:
    _refsH.append((name, _romSize))
    return 0 # placeholder

def align(m=0x100, chunkSize=0x10000):
  """Insert nops to align with chunk boundary"""
  global _maxRomSize
  n = (m - pc()) % m
  comment = 'filler' if n==1 else '{:d} fillers'.format(n)
  while pc() % m > 0:
    nop()
    comment = C(comment)
  _maxRomSize = min(0x10000, pc() + chunkSize)

def wait(n):
  """Insert delay sequence of n cycles. Might clobber AC"""
  comment = 'Wait {!s} cycle{!s}'.format(n, '' if n==1 else 's')
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
  """Current ROM address"""
  return _romSize

def zpByte(len=1):
  """Allocate one or more bytes from the zero-page"""
  global _zpSize
  s = _zpSize
  if s <= 0x80 and 0x80 < s + len:
   s = 0x81 # Keep 0x80 reserved
  _zpSize = s+len
  assert _zpSize <= 0x100
  return s

def zpReset(startFrom=1):
  """Reset zero-page allocation"""
  global _zpSize
  _zpSize = startFrom

def fillers(until=256, instruction=nop):
  """Insert fillers until given page offset"""
  n = until - (pc() & 255)
  comment = 'filler' if n==1 else '{:d} fillers'.format(n)
  for i in range(n):
    instruction(0)
    comment = C(comment)

def trampoline():
  """Read 1 byte from ROM page"""
  fillers(256-5)
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
  C('| Trampoline for page ${:04x} lookups |'.format((pc()&~255)))
  jmp(Y,lo('lupReturn'))        #17
  C('|                                   |')
  st([lo('vAC')])               #18
  C('+-----------------------------------+')
  align(1, 0x100)

def end():
  """Resolve symbols and write output"""
  global _errors

  for name, where in _refsL:
    if name in _symbols:
      _rom1[where] += _symbols[name] # adding allows some label tricks
      _rom1[where] &= 255
    else:
      print('Error: Undefined symbol {!s}'.format(repr(name)))
      _symbols[name] = 0 # No more errors
      _errors += 1

  for name, where in _refsH:
    if name in _symbols:
      _rom1[where] += _symbols[name] >> 8
    else:
      print('Error: Undefined symbol {!s}'.format(repr(name)))
      _errors += 1

  if _errors:
    print('{:d} error(s)\n'.format(_errors))
    exit()

  align(1)

#------------------------------------------------------------------------
#       Behind the scenes
#------------------------------------------------------------------------

# Module variables because I don't feel like making a class
_romSize, _maxRomSize, _zpSize = 0, 0, 1
_symbols, _refsL, _refsH = {}, [], []
_labels = {} # Inverse of _symbols, but only when made with label(). For disassembler
_comments = {}
_rom0, _rom1 = [], []
_errors = 0

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

# Bus access modes
_busD   = 0
_busRAM = 1
_busAC  = 2
_busIN  = 3

# Addressing modes
#
# How addresses into RAM are composed.
# In a sufficiently large RAM system there must be both immediate
# (absolute) addresses and computed addresses.
#
# A reasonable MAU could comprise of a 16-bit data pointer, DP, to which
# an immediate 8-bit offset is optionally added, with the option to disable
# the DP (so we have zero page addressing). Such a unit requires 4 TTL
# adders + 6 TTL AND chips = 10 TTL chips.
#
# Our address unit is a "poor man's" MAU that supports some workable combinations.
# There is no addition, so no linear address space, just selecting which
# part go into which half. It uses 4 TTL chips. The data pointer DP is
# replaced with 8-bit X and 8-bit Y.
#
# Register and addressing modes are compacted to 8 combinations
#
_ea0DregAC    = 0 << 2
_ea0XregAC    = 1 << 2
_eaYDregAC    = 2 << 2
_eaYXregAC    = 3 << 2
_ea0DregX     = 4 << 2
_ea0DregY     = 5 << 2
_ea0DregOUT   = 6 << 2
_eaYXregOUTIX = 7 << 2  # Post-increment of X

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
_jL  = 0 << 2
_jGT = 1 << 2
_jLT = 2 << 2
_jNE = 3 << 2
_jEQ = 4 << 2
_jGE = 5 << 2
_jLE = 6 << 2
_jS  = 7 << 2

def _assemble(op, val, to=AC, addr=None):
  """Assemble and emit one instruction"""
  d, mode, bus = 0, 0, 0                                # [D] (default)

  # First operand can be optional
  if isinstance(val, list):
    val, addr = None, val

  # Process list notation for addressing mode
  if isinstance(addr, list):
    if op != _opST: bus = _busRAM
    if addr[-1] not in [X, Xpp]:
      d = addr[-1]
    if addr[0] is Y:
      if   addr[-1] is X:   mode = _eaYXregAC           # [Y,X]
      elif addr[-1] is Xpp: mode = _eaYXregOUTIX        # [Y,X++]
      else:                 mode = _eaYDregAC           # [Y,D]
    elif   addr[-1] is X:   mode = _ea0XregAC           # [X]

  # Process target designation
  if to is X:      mode = _ea0DregX
  if to is Y:      mode = _ea0DregY
  if to is OUT and mode != _eaYXregOUTIX: mode = _ea0DregOUT

  # Check that addressing mode matches with any target designation
  assert to is None or to is [AC,AC,AC,AC,X,Y,OUT,OUT][mode>>2]

  # Process source designation
  if   val is AC: bus = _busAC
  elif val is IN: bus = _busIN
  elif isinstance(val, str): d = lo(val) # Convenient for branch instructions
  elif isinstance(val, int): d = val

  _emit(op | mode | bus, d & 255)

_mnemonics = [ 'ld', 'anda', 'ora', 'xora', 'adda', 'suba', 'st', 'j' ]

def _hexString(val):
  return '${:02x}'.format(val)

def disassemble(opcode, operand, address=None, lastOpcode=None):
  text = _mnemonics[opcode >> 5] # (74LS155)
  isStore = (opcode & _maskOp) == _opST

  # Decode addressing and register mode (74LS138)
  if text != 'j':
    if opcode & _maskMode == _ea0DregAC:    _ea, reg = '[{!s}]'.format(_hexString(operand)), 'ac'
    if opcode & _maskMode == _ea0XregAC:    _ea, reg = '[x]', 'ac'
    if opcode & _maskMode == _eaYDregAC:    _ea, reg = '[y,{!s}]'.format(_hexString(operand)), 'ac'
    if opcode & _maskMode == _eaYXregAC:    _ea, reg = '[y,x]', 'ac'
    if opcode & _maskMode == _ea0DregX:     _ea, reg = '[{!s}]'.format(_hexString(operand)), 'x'
    if opcode & _maskMode == _ea0DregY:     _ea, reg = '[{!s}]'.format(_hexString(operand)), 'y'
    if opcode & _maskMode == _ea0DregOUT:   _ea, reg = '[{!s}]'.format(_hexString(operand)), 'out'
    if opcode & _maskMode == _eaYXregOUTIX: _ea, reg = '[y,x++]', 'out'
  else:
    _ea = '[{!s}]'.format(_hexString(operand))

  # Decode bus mode (74LS139)
  if opcode & _maskBus == _busD:   bus = _hexString(operand)
  if opcode & _maskBus == _busRAM: bus = None if isStore else _ea
  if opcode & _maskBus == _busAC:  bus = 'ac'
  if opcode & _maskBus == _busIN:  bus = 'in'

  if text == 'j':
    # Decode jumping mode (74LS153)
    if opcode & _maskCc == _jL:  text = 'jmp  y,'
    if opcode & _maskCc == _jS:  text = 'bra  '
    if opcode & _maskCc == _jEQ: text = 'beq  '
    if opcode & _maskCc == _jNE: text = 'bne  '
    if opcode & _maskCc == _jGT: text = 'bgt  '
    if opcode & _maskCc == _jGE: text = 'bge  '
    if opcode & _maskCc == _jLT: text = 'blt  '
    if opcode & _maskCc == _jLE: text = 'ble  '
    if address is not None and opcode & _maskCc != _jL and opcode & _maskBus == _busD:
      # We can calculate the destination address
      lo, hi = address & 255, address >> 8
      if lo == 255: # When branching from $xxFF, we still end up in the next page
        hi = (hi + 1) & 255
      destination = (hi << 8) + operand
      if lastOpcode & (_maskOp|_maskCc) == _opJ|_jL:
        bus = '${:02x}'.format(operand)
      elif destination in _labels:
        bus = _labels[destination][-1]
      else:
        bus = '${:04x}'.format(destination)
    text += bus
  else:
    # Compose string
    if isStore:
      if bus == 'ac':
        text = '{:4} {!s}'.format(text, _ea)
      else:
        text = '{:4} {!s},{!s}'.format(text, bus, _ea)
      if bus is None:                  # Write/read combination means I/O control
         text = 'ctrl {!s}'.format(_ea[1:-1])  # Strip the brackets
      if reg != 'ac' and reg != 'out': # X and Y are not muted
        text += ',' + reg
    else:
      if reg == 'ac':
        text = '{:4} {!s}'.format(text, bus)
      else:
        text = '{:4} {!s},{!s}'.format(text, bus, reg)
      # Specials
      if opcode == _opLD | _busAC: text = 'nop'

  # Emit as text
  return text

def _emit(opcode, operand):
  global _romSize, _maxRomSize, _errors
  if _romSize >= _maxRomSize:
      disassembly = disassemble(opcode, operand)
      print('{:04x} {:02x:02x}  {!s}'.format(_romSize, opcode, operand, disassembly))
      print('Error: Program size limit exceeded')
      _errors += 1
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
    opcode & _maskBus == _busRAM and\
    opcode & _maskCc in [ _jGT, _jLT, _jNE, _jEQ, _jGE, _jLE ]:
    disassembly = disassemble(opcode, operand)
    print('{:04x} {:02x:02x}  {!s}'.format(_romSize, opcode, operand, disassembly))
    print('Warning: large propagation delay (conditional branch with RAM on bus)')

def loadBindings(symfile):
  # Load JSON file into symbol table
  global _symbols
  with open(symfile) as file:
    for (name, value) in json.load(file).items():
      if not isinstance(value, int):
        value = int(value, base=0)
      _symbols[str(name)] = value

def getRom1():
  return ''.join(chr(byte) for byte in _rom1)

def writeRomFiles(sourceFile):

  # Determine stem for file names
  stem, _ = splitext(sourceFile)
  stem = basename(stem)
  if stem == '': stem = 'out'

  # Disassemble for readability
  filename = stem + '.asm'
  print('Create file {!s}'.format(filename))
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

    lastOpcode = None
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
        disassembly = disassemble(opcode, operand, address, lastOpcode)
        if comment:
          line = '{:13} {:04x} {:02x}{:02x}  {:16} ;{!s}\n'.format(label, address, opcode, operand, disassembly, comment)
        else:
          line = '{:13} {:04x} {:02x}{:02x}  {!s}\n'.format(label, address, opcode, operand, disassembly)

      if repeats < maxRepeat:
        file.write(line) # always write first N
        if comment:
          for extra in _comments[address][1:]:
            file.write(42*' ' + ';{!s}\n'.format(extra))
      if repeats == maxRepeat:
        postponed = line # if this turns out to  be the last repeat, emit the line
      if repeats > maxRepeat: # now it makes sense to abbreviate the output
        postponed = 14*' '+'* {:d} times\n'.format(1+repeats)

      address += 1
      lastOpcode = opcode

    if postponed:
      file.write(postponed)
    file.write(14*' '+'{:04x}\n'.format(address))
    assert len(_rom0) == _romSize
    assert len(_rom1) == _romSize

# # Write ROM files
# filename = stem + '.lo.rom'
# print 'Create file', filename
# with open(filename, 'wb') as file:
#   file.write(''.join([chr(byte) for byte in _rom0]))

# filename = stem + '.hi.rom'
# print 'Create file', filename
# with open(filename, 'wb') as file:
#   file.write(''.join([chr(byte) for byte in _rom1]))

  # 16-bit version for 27C1024, little endian
  filename = stem + '.rom'
  print('Create file {!s}'.format(filename))
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

  print('OK used {:d} free {:d} size {:d}'.format(_romSize, _maxRomSize-_romSize, len(_rom2)))

