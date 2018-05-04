#!/usr/bin/env python

import sys
from sys import argv
from os  import getenv

from asm import *
import gcl0x as gcl

# TODO: Save out zpFree in theloop.py
# TODO: Custom start address

if len(argv) is not 3:
  print 'Usage: compilegcl.py SOURCE SYMTABLE'
  print 'Compile SOURCE.gcl file, linking against SYMTABLE'
  sys.exit(1)

def addProgram(gclSource, name, zpFree=0x0030):
  startAdr = pc()
  label(name)
  print 'Compiling file %s label %s' % (gclSource, name)
  program = gcl.Program(0x0200, name, False)
  zpReset(zpFree)
  for line in open(gclSource).readlines():
    program.line(line)
  program.end()
  print 'Success ROM %04x-%04x' % (startAdr, pc())
  print
  return (startAdr, pc())

align(0x100)
link(argv[2])
(gclStart, gclEnd) = addProgram(argv[1], 'Main')

#-----------------------------------------------------------------------
# Finish assembly
#-----------------------------------------------------------------------
stem = basename(splitext(argv[1])[0])
end(stem)

# Extract compiled gcl into separate file
in_filename = stem + '.1.rom'
out_filename = stem + '.gt1'

print 'Create file', out_filename

with open(in_filename, 'rb') as input:
  input.seek(gclStart)
  data = input.read(gclEnd - gclStart)
  with open(out_filename, 'wb') as output:
    output.write(data)
    # Write out start address
    output.write(''.join([chr(0x02), chr(0x00)]))
