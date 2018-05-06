#!/usr/bin/env python

from asm import *
import sys

# XXX Give warning for redefining a label
# XXX Make dot-labels local ('.loop:')
# XXX Make inline vCPU assembly possible
# XXX Notation for DOKE/DEEK
# XXX Give a warning when starting new block after calls were made
# XXX Give warning when def-block contains 'call' put no 'push'
# XXX Give warning when def-block contains code but no 'ret'
# XXX Primitive or macro to clear just lower byte of vAC
# XXX Give warning when not all comments are closed
# XXX Give warning when a variable is not both written and read 
# XXX Macros
# XXX 'page' macro

class Program:
  def __init__(self, address, name, forRom=True):
    self.name = name
    self.forRom = forRom
    self.comment = 0
    self.lineNumber, self.filename = 0, None
    self.blocks, self.block = [0], 1
    self.loops = {} # block -> address of last do
    self.conds = {} # block -> address of continuation
    self.defs  = {} # block -> address of last def
    self.vars  = {} # name -> address
    self.vPC = None
    self.segId = 0
    self.org(address)
    self.version = None # Must be first word 'gcl<N>'

  def segInfo(self):
    print ' Segment at %04x size %3d used %3d unused %3d' % (
      self.segStart,
      self.segEnd - self.segStart,
      self.vPC - self.segStart,
      self.segEnd - self.vPC)
    define('$%s.seg.%d' % (self.name, self.segId), self.vPC - self.segStart)
    self.segId += 1

  def org(self, address):
    # Configure start address for emit
    if self.vPC is not None and self.segStart < self.vPC:
      self.segInfo()
    if address != self.vPC or self.segStart < self.vPC:
      assert(address>>8) # Because a zero would mark the end of stream
      self.putInRomTable(address>>8, '| RAM segment address (high byte first)')
      self.putInRomTable(address&0xff, '|')
      self.putInRomTable(lo('$%s.seg.%d' % (self.name, self.segId)), '| Length (1..256)')
    self.segStart = address
    page = address & ~255
    self.segEnd = page + (250 if page <= 0x400 else 256)
    self.vPC = self.segStart

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
      elif nextChar not in '{}[]()':
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
        elif nextChar == '[': self.blocks.append(self.block); self.block +=  1
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
    self.putInRomTable(byte, comment)
    self.vPC += 1

  def opcode(self, ins):
    """Next opcode in RAM"""
    if self.vPC >= self.segEnd:
        self.error('Out of code space')
    self.putInRomTable(lo(ins), '%04x %s' % (self.vPC, ins))
    self.vPC += 1

  def word(self, word):
    """Process a word and emit its code"""
    if len(word) == 0:
      return

    if self.version is None:
      if word in ['gcl0x']:
        self.version = word
      else:
        self.error('Invalid GCL version %s' % repr(word))
    elif word == 'loop':
      to = [block for block in self.blocks if block in self.loops]
      to = self.loops[to[-1]]
      to = prev(to)
      if self.vPC>>8 != to>>8:
        self.error('Loop outside page')
      self.opcode('BRA')
      self.emit(to&255)
    elif word == 'def':
      pc = self.vPC # Just an identifier
      self.opcode('DEF')
      self.defs[self.thisBlock()] = pc
      self.emit(lo('$%s.def.%d' % (self.name, pc)))
    elif word == 'do':
      self.loops[self.thisBlock()] = self.vPC
    elif word == 'if<>0': self._emitIf('EQ')
    elif word == 'if=0':  self._emitIf('NE')
    elif word == 'if>=0': self._emitIf('LT')
    elif word == 'if<=0': self._emitIf('GT')
    elif word == 'if>0':  self._emitIf('LE')
    elif word == 'if<0':  self._emitIf('GE')
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

    elif word == 'push': self.opcode('PUSH')
    elif word == 'pop':  self.opcode('POP')
    elif word == 'ret':  self.opcode('RET')
    elif word == 'call':
          self.opcode('CALL')
          self.emit(symbol('vAC'), '%04x vAC' % prev(self.vPC, 1))
    elif word == 'peek': self.opcode('PEEK')
    else:
      var, con, op = self.parseWord(word) # XXX Simplify this
      if op is None:
        if var:
          self.opcode('LDW')
          self.emit(self.getAddress(var), '%04x %s' % (prev(self.vPC, 1), repr(var)))
        else:
          if 0 <= con < 256:
            self.opcode('LDI')
            self.emit(con)
          else:
            self.opcode('LDWI')
            self.emit( con    &0xff)
            self.emit((con>>8)&0xff)
      elif op == ';' and con is not None:
          self.opcode('LDW')
          self.emit(con)
      elif op == ':' and con is not None: # XXX Replace with automatic allocation ('page')
          self.org(con)
      elif op == '=' and var:
          self.opcode('STW')
          self.emit(self.getAddress(var), '%04x %s' % (prev(self.vPC, 1), repr(var)))
      elif op == '=' and con is not None:
          self.opcode('STW')
          self.emit(con)
      elif op == '+' and var:
          self.opcode('ADDW')
          self.emit(self.getAddress(var), '%04x %s' % (prev(self.vPC, 1), repr(var)))
      elif op == '+' and con is not None:
          self.opcode('ADDI')
          self.emit(con)
      elif op == '-' and var:
          self.opcode('SUBW')
          self.emit(self.getAddress(var), '%04x %s' % (prev(self.vPC, 1), repr(var)))
      elif op == '-' and con is not None:
          self.opcode('SUBI')
          self.emit(con)
      elif op == '<<' and con is not None:
          for i in range(con):
            self.opcode('LSLW')
      elif op == '--' and con is not None:
          self.opcode('ALLOC')
          self.emit(-con&255)
      elif op == '++' and con is not None:
          self.opcode('ALLOC')
          self.emit(con)
      elif op == '%=' and con is not None:
          self.opcode('STLW')
          self.emit(con)
      elif op == '%' and con is not None:
          self.opcode('LDLW')
          self.emit(con)
      elif op == '#' and con is not None:
          self.emit(con & 255)
      elif op == '?' and con is not None:
          self.opcode('LUP')
          self.emit(con)
      elif op == '&' and var:
          self.opcode('ANDW')
          self.emit(self.getAddress(var), '%04x %s' % (prev(self.vPC, 1), repr(var)))
      elif op == '&' and con is not None:
          self.opcode('ANDI')
          self.emit(con)
      elif op == '|' and var:
          self.opcode('ORW')
          self.emit(self.getAddress(var), '%04x %s' % (prev(self.vPC, 1), repr(var)))
      elif op == '|' and con is not None:
          self.opcode('ORI')
          self.emit(con)
      elif op == '^' and var:
          self.opcode('XORW')
          self.emit(self.getAddress(var), '%04x %s' % (prev(self.vPC, 1), repr(var)))
      elif op == '^' and con is not None:
          self.opcode('XORI')
          self.emit(con)
      elif op == '.' and con is not None:
          self.opcode('ST')
          self.emit(con)
      elif op == ',' and con is not None:
          self.opcode('LD')
          self.emit(con)
      elif op == '.' and var:
          self.opcode('POKE')
          self.emit(self.getAddress(var), '%04x %s' % (prev(self.vPC, 1), repr(var)))
      elif op == '<.' and var:
          self.opcode('ST')
          self.emit(self.getAddress(var), '%04x %s' % (prev(self.vPC, 1), repr(var)))
      elif op == '>.' and var:
          self.opcode('ST')
          self.emit(self.getAddress(var)+1, '%04x %s+1' % (prev(self.vPC, 1), repr(var)))
      elif op == ',' and var:
          self.opcode('LDW')
          self.emit(self.getAddress(var), '%04x %s' % (prev(self.vPC, 1), repr(var)))
          self.opcode('PEEK')
      elif op == '<++' and var:
          self.opcode('INC')
          self.emit(self.getAddress(var), '%04x %s' % (prev(self.vPC, 1), repr(var)))
      elif op == '<++' and con is not None:
          self.opcode('INC')
          self.emit(con)
      elif op == '>++' and var:
          self.opcode('INC')
          self.emit(self.getAddress(var)+1, '%04x %s+1' % (prev(self.vPC, 1), repr(var)))
      elif op == '>++' and con is not None:
          self.opcode('INC')
          self.emit(con+1)
      elif op == '<,' and var:
          self.opcode('LD')
          self.emit(self.getAddress(var), '%04x %s' % (prev(self.vPC, 1), repr(var)))
      elif op == '>,' and var:
          self.opcode('LD')
          self.emit(self.getAddress(var)+1, '%04x %s+1' % (prev(self.vPC, 1), repr(var)))
      elif op == '!' and var:
          self.opcode('CALL')
          self.emit(self.getAddress(var), '%04x %s' % (prev(self.vPC, 1), repr(var)))
      elif op == '!' and con is not None:
          if con&1:
            self.error('Invalid value %s (must be even)' % repr(con))
          minSYS, maxSYS = symbol('$minSYS'), symbol('$maxSYS')
          if con > maxSYS:
            self.warning('Large cycle count %s > %s (will never run)' % (repr(con), repr(maxSYS)))
          self.opcode('SYS')
          extraTicks = con/2 - symbol('$maxTicks')
          self.emit(256 - extraTicks if extraTicks > 0 else 0)
      else:
        self.error('Invalid word %s' % repr(word))

  def _emitIf(self, cond):
      self.opcode('BCC')
      self.opcode(cond)
      block = self.thisBlock()
      self.emit(lo('$%s.if.%d.0' % (self.name, block)))
      self.conds[block] = 0

  def _emitIfLoop(self, cond):
      self.opcode('BCC')
      self.opcode(cond)
      block = self.thisBlock()
      to = [block for block in self.blocks if block in self.loops]
      to = self.loops[to[-1]]
      to = prev(to)
      self.emit(to&0xff)
      if self.vPC>>8 != to>>8:
        self.error('Loop outside page')

  def parseWord(self, word):
    """Break word into pieces"""

    word += '\0' # Avoid checking len() everywhere
    unnamed, sign = None, None
    name, number = None, 0

    ix = 0
    if word[ix] == '%':
      # Unnamed variable
      unnamed = word[ix]
      ix += 1

    if word[ix] in '-+':
      # Number sign
      sign = word[ix]
      ix += 1

    if word[ix] == '$':
      # Hexadecimal number?
      jx = ix+1
      while word[jx]:
        o = ord(word[jx])
        if word[jx] in '0123456789': number = 16*number + o - ord('0')
        elif word[jx] in 'abcdef': number = 16*number + 10 + o - ord('a')
        elif word[jx] in 'ABCDEF': number = 16*number + 10 + o - ord('A')
        else: break
        jx += 1
      ix = jx if jx-ix > 1 else 0
    elif word[ix].isdigit():
      # Decimal number
      while word[ix].isdigit():
        number = 10*number + ord(word[ix]) - ord('0')
        ix += 1
    elif word[ix] == '\\':
        sym = ''
        ix += 1
        while word[ix].isalnum() or word[ix] == '_':
          sym += word[ix]
          ix += 1
        number = symbol(sym)
        if number is None:
          self.error('Undefined symbol %s' % repr(sym))
    else:
      # Named variable?
      number = None
      if unnamed or sign:
         ix = 0 # Reset
      else:
        name = ''
        while word[ix].isalnum() or word[ix] == '_':
          name += word[ix]
          ix += 1

        name = name if len(name)>0 else None

    if number is not None:
      if sign == '-': number = -number
      if unnamed: name, number = number, None

    op = word[ix:-1]
    return (name, number, op if len(op)>0 else None)

  def end(self):
    self.segInfo()
    # XXX Check all blocks are closed
    if len(self.conds) > 0:
      self.error('Dangling if statements')
    print ' Variables count %d bytes %d end %04x' % (len(self.vars), 2*len(self.vars), zpByte(0))
    symbols, n = sorted(self.vars.keys()), 8
    for i in range(0, len(symbols), n):
      print ' Symbols ' + ' '.join(symbols[i:i+n])
    self.putInRomTable(0) # Zero marks the end of stream
    C('End of file')

  def warning(self, message):
    prefix = 'GCL warning:'
    prefix += (' file %s' % repr(self.filename)) if self.filename else ''
    prefix += ' line %s:' % self.lineNumber
    print prefix, message

  def error(self, message):
    prefix = 'GCL error:'
    prefix += (' file %s' % repr(self.filename)) if self.filename else ''
    prefix += ' line %s:' % self.lineNumber
    print prefix, message
    sys.exit()

  def putInRomTable(self, byte, comment=None):
    ld(val(byte))
    if comment:
      C(comment)
    if self.forRom and pc()&255 == 251:
      trampoline()

def prev(address, step=2):
  """Take vPC two bytes back, wrap around if needed to stay on page"""
  return (address & 0xff00) | ((address-step) & 0x00ff)
