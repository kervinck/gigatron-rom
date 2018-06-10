#!/usr/bin/env python

import sys
from sys import argv
from os  import getenv

from asm import *
import gcl0x as gcl

# TODO: Save out zpFree in theloop.py
# TODO: Replace SYMTABLE with a more formally defined ABI (e.g. "bindings.json")
# TODO: Reverse the concept: first compile to GT1, then include it into ROM file

vCpuStart = 0x200 # default

if len(argv) is not 3:
  print 'Usage: compilegcl.py SOURCE SYMTABLE'
  print 'Compile SOURCE.gcl file, linking against SYMTABLE'
  sys.exit(1)

def addProgram(gclSource, name, zpFree=0x0030):
  pcStart = pc()
  label(name)
  print 'Compiling file %s label %s' % (gclSource, name)
  program = gcl.Program(vCpuStart, name, forRom=False)
  zpReset(zpFree)
  for line in open(gclSource).readlines():
    program.line(line)
  program.end()
  end() # End assembly
  print 'Success ROM %04x-%04x' % (pcStart, pc())
  print
  data = getRom1()
  return data[pcStart:pc()]

align(0x100)
loadBindings(argv[2])
data = addProgram(argv[1], 'Main')

#-----------------------------------------------------------------------
# Inject patch for reliable start using ROM v1 Loader application
# See: https://forum.gigatron.io/viewtopic.php?p=27#p27
#-----------------------------------------------------------------------

patchArea = 0x5b86 # Somewhere after the ROMv1 Loader's buffer

data = data[:-1]
data += ''.join(chr(byte) for byte in [
  patchArea>>8, patchArea&255, 6,   # Patch segment, 6 bytes at $5b80
  0x11, vCpuStart&255, vCpuStart>>8,# LDWI vCpuStart
  0x2b, 0x1a,                       # STW  vLR
  0xff,                             # RET
  0x00, patchArea>>8, patchArea&255 # Execute: run patch first
])

#-----------------------------------------------------------------------
#  Write out GT1 file
#-----------------------------------------------------------------------

stem = basename(splitext(argv[1])[0])
gt1File = stem + '.gt1'    # Resulting object file

print 'Create file', gt1File

with open(gt1File, 'wb') as output:
  output.write(data)
