#!/usr/bin/env python

class Compiler:
  def __init__(self):
    self.comment = 0
    self.lineNumber = 0
    self.filename = None
    self.block = 0
    self.blocks = []

  def line(self, line):
    """Process a line by tokenizing and processing the words"""

    print
    self.lineNumber += 1
    nextWord = ''

    for nextChar in line:
      if self.comment > 0:
        if nextChar == '{': self.comment += 1
        if nextChar == '}': self.comment -= 1
        # Inside comments anything goes
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
        elif nextChar == ']': self.blocks.pop()
        elif nextChar == '(': pass
        elif nextChar == ')': pass
    self.word(nextWord)

  def word(self, word):
    """Process a word and emit its code"""
    if len(word) == 0:
      return
    print 'word', repr(word)

    ix = 0
    unnamed = False
    negative = False
    number = 0
    name = ''
    ops = ''

    # Rip the word apart
    if word[ix] == '%': unnamed=True; ix+=1
    if word[ix] in '-+' and len(word)>ix:
      if word[ix+1].isdigit() or word[ix+1] == '$':
        negative = (word[ix] == '-')
        ix += 1
    if word[ix] == '$':
      ix, jx = ix+1, ix+1
      while ix < len(word):
        o = ord(word[ix])
        if word[ix] in '0123456789': number = 16*number + o - ord('0')
        elif word[ix] in 'abcdef': number = 16*number + o - ord('a')
        elif word[ix] in 'ABCDEF': number = 16*number + o - ord('A')
        else: break
        ix += 1
      if jx==ix: self.error('Missing hex in %s' % repr(word))
    elif word[ix].isdigit():
      while ix<len(word) and word[ix].isdigit():
        number = 10*number + ord(word[ix]) - ord('0')
        ix += 1
    elif word[ix].isupper():
      while ix<len(word) and word[ix].isalnum():
        name += word[ix]
        ix += 1
    elif word[ix].islower():
      while ix<len(word) and word[ix].isalnum():
        name += word[ix]
        ix += 1
    ops += word[ix:]

  def end(self):
     pass # XXX Check all blocks are closed

  def error(self, message):
    prefix = 'file %s' % repr(self.filename) if self.filename else ''
    prefix += ' line %s:' % self.lineNumber
    raise SyntaxError(prefix + message)

if __name__ == '__main__':
  gcl = Compiler()
  for line in open('fibo.gcl').readlines():
    gcl.line(line)
  gcl.end()
