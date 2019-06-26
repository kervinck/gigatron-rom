#!/usr/bin/env python
from __future__ import print_function

# XXX Change to Python3
# XXX Backquoted words should have precedence over grouping
# XXX Give warning when starting new block after calls were made
# XXX Give warning when def-block contains 'call' put no 'push'
# XXX Give warning when def-block contains code but no 'ret'
# XXX Give warning when a variable is not both written and read 

from asm import *
import string
import sys

class Program:
  def __init__(self, name, forRom=True):
    self.name = name     # For defining unique labels in global symbol table
    self.forRom = forRom # Inject trampolines if compiling for ROM XXX why not do that outside?
    self.comment = 0     # Nesting level
    self.lineNumber = 0
    self.filename = None
    self.openBlocks = [0] # Outside first block is 0
    self.nextBlockId = 1
    self.start = {} # blockId -> address of block start
    self.loops = {} # blockId -> address after `do'
    self.elses = {} # blockId -> count of `else'
    self.defs  = {} # blockId -> address of last `def'
    self.vars  = {} # name -> address (GCL variables)
    self.segStart = None
    self.vPC = None
    self.segId = 0
    self.version = None # Must be first word 'gcl<N>'
    self.execute = None
    self.needPatch = False
    self.lengths = {} # block -> length, or var -> length

  def org(self, address):
    """Set start address"""
    self.closeSegment()
    self.segStart = address
    self.vPC = address
    page = address & ~255
    self.segEnd = page + (250 if 0x100 <= page <= 0x400 else 256)

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
        elif nextChar == '[':
           self.openBlocks.append(self.nextBlockId)
           self.elses[self.nextBlockId] = 0
           self.start[self.nextBlockId] = self.vPC
           self.nextBlockId += 1
        elif nextChar == ']':
          if len(self.openBlocks) <= 1:
            self.error('Block close without open')
          b = self.openBlocks.pop()
          if self.start[b]>>8 != (self.vPC-1)>>8:
            self.error('Block crosses page boundary')
          define('__%s_%d_cond%d__' % (self.name, b, self.elses[b]), prev(self.vPC))
          del self.elses[b]
          if b in self.defs:
            self.lengths[self.thisBlock()] = self.vPC - self.defs[b] + 2
            define('__%s_%#04x_def__' % (self.name, self.defs[b]), prev(self.vPC))
            del self.defs[b]
        elif nextChar == '(': pass
        elif nextChar == ')': pass
    self.word(nextWord)

  def end(self):
    """Signal end of program"""
    if self.comment > 0:
      self.error('Unterminated comment')
    self.closeSegment()
    if len(self.openBlocks) > 1:
      self.error('Unterminated block')
    self.putInRomTable(0) # Zero marks the end of stream
    if self.lineNumber > 0:
      self.dumpVars()
    C('End of file')

  def dumpVars(self):
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

  def word(self, word):
    # Process a GCL word and emit its corresponding code
    if len(word) == 0:
      return
    self.lastWord = word

    # Simple keywords
    if not has(self.version):
      if word in ['gcl0x']:
        self.version = word
      else:
        self.error('(%s) Invalid GCL version' % word)
    elif word == 'def':       self.emitDef()
    elif word == 'do':        self.loops[self.thisBlock()] = self.vPC
    elif word == 'loop':      self.emitLoop()
    elif word == 'if<>0':     self.emitIf('EQ')
    elif word == 'if=0':      self.emitIf('NE')
    elif word == 'if>=0':     self.emitIf('LT')
    elif word == 'if<=0':     self.emitIf('GT')
    elif word == 'if>0':      self.emitIf('LE')
    elif word == 'if<0':      self.emitIf('GE')
    elif word == 'if<>0loop': self.emitIfLoop('NE')
    elif word == 'if=0loop':  self.emitIfLoop('EQ')
    elif word == 'if>0loop':  self.emitIfLoop('GT')
    elif word == 'if<0loop':  self.emitIfLoop('LT')
    elif word == 'if>=0loop': self.emitIfLoop('GE')
    elif word == 'if<=0loop': self.emitIfLoop('LE')
    elif word == 'else':      self.emitElse()
    elif word == 'call':      self.emitOp('CALL'); self.emit(symbol('vAC'), '%04x vAC' % prev(self.vPC, 1))
    elif word == 'push':      self.emitOp('PUSH')
    elif word == 'pop':       self.emitOp('POP')
    elif word == 'ret':       self.emitOp('RET'); self.needPatch = self.needPatch or len(self.openBlocks) == 1 # Top-level use of 'ret' --> apply patch
    elif word == 'peek':      self.emitOp('PEEK')
    elif word == 'deek':      self.emitOp('DEEK')
    else:
      var, con, op = self.parseWord(word)

      # Words with constant value as operand
      if has(con):
        if not has(op):
          if 0 <= con < 256:
            self.emitOp('LDI')
          else:
            self.emitOp('LDWI'); self.emit(lo(con)); con = hi(con)
        elif op == '*= ':  self.org(con); con = None
        elif op == ';':    self.emitOp('LDW')
        elif op == '=':    self.emitOp('STW'); self.depr('i=', 'i:')
        elif op == ':' and con < 256: self.emitOp('STW')
        elif op == ',':    self.emitOp('LD')
        elif op == '.':    self.emitOp('ST')
        elif op == '&':    self.emitOp('ANDI')
        elif op == '|':    self.emitOp('ORI')
        elif op == '^':    self.emitOp('XORI')
        elif op == '+':    self.emitOp('ADDI')
        elif op == '-':    self.emitOp('SUBI')
        elif op == '% =':  self.emitOp('STLW')
        elif op == '% ':   self.emitOp('LDLW')
        elif op == '--':   self.emitOp('ALLOC'); con = -con & 255
        elif op == '++':   self.emitOp('ALLOC')
        elif op == '< ++': self.emitOp('INC')
        elif op == '> ++': self.emitOp('INC'); con += 1
        elif op == '!':    self.emitOp('SYS'); con = self.sysTicks(con)
        elif op == '?':    self.emitOp('LUP')
        elif op == '# ':   con &= 255
        elif op == '<<':
          for i in range(con):
            self.emitOp('LSLW')
          con = None
        # Depricated syntax
        elif op == ':':    self.org(con); con = None;   #self.depr('ii:', '*=ii')
        elif op == '#':    con &= 255;                  #self.depr('i#', '#i')
        elif op == '<++':  self.emitOp('INC');          #self.depr('i<++', '<i++')
        elif op == '>++':  self.emitOp('INC'); con += 1 #self.depr('i>++', '>i++')
        elif op == '%=':   self.emitOp('STLW');         #self.depr('i%=', '%i=')
        elif op == '%':    self.emitOp('LDLW');         #self.depr('i%', %i')
        else:
          self.error("Invalid operator '%s' with constant" % op)
        if has(con):
          self.emit(con)

      # Words with variable name as operand
      elif has(var):
        offset = 0
        if not has(op):    self.emitOp('LDW')
        elif op == '=':    self.emitOp('STW'); self.defInfo(var)
        elif op == ',':    self.emitOp('LDW'); self.emitVar(var); self.emitOp('PEEK'); var = None
        elif op == ';':    self.emitOp('LDW'); self.emitVar(var); self.emitOp('DEEK'); var = None
        elif op == '.':    self.emitOp('POKE')
        elif op == ':':    self.emitOp('DOKE')
        elif op == '< ,':  self.emitOp('LD')
        elif op == '> ,':  self.emitOp('LD'); offset = 1
        elif op == '< .':  self.emitOp('ST')
        elif op == '> .':  self.emitOp('ST'); offset = 1
        elif op == '&':    self.emitOp('ANDW')
        elif op == '|':    self.emitOp('ORW')
        elif op == '^':    self.emitOp('XORW')
        elif op == '+':    self.emitOp('ADDW')
        elif op == '-':    self.emitOp('SUBW')
        elif op == '< ++': self.emitOp('INC')
        elif op == '> ++': self.emitOp('INC'); offset = 1
        elif op == '!':    self.emitOp('CALL')
        elif op == '`':    self.emitQuote(var); var = None
        elif op == '=*':   define(var, self.vPC); var = None
        # Depricated syntax
        elif op == '<++':  self.emitOp('INC');             #self.depr('X<++', '<X++')
        elif op == '>++':  self.emitOp('INC'); offset = 1; #self.depr('X>++', '>X++')
        elif op == '<,':   self.emitOp('LD');              #self.depr('X<,', '<X,')
        elif op == '>,':   self.emitOp('LD'); offset = 1;  #self.depr('X>,', '>X,')
        elif op == '<.':   self.emitOp('ST');              #self.depr('X<.', '<X.')
        elif op == '>.':   self.emitOp('ST'); offset = 1;  #self.depr('X>.', '>X.')
        else:
          self.error("Invalid operator '%s' with variable" % op)
        if has(var):
          self.emitVar(var, offset)

      else:
        self.error('Invalid keyword')

  def parseWord(self, word):
    # Break word into pieces

    word += '\0' # Avoid checking len() everywhere
    sign = None
    name, number, op = None, None, ''

    if word[0] == '`':
      # Quoted word
      name, op = word[1:-1], word[0]
      return name, number, op

    ix = 0
    prefixes = ['%', '#', '<', '>', '*', '=']
    if word[ix] in prefixes:
      # Prefix operators
      while word[ix] in prefixes:
        op += word[ix]
        ix += 1
      op += ' ' # Use space to marks prefix operators

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
          number = name[1:] # Forward reference
        name = None

    if sign == '-':
      if has(number):
        number = -number
      else:
        self.error('Can\'t negate')

    op += word[ix:-1]                   # Also strips sentinel '\0'
    return (name, number, op if len(op)>0 else None)

  def sysTicks(self, con):
    # Convert maximum Gigatron cycles to the negative of excess ticks
    if con & 1:
      self.error('Invalid value (must be even, got %d)' % con)
    extraTicks = con/2 - symbol('maxTicks')
    return 256 - extraTicks if extraTicks > 0 else 0

  def emitQuote(self, var):
    # Emit symbol as text, replacing backquotes with spaces
    if len(var) > 0:
      for c in var:
        self.emit(ord(' ' if c == '`' else c))
    else:
      self.emit(ord('`')) # And symbol becomes a backquote

  def emitDef(self):
      self.emitOp('DEF')
      b = self.thisBlock()
      if b in self.defs:
        self.error('Second DEF in block')
      self.defs[b] = self.vPC
      self.emit(lo('__%s_%#04x_def__' % (self.name, self.vPC)))

  def defInfo(self, var):
    # Heuristic to track def lengths
    if var not in self.lengths and self.thisBlock() in self.lengths:
      self.lengths[var] = self.lengths[self.thisBlock()]
    else:
      self.lengths[var] = None # No def lengths can be associated

  def emitLoop(self):
      to = [b for b in self.openBlocks if b in self.loops]
      if len(to) == 0:
        self.error('Loop without do')
      to = self.loops[to[-1]]
      to = prev(to)
      if self.vPC>>8 != to>>8:
        self.error('Loop crosses page boundary')
      self.emitOp('BRA')
      self.emit(to&255)

  def emitIf(self, cond):
      self.emitOp('BCC')
      self.emitOp(cond)
      b = self.thisBlock()
      self.emit(lo('__%s_%d_cond%d__' % (self.name, b, self.elses[b])))

  def emitIfLoop(self, cond):
      to = [blockId for blockId in self.openBlocks if blockId in self.loops]
      if len(to) == 0:
        self.error('Loop without do')
      to = self.loops[to[-1]]
      to = prev(to)
      if self.vPC>>8 != to>>8:
        self.error('Loop to different page')
      self.emitOp('BCC')
      self.emitOp(cond)
      self.emit(to&255)

  def emitElse(self):
      self.emitOp('BRA')
      b = self.thisBlock()
      i = self.elses[b]
      self.emit(lo('__%s_%d_cond%d__' % (self.name, b, i+1)))
      define('__%s_%d_cond%d__' % (self.name, b, i), prev(self.vPC))
      self.elses[b] = i+1

  def emitOp(self, ins):
    # Emit vCPU opcode
    if self.vPC >= self.segEnd:
      self.error('Out of code space (%04x)' % self.vPC)
    if self.segStart == self.vPC:
      self.openSegment()
    self.putInRomTable(lo(ins), '%04x %s' % (self.vPC, ins))
    self.vPC += 1

  def emitVar(self, var, offset=0):
    # Get or create address for GCL variable and emit it
    comment = '%04x %s' % (prev(self.vPC, 1), repr(var))
    comment += '%+d' % offset if offset else ''
    if var not in self.vars:
      self.vars[var] = zpByte(2)
    self.emit(self.vars[var] + offset, comment)

  def thisBlock(self):
    return self.openBlocks[-1]

  def openSegment(self):
    # Write header for GT1 segment
    address = self.segStart
    if not has(self.execute):
      self.execute = address
    assert self.segId == 0 or address>>8 != 0 # Zero-page segment can only be first
    self.putInRomTable(address>>8, '| RAM segment address (high byte first)')
    self.putInRomTable(address&255, '|')
    # Fill in the length through the symbol table
    self.putInRomTable(lo('__%s_seg%d__' % (self.name, self.segId)), '| Length (1..256)')

  def emit(self, byte, comment=None):
    # Next program byte in RAM
    if self.vPC >= self.segEnd:
      self.error('Out of code space (%04x)' % self.vPC)
    if byte < 0 or byte >= 256:
      self.error('Value out of range %d (must be 0..255)' % byte)
    if self.segStart == self.vPC:
      self.openSegment()
    self.putInRomTable(byte, comment)
    self.vPC += 1

  def closeSegment(self):
    # Register length of GT1 segment
    if self.vPC != self.segStart:
      print(' Segment at %04x size %3d used %3d unused %3d' % (
        self.segStart,
        self.segEnd - self.segStart,
        self.vPC - self.segStart,
        self.segEnd - self.vPC))
      length = self.vPC - self.segStart
      assert 1 <= length <= 256
      define('__%s_seg%d__' % (self.name, self.segId), length)
      self.segId += 1

  def putInRomTable(self, byte, comment=None):
    ld(byte)
    if comment:
      C(comment)
    if self.forRom and pc()&255 == 251:
      trampoline()

  def depr(self, old, new):
    self.warning('%s is depricated, please use %s' % (old, new))

  def warning(self, message):
    print(self.prefix('Warning'), message)

  def error(self, message):
    print(self.prefix('Error'), message)
    sys.exit()

  def prefix(self, prefix):
    # Informative line prefix for warning and error messages
    if has(self.filename):
       prefix += ' file %s' % repr(self.filename)
    if self.lineNumber != 0:
      prefix += ':%s' % self.lineNumber
    if has(self.lastWord):
      prefix += ' %s' % self.lastWord
    return prefix + ':'

def has(x):
  return x is not None

def prev(address, step=2):
  # Take vPC two bytes back, wrap around if needed to stay on page
  return (address & ~255) | ((address-step) & 255)
