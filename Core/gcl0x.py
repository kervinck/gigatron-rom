#!/usr/bin/env python
from __future__ import print_function

from asm import *
import string
import sys

# XXX Change to Python3
# XXX Give warning when starting new block after calls were made
# XXX Give warning when def-block contains 'call' put no 'push'
# XXX Give warning when def-block contains code but no 'ret'
# XXX Give warning when a variable is not both written and read 

def has(x):
  return x is not None

class Program:
  def __init__(self, address, name, forRom=True):
    self.name = name
    self.forRom = forRom # Inject trampolines
    self.comment = 0     # Nesting level
    self.lineNumber, self.filename = 0, None
    self.blocks, self.blockId = [0], 1
    self.loops = {} # block -> address of last do
    self.conds = {} # block -> address of continuation
    self.defs  = {} # block -> address of last def
    self.vars  = {} # name -> address (GCL variables)
    self.segStart = None
    self.vPC = None
    self.segId = 0
    self.org(address)
    self.version = None # Must be first word 'gcl<N>'
    self.execute = None
    self.needPatch = False
    self.lengths = {} # block -> length, or var -> length

  def org(self, address):
    """Set a new address"""
    self.closeSegment()
    self.segStart = address
    self.vPC = address
    page = address & ~255
    self.segEnd = page + (250 if 0x100 <= page <= 0x400 else 256)

  def openSegment(self):
    """Write header for segment"""
    address = self.segStart
    if not has(self.execute):
      self.execute = address
    assert self.segId == 0 or address>>8 != 0 # Zero-page segment can only be first
    self.putInRomTable(address>>8, '| RAM segment address (high byte first)')
    self.putInRomTable(address&255, '|')
    # Fill in the length through the symbol table
    self.putInRomTable(lo('$%s.seg.%d' % (self.name, self.segId)), '| Length (1..256)')

  def closeSegment(self):
    """Register length of segment"""
    if len(self.blocks) > 1:
      self.error('Unterminated block')
    if self.vPC != self.segStart:
      print(' Segment at %04x size %3d used %3d unused %3d' % (
        self.segStart,
        self.segEnd - self.segStart,
        self.vPC - self.segStart,
        self.segEnd - self.vPC))
      length = self.vPC - self.segStart
      assert 1 <= length <= 256
      define('$%s.seg.%d' % (self.name, self.segId), length)
      self.segId += 1

  def thisBlock(self):
    return self.blocks[-1]

  def line(self, line):
    """Process a line by tokenizing and processing the words"""

    self.lineNumber += 1
    nextWord = ''

    for nextChar in line:
      if self.comment > 0:
        # Inside comments anything goes
        if nextChar == '{': self.comment += 1
        if nextChar == '}': self.comment -= 1
      elif nextChar not in '{}[]':
        if nextChar.isspace():
          self.word(nextWord)
          nextWord = ''
        else:
          nextWord += nextChar
      else:
        self.word(nextWord)
        nextWord = ''
        if nextChar == '{': self.comment += 1
        elif nextChar == '}': self.error('Spurious %s' % repr(nextChar))
        elif nextChar == '[': self.blocks.append(self.blockId); self.blockId +=  1
        elif nextChar == ']':
          if len(self.blocks) <= 1:
            self.error('Unexpected %s' % repr(nextChar))
          block = self.blocks.pop()
          if block in self.conds:
            # There was an if-statement in this block
            # Define the label to jump here
            # XXX why not always make a label?
            define('$%s.if.%d.%d' % (self.name, block, self.conds[block]), prev(self.vPC))
            del self.conds[block]
          if block in self.defs:
            define('$%s.def.%d' % (self.name, self.defs[block]), prev(self.vPC))
            self.lengths[self.thisBlock()] = self.vPC - self.defs[block] + 2
            del self.defs[block]
        elif nextChar == '(': pass
        elif nextChar == ')': pass
    self.word(nextWord)

  def getAddress(self, var):
    if isinstance(var, str):
      if var not in self.vars:
        self.vars[var] = zpByte(2)
      return self.vars[var]
    else:
      if var<0 or var>255:
        self.error('Index out of range %s' % repr(var))
      return var

  def emit(self, byte, comment=None):
    """Next program byte in RAM"""
    if self.vPC >= self.segEnd:
      self.error('Out of code space')
    if byte < 0 or byte >= 256:
      self.error('Value out of range %d (must be 0..255)' % byte)
    if self.segStart == self.vPC:
      self.openSegment()
    self.putInRomTable(byte, comment)
    self.vPC += 1

  def opcode(self, ins):
    """Next opcode in RAM"""
    if self.vPC >= self.segEnd:
      self.error('Out of code space')
    if self.segStart == self.vPC:
      self.openSegment()
    self.putInRomTable(lo(ins), '%04x %s' % (self.vPC, ins))
    self.vPC += 1

  def address(self, var, offset=0):
    comment = '%04x %s' % (prev(self.vPC, 1), repr(var))
    comment += '%+d' % offset if offset else ''
    self.emit(self.getAddress(var)+offset, comment)

  def word(self, word):
    """Process a word and emit its code"""
    if len(word) == 0:
      return

    # Bare keywords
    if not has(self.version):
       # XXX For ideas on language changes, see Docs/GCL-language.txt
      if word in ['gcl0x']:
        self.version = word
      else:
        self.error('Invalid GCL version %s' % repr(word))
    elif word == 'def':
      pc = self.vPC # Just an identifier
      self.opcode('DEF')
      self.defs[self.thisBlock()] = pc
      self.emit(lo('$%s.def.%d' % (self.name, pc)))
    elif word == 'do':
      self.loops[self.thisBlock()] = self.vPC
    elif word == 'loop':
      to = [block for block in self.blocks if block in self.loops]
      if len(to) == 0:
        self.error('Loop without do')
      to = self.loops[to[-1]]
      to = prev(to)
      if self.vPC>>8 != to>>8:
        self.error('Loop outside page')
      self.opcode('BRA')
      self.emit(to&255)
    elif word == 'if<>0':     self._emitIf('EQ')
    elif word == 'if=0':      self._emitIf('NE')
    elif word == 'if>=0':     self._emitIf('LT')
    elif word == 'if<=0':     self._emitIf('GT')
    elif word == 'if>0':      self._emitIf('LE')
    elif word == 'if<0':      self._emitIf('GE')
    elif word == 'if<>0loop': self._emitIfLoop('NE')
    elif word == 'if=0loop':  self._emitIfLoop('EQ')
    elif word == 'if>0loop':  self._emitIfLoop('GT')
    elif word == 'if<0loop':  self._emitIfLoop('LT')
    elif word == 'if>=0loop': self._emitIfLoop('GE')
    elif word == 'if<=0loop': self._emitIfLoop('LE')
    elif word == 'else':
      block = self.thisBlock()
      if block not in self.conds:
        self.error('Unexpected %s' % repr(word))
      if self.conds[block] > 0:
        self.error('Too many %s' % repr(word))
      self.opcode('BRA')
      self.emit(lo('$%s.if.%d.1' % (self.name, block)))
      define('$%s.if.%d.0' % (self.name, block), prev(self.vPC))
      self.conds[block] = 1
    elif word == 'call':
      self.opcode('CALL')
      self.emit(symbol('vAC'), '%04x vAC' % prev(self.vPC, 1))
    elif word == 'push': self.opcode('PUSH')
    elif word == 'pop':  self.opcode('POP')
    elif word == 'ret':
      self.opcode('RET')
      if len(self.blocks) == 1:
        self.needPatch = True # Top-level use of 'ret' --> apply patch
    elif word == 'peek': self.opcode('PEEK')
    elif word == 'deek': self.opcode('DEEK')
    else:
      # Operand words
      var, con, op = self.parseWord(word) # XXX Simplify this
      if not has(op):
        if var:
          self.opcode('LDW')
          self.address(var)
        else:
          if not has(con):
            self.error('(%s) Invalid word' % word)
          if 0 <= con < 256:
            self.opcode('LDI')
            self.emit(con)
          else:
            self.opcode('LDWI')
            self.emit( con     & 255)
            self.emit((con>>8) & 255)

      # Constant words
      elif has(con):
        if op == '<<':
          for i in range(con):
            self.opcode('LSLW')
          con = None
        elif op == ':' and con > 255: self.org(con); con = None
        elif op == '=':    self.opcode('STW'); self.depr(word, 'i=', 'i:')
        elif op == ':':    self.opcode('STW')
        elif op == ';':    self.opcode('LDW')
        elif op == '.':    self.opcode('ST')
        elif op == ',':    self.opcode('LD')
        elif op == '&':    self.opcode('ANDI')
        elif op == '|':    self.opcode('ORI')
        elif op == '^':    self.opcode('XORI')
        elif op == '+':    self.opcode('ADDI')
        elif op == '-':    self.opcode('SUBI')
        elif op == '%=':   self.opcode('STLW')
        elif op == '%':    self.opcode('LDLW')
        elif op == '--':   self.opcode('ALLOC'); con = -con & 255
        elif op == '++':   self.opcode('ALLOC')
        elif op == '#':    con &= 255                      #self.depr(word, 'i#', '#i')
        elif op == '# ':   con &= 255
        elif op == '<++':  self.opcode('INC');             #self.depr(word, 'i<++', '<i++')
        elif op == '>++':  self.opcode('INC'); con += 1;   #self.depr(word, 'i>++', '>i++')
        elif op == '< ++': self.opcode('INC')
        elif op == '> ++': self.opcode('INC'); con += 1
        elif op == '!':    self.opcode('SYS'); con = self.sysOperand(word, con)
        elif op == '?':    self.opcode('LUP')
        else:
          self.error('(%s) Invalid word' % word)
        if has(con):
          self.emit(con)

      # Variable words
      elif has(var):
        offset = 0
        if op == '`': # Inline ASCII
          if len(var) > 0:
            for c in var:
              self.emit(ord(' ' if c == '`' else c))
          else:
            self.emit(ord('`'))
          var = None
        elif op == '=':    self.opcode('STW'); self.defInfo(var)
        elif op == ',':    self.opcode('LDW'); self.address(var); self.opcode('PEEK'); var = None
        elif op == ';':    self.opcode('LDW'); self.address(var); self.opcode('DEEK'); var = None
        elif op == '&':    self.opcode('ANDW')
        elif op == '|':    self.opcode('ORW')
        elif op == '^':    self.opcode('XORW')
        elif op == '+':    self.opcode('ADDW')
        elif op == '-':    self.opcode('SUBW')
        elif op == '.':    self.opcode('POKE')
        elif op == ':':    self.opcode('DOKE')
        elif op == '<++':  self.opcode('INC');             #self.depr(word, 'X<++', '<X++')
        elif op == '>++':  self.opcode('INC'); offset = 1; #self.depr(word, 'X>++', '>X++')
        elif op == '< ++': self.opcode('INC')
        elif op == '> ++': self.opcode('INC'); offset = 1
        elif op == '<,':   self.opcode('LD');              #self.depr(word, 'X<,', '>X,')
        elif op == '>,':   self.opcode('LD'); offset = 1;  #self.depr(word, 'X>,', '>X,')
        elif op == '<.':   self.opcode('ST');              #self.depr(word, 'X<.', '<X.')
        elif op == '>.':   self.opcode('ST'); offset = 1;  #self.depr(word, 'X>.', '>X.')
        elif op == '< ,':  self.opcode('LD')
        elif op == '> ,':  self.opcode('LD'); offset = 1
        elif op == '< .':  self.opcode('ST')
        elif op == '> .':  self.opcode('ST'); offset = 1
        elif op == '!':    self.opcode('CALL')
        else:
          self.error('(%s) Invalid word' % word)
        if has(var):
          self.address(var, offset)

      else:
        self.error('(%s) Invalid word' % word)

  def defInfo(self, var):
    # Heuristic to track def lengths
    if var not in self.lengths and self.thisBlock() in self.lengths:
      self.lengths[var] = self.lengths[self.thisBlock()]
    else:
      self.lengths[var] = None # No def lengths can be associated

  def sysOperand(self, word, con):
    if con & 1:
      self.error('(%s) Invalid value (must be even)' % word)
    extraTicks = con/2 - symbol('maxTicks')
    return 256 - extraTicks if extraTicks > 0 else 0

  def _emitIf(self, cond):
      self.opcode('BCC')
      self.opcode(cond)
      block = self.thisBlock()
      self.emit(lo('$%s.if.%d.0' % (self.name, block)))
      self.conds[block] = 0

  def _emitIfLoop(self, cond):
      block = self.thisBlock()
      to = [block for block in self.blocks if block in self.loops]
      if len(to) == 0:
        self.error('Loop without do')
      to = self.loops[to[-1]]
      to = prev(to)
      if self.vPC>>8 != to>>8:
        self.error('Loop outside page')
      self.opcode('BCC')
      self.opcode(cond)
      self.emit(to&255)

  def parseWord(self, word):
    """Break word into pieces"""

    word += '\0' # Avoid checking len() everywhere
    sign = None
    name, number, op = None, None, ''

    if word[0] == '`':
      # Quoted word
      name, op = word[1:-1], word[0]
      return name, number, op

    ix = 0
    if word[ix] in ['%', '#', '<', '>']:
      # Prefix operators
      op += word[ix] + ' ' # We use space to marks prefix operators
      ix += 1

    if word[ix] in ['-', '+']:
      # Number sign
      sign = word[ix]
      ix += 1

    if word[ix] == '$' and word[ix+1] in string.hexdigits:
      # Hexadecimal number
      jx = ix+1
      number = 0
      while word[jx] in string.hexdigits:
        o = string.hexdigits.index(word[jx])
        number = 16*number + (o if o<16 else o-6)
        jx += 1
      ix = jx if jx-ix > 1 else 0
    elif word[ix].isdigit():
      # Decimal number
      number = 0
      while word[ix].isdigit():
        number = 10*number + ord(word[ix]) - ord('0')
        ix += 1
    else:
      name = ''
      while word[ix].isalnum() or word[ix] in ['\\', '_']:
        name += word[ix]
        ix += 1
      name = name if len(name)>0 else None

      # Resolve '\symbol' as the number it represents
      if has(name) and name[0] == '\\':
        # Peeking into the assembler's symbol table (not GCL's)
        # Substitute \symbol with its value and keep the postfix operator
        number = symbol(name[1:])
        if not has(number):
          self.error('(%s) Undefined symbol %s' % (word, name))
        name, op = None, ''

    if has(number):
      if sign == '-': number = -number

    op += word[ix:-1]                   # Also strips sentinel '\0'
    return (name, number, op if len(op)>0 else None)

  def end(self):
    if self.comment > 0:
      self.error('Unterminated comment')
    self.closeSegment()
    print(' Variables count %d bytes %d end %04x' % (len(self.vars), 2*len(self.vars), zpByte(0)))
    line = ' :'
    for var in sorted(self.vars.keys()):
      if var in self.lengths and self.lengths[var]:
        var += ' [%s]' % self.lengths[var]
      if len(line + var) + 1 > 72:
        print(line)
        line = ' :'
      line += ' ' + var
    print(line)
    self.putInRomTable(0) # Zero marks the end of stream
    C('End of file')

  def prefix(self, prefix):
    prefix += (' file %s' % repr(self.filename)) if self.filename else ''
    prefix += ' line %s:' % self.lineNumber
    return prefix

  def warning(self, message):
    print(self.prefix('GCL warning'), message)

  def error(self, message):
    print(self.prefix('GCL error'), message)
    sys.exit()

  def depr(self, word, old, new):
    self.warning('(%s) %s is depricated, use %s' % (word, old, new))

  def putInRomTable(self, byte, comment=None):
    ld(byte)
    if comment:
      C(comment)
    if self.forRom and pc()&255 == 251:
      trampoline()

def prev(address, step=2):
  """Take vPC two bytes back, wrap around if needed to stay on page"""
  return (address & 0xff00) | ((address-step) & 0x00ff)
