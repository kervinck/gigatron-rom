
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
    self.comments = []   # Stack of line numbers
    self.lineNumber = 0
    self.lastWord = None
    self.filename = None
    self.openBlocks = [0] # Outside first block is 0
    self.nextBlockId = 1
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
    loadBindings('Core/v6502.json') # XXX Provisional method to load mnemonics

  def org(self, address):
    """Set start address"""
    self.closeSegment()
    # Don't open new segment before the first byte comes
    self.segStart = address
    self.vPC = address
    page = address & ~255
    self.segEnd = page + (250 if 0x100 <= page <= 0x400 else 256)

  def line(self, line):
    """Process a line by tokenizing and processing the words"""

    self.lineNumber += 1
    nextWord = ''

    for nextChar in line:
      if len(self.comments) > 0:
        # Inside comments anything goes
        if nextChar == '{': self.comments.append(self.lineNumber)
        if nextChar == '}': self.comments.pop()
      elif nextChar not in '{}[]':
        if nextChar.isspace():
          self.word(nextWord)
          nextWord = ''
        else:
          nextWord += nextChar
      else:
        self.word(nextWord)
        nextWord = ''
        if nextChar == '{': self.comments.append(self.lineNumber)
        elif nextChar == '}': self.error('Spurious %s' % repr(nextChar))
        elif nextChar == '[':
           self.openBlocks.append(self.nextBlockId)
           self.elses[self.nextBlockId] = 0
           self.nextBlockId += 1
        elif nextChar == ']':
          if len(self.openBlocks) <= 1:
            self.error('Block close without open')
          b = self.openBlocks.pop()
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
    if len(self.comments) > 0:
      self.lineNumber = self.comments[-1]
      self.error('Unterminated comment')
    self.closeSegment()
    if len(self.openBlocks) > 1:
      self.error('Unterminated block')
    self.putInRomTable(0) # Zero marks the end of stream
    if self.lineNumber > 0:
      self.dumpVars()

  def dumpVars(self):
    print(' Variables count %d bytes %d end $%04x' % (len(self.vars), 2*len(self.vars), zpByte(0)))
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
    # Process a GCL word and emit its corresponding vCPU code
    if len(word) == 0:
      return
    self.lastWord = word

    # Simple keywords
    if not has(self.version):
      if word in ['gcl0x']:
        self.version = word
      else:
        self.error('Invalid GCL version')
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
    elif word == 'call':      self.emitOp('CALL').emit(symbol('vAC'), '%04x vAC' % prev(self.vPC, 1))
    elif word == 'push':      self.emitOp('PUSH')
    elif word == 'pop':       self.emitOp('POP')
    elif word == 'ret':       self.emitOp('RET'); self.needPatch = self.needPatch or len(self.openBlocks) == 1 # Top-level use of 'ret' --> apply patch
    elif word == 'peek':      self.emitOp('PEEK')
    elif word == 'deek':      self.emitOp('DEEK')
    else:
      var, con, op = self.parseWord(word)

      # Label definitions
      if has(var) and has(con):
        if   op == '=' and var == 'zpReset': zpReset(con)
        elif op == '=' and var == 'execute': self.execute = con
        elif op == '=': self.defSymbol(var, con)
        else: self.error("Invalid operator '%s' with name and constant" % op)

      # Words with constant value as operand
      elif has(con):
        if not has(op):
          if isinstance(con, int) and 0 <= con < 256:
            self.emitOp('LDI')
          else:
            self.emitOp('LDWI').emit(lo(con)); con = hi(con)
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
        elif op == '--':   self.emitOp('ALLOC'); con = 256-con if con else 0
        elif op == '++':   self.emitOp('ALLOC')
        elif op == '< ++': self.emitOp('INC')
        elif op == '> ++': self.emitOp('INC'); con += 1
        elif op == '!!':   self.emitOp('SYS'); con = self.sysTicks(con)
        elif op == '!':
          if isinstance(con, int) and 0 <= con < 256:
            # XXX Deprecate in gcl1, replace with i!!
            self.emitOp('SYS'); con = self.sysTicks(con);self.depr('i!', 'i!!')
          else:
            self.emitOp('CALLI_v5').emit(lo(con)); con = hi(con)
        elif op == '?':    self.emitOp('LUP');          #self.depr('i?', 'i??')
        elif op == '??':   self.emitOp('LUP')
        elif op == '# ':   self.emitOp(con); con = None # Silent truncation
        elif op == '#< ':  self.emitOp(con); con = None
        elif op == '#> ':  con = hi(con); assert self.segStart != self.vPC # XXX Conflict
        elif op == '## ':  self.emit(lo(con)).emit(hi(con)); con = None
        elif op == '<<':
          for i in range(con):
            self.emitOp('LSLW')
          con = None
        # Deprecated syntax
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

      # Words with variable or symbol name as operand
      elif has(var):
        offset = 0
        if not has(op):    self.emitOp('LDW')
        elif op == '=':    self.emitOp('STW'); self.updateDefInfo(var)
        elif op == ',':    self.emitOp('LDW').emitVar(var).emitOp('PEEK'); var = None
        elif op == ';':    self.emitOp('LDW').emitVar(var).emitOp('DEEK'); var = None
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
        elif op == '`':    self.emitQuote(var);                 var = None
        elif op == '=*':   self.defSymbol(var, self.vPC);       var = None
        elif op == '# ':   self.emitImm(var);                   var = None
        elif op == '#< ':  self.emitImm(var);                   var = None
        elif op == '#> ':  self.emitImm(var, half=hi);          var = None
        elif op == '## ':  self.emitImm(var).emit(hi(var[1:])); var = None
        elif op == '#@ ':  offset = -self.vPC-1 # PC relative, 6502 style
        # Deprecated syntax
        elif op == '<++':  self.emitOp('INC');             #self.depr('X<++', '<X++')
        elif op == '>++':  self.emitOp('INC'); offset = 1; #self.depr('X>++', '>X++')
        elif op == '<,':   self.emitOp('LD');              #self.depr('X<,', '<X,')
        elif op == '>,':   self.emitOp('LD'); offset = 1;  #self.depr('X>,', '>X,')
        elif op == '<.':   self.emitOp('ST');              #self.depr('X<.', '<X.')
        elif op == '>.':   self.emitOp('ST'); offset = 1;  #self.depr('X>.', '>X.')
        else:
          self.error("Invalid operator '%s' with variable or symbol '%s'" % (op, var))
        if has(var):
          self.emitVar(var, offset)

      else:
        self.error('Invalid word')

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
    prefixes = ['%', '#', '<', '>', '*', '=', '@']
    if word[ix] in prefixes:
      # Prefix operators
      while word[ix] in prefixes:
        op += word[ix]
        ix += 1
      op += ' ' # Space to demarcate prefix operators

    if word[ix].isalpha() or word[ix] in ['&', '\\', '_']:
      # Named variable or named constant
      name = word[ix]
      ix += 1
      while word[ix].isalnum() or word[ix] == '_':
        name += word[ix]
        ix += 1

    if word[ix] == '=':
      # Infix symbol definition
      op += word[ix]
      # op += ' ' # Space to demarcate infix operator
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
    elif has(sign):
      op += sign
      sign = None
    else:
      pass

    # Resolve '&_symbol' as the number it represents
    if has(name) and name[0] == '&':
      if name[1] == '_':
        number = symbol(name[2:])
      if not has(number):
        number = name[2:] # Pass back as an unresolved reference without '_'
      name = None

    # Resolve '\symbol' as the number it represents
    if has(name) and name[0] == '\\':
      # Peeking into the assembler's symbol table (not GCL's)
      # Substitute \symbol with its value, and keeping the operator
      number = symbol(name[1:])
      if not has(number):
        number = name[1:] # Pass back as an unresolved reference
      name = None

    if sign == '-':
      if has(number) and isinstance(number, int):
        number = -number
      else:
        self.error('Unable to negate')

    op += word[ix:-1]                   # Also strips sentinel '\0'
    return (name, number, op if len(op)>0 else None)

  def sysTicks(self, con):
    # Convert maximum Gigatron cycles to the negative of excess ticks
    if con & 1:
      self.error('Invalid value (must be even, got %d)' % con)
    extraTicks = con//2 - symbol('maxTicks')
    return 256 - extraTicks if extraTicks > 0 else 0

  def emitQuote(self, var):
    if len(var) > 0:
      d = '' # Replace backquotes with spaces
      for c in var:
        d += ' ' if c == '`' else c
    else:
      d = '`' # And symbol becomes a backquote
    for c in d:
      comment = '%04x %s' % (self.vPC, repr(c))
      self.emit(ord(c), comment=comment)

  def emitDef(self):
      self.emitOp('DEF')
      b = self.thisBlock()
      if b in self.defs:
        self.error('Second DEF in block')
      self.defs[b] = self.vPC
      self.emit(lo('__%s_%#04x_def__' % (self.name, self.vPC)))

  def updateDefInfo(self, var):
    # Heuristically track `def' lengths for reporting on stdout
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
    self.prepareSegment()
    self.putInRomTable(lo(ins), '%04x %s' % (self.vPC, ins))
    self.vPC += 1
    return self

  def emitVar(self, var, offset=0):
    # Get or create address for GCL variable and emit it
    # !!! Also safe at start of segment !!!
    self.prepareSegment()
    if var[0] == '_':
      # _C notation for labels as variables
      address, offset = lo(var[1:]), offset & 255
    else:
      # Regular GCL variable
      if var not in self.vars:
        self.vars[var] = zpByte(2)
      address = self.vars[var]
    comment = '%04x %s' % (prev(self.vPC, 1), repr(var))
    comment += '%+d' % offset if offset else ''
    byte = address + offset
    if byte < -128 or byte >= 256:
      self.error('Value %s out of range (must be -128..255)' % repr(byte))
    self.putInRomTable(byte, comment)
    self.vPC += 1
    return self

  def emitImm(self, var, half=lo):
    # Emit low or high byte of symbol
    # !!! Also safe at start of segment !!!
    #
    # Here we see the subtle differences between variables and named constants
    # again. For named constants (preceeded by '_'), we want their value.
    # For named variables, we want their address. This becomes evident with the
    # '>' modifier: constant>>8 vs. address+1
    self.prepareSegment()
    if var[0] == '_':
      address = half(var[1:])
    else:
      if var not in self.vars:
        self.vars[var] = zpByte(2)
      address = self.vars[var]
      if half is hi:
        address += 1
        var = '>' + var
    self.putInRomTable(address, '%04x %s' % (self.vPC, var))
    self.vPC += 1
    return self

  def thisBlock(self):
    return self.openBlocks[-1]

  def prepareSegment(self):
    # Check if there's space in the current segment
    if self.vPC >= self.segEnd:
      severity = self.warning if self.vPC & 255 > 0 else self.error
      severity('Out of code space ($%04x)' % self.vPC)

    # And write header bytes for a new segment
    if self.segStart == self.vPC:
      # This must come before any lo() or hi()
      # Write header for GT1 segment
      address = self.segStart
      if not has(self.execute) and address >= 0x200:
        self.execute = address
      assert self.segId == 0 or address>>8 != 0 # Zero-page segment can only be first
      self.putInRomTable(address>>8, '| RAM segment address (high byte first)')
      self.putInRomTable(address&255, '|')
      # Fill in the length through the symbol table
      self.putInRomTable(lo('__%s_seg%d__' % (self.name, self.segId)), '| Length (1..256)')

  def emit(self, byte, comment=None):
    # Next program byte in RAM
    self.prepareSegment()
    if not isinstance(byte, (int, float)):
      self.error('Invalid value (number expected, got %s)' % repr(byte))
    if byte < -128 or byte >= 256:
      self.error('Value %s out of range (must be -128..255)' % repr(byte))
    self.putInRomTable(byte, comment)
    self.vPC += 1
    return self

  def closeSegment(self):
    # Register length of GT1 segment
    if self.vPC != self.segStart:
      print(' Segment at $%04x size %3d used %3d unused %3d' % (
        self.segStart,
        self.segEnd - self.segStart,
        self.vPC - self.segStart,
        self.segEnd - self.vPC))
      length = self.vPC - self.segStart
      assert 1 <= length <= 256
      define('__%s_seg%d__' % (self.name, self.segId), length)
      self.segId += 1

  def putInRomTable(self, byte, comment=None):
    if byte < -128 or byte >= 256:
      self.error('Value %s out of range (must be -128..255)' % repr(byte))
    ld(byte)
    if comment:
      C(comment)
    if self.forRom and pc()&255 == 251:
      trampoline()

  def depr(self, old, new):
    var, con, _op = self.parseWord(self.lastWord)
    old = old.replace(' ', str(con) if has(con) else var)
    new = new.replace(' ', str(con) if has(con) else var)
    self.warning('%s is deprecated, please use %s' % (old, new))

  def warning(self, message):
    highlight(self.prefix('Warning'), message)

  def error(self, message):
    highlight(self.prefix('Error'), message)
    sys.exit(1)

  def prefix(self, prefix):
    # Informative line prefix for warning and error messages
    if has(self.filename):
       prefix += ' file %s' % repr(self.filename)
    if self.lineNumber != 0:
      prefix += ':%s' % self.lineNumber
    if has(self.lastWord):
      prefix += ' (%s)' % self.lastWord
    return prefix + ':'

  def defSymbol(self, name, value):
    # Define a label from GCL in the systems symbol table
    if name[0] != '_':
      self.error('Symbol \'%s\' must begin with underscore (\'_\')' % name)
    define(name[1:], value)

def prev(address, step=2):
  # Take vPC two bytes back, wrap around if needed to stay on page
  return (address & ~255) | ((address-step) & 255)
