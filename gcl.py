#!/usr/bin/env python

from asm import *

class Program:
  def __init__(self, address):
    self.comment = 0
    self.lineNumber, self.filename = 0, None
    self.blocks, self.block = [0], 1
    self.loops = {} # block -> address of last do
    self.conds = {} # block -> address of continuation

    # Configure for start address
    self.vPC = address
    ld(val(address&0xff),regX)
    ld(val(address>>8), regY)

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
            define('$if.%d.%d' % (block, self.conds[block]), prev(self.vPC))
            del self.conds[block]
        elif nextChar == '(': pass
        elif nextChar == ')': pass
    self.word(nextWord)

  def getAddress(self, var):
    if len(var) > 1:
      self.error('Name too long %s' % repr(var))
    o = ord(var[0]) - ord('A')
    return 0x81 + 2*o # XXX Proper allocation

  def emit(self, byte):
    """Next program byte in RAM"""
    st(val(byte), eaYXregOUTIX) # XXX Use ROM tables
    self.vPC += 1

  def word(self, word):
    """Process a word and emit its code"""
    if len(word) == 0:
      return

    if word == 'loop':
      to = self.loops[self.thisBlock()]
      to = prev(to)
      self.emit(lo('JUMP'))
      self.emit(to&0xff)
      self.emit(to>>8)
    elif word == 'do':
      self.loops[self.thisBlock()] = self.vPC
    elif word == 'if<0':
      self.emit(lo('SIGNW'))
      self.emit(lo('BGE'))
      block = self.thisBlock()
      self.emit(lo('$if.%d.0' % block))
      if block in self.conds:
        self.error('Too many %s' % repr(word)) # XXX Think harder about this
      self.conds[block] = 0
    elif word == 'if>0':
      self.emit(lo('SIGNW'))
      self.emit(lo('BLE'))
      block = self.thisBlock()
      self.emit(lo('$if.%d.0' % block))
      self.conds[block] = 0
    elif word == 'else':
      block = self.thisBlock()
      if block not in self.conds:
        self.error('Unexpected %s' % repr(word))
      if self.conds[block] > 0:
        self.error('Too many %s' % repr(word))
      self.emit(lo('BRA'))
      self.emit(lo('$if.%d.1' % block))
      define('$if.%d.0' % block, prev(self.vPC))
      self.conds[block] = 1
    else:
      var, con, op = self.parseWord(word) # XXX Simplify this
      if op is None:
        if var:
          if var[0].isupper() and len(var) == 1:
            self.emit(lo('LDW'))
            self.emit(self.getAddress(var))
          else:
            self.error('Not implemented %s' % repr(word))
        else:
          self.emit(lo('LDWI'))
          self.emit(con&0xff)
          self.emit(con>>8)
      elif op == '=' and var:
          self.emit(lo('STW'))
          self.emit(self.getAddress(var))
      elif op == '+' and var:
          self.emit(lo('ADDW'))
          self.emit(self.getAddress(var))
      elif op == '&' and con:
          if con<0 or 0xff<con:
            self.error('Out of range %s' % repr(con))
          self.emit(lo('ANDI'))
          self.emit(con)
      elif op == '|' and con:
          if con<0 or 0xff<con:
            self.error('Out of range %s' % repr(con))
          self.emit(lo('ORI'))
          self.emit(con)
      elif op == '!' and var:
          self.emit(lo('POKE'))
          self.emit(self.getAddress(var))
      else:
        self.error('Invalid word %s' % repr(word))

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
    else:
      # Named variable?
      number = None
      if unnamed or sign:
         ix = 0 # Reset
      else:
        name = ''
        while word[ix].isalnum():
          name += word[ix]
          ix += 1
        name = name if len(name)>0 else None

    if number is not None:
      if sign == '-': number = -number
      if unnamed: name, number = number, None

    op = word[ix:-1]
    return (name, number, op if len(op)>0 else None)

  def end(self):
     pass # XXX Check all blocks are closed
     if len(self.conds) > 0:
       self.error('Dangling if statements')

  def error(self, message):
    prefix = 'file %s' % repr(self.filename) if self.filename else ''
    prefix += ' line %s:' % self.lineNumber
    raise SyntaxError(prefix + message)

def prev(address, step=2):
  """Take vPC two bytes back, wrap around if needed to stay on page"""
  return (address & 0xff00) | ((address-step) & 0x00ff)

