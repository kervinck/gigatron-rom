#!/usr/bin/env python

import sys
from sys import argv
from os  import getenv

from asm import *
import gcl0x as gcl

# TODO: Get vCpuSart and zpFree from bindings.json
# TODO: Replace SYMTABLE with a more formally defined ABI (e.g. "bindings.json")
# TODO: Reverse the concept: first compile to GT1, then include it into ROM file

vCpuStart = 0x0200 # default
zpFree    = 0x30   # default

if len(argv) is not 3:
  print 'Usage: compilegcl.py SOURCE SYMTABLE'
  print 'Compile SOURCE.gcl file, linking against SYMTABLE'
  sys.exit(1)

def addProgram(gclSource):
  print 'Compiling file %s' % gclSource
  program = gcl.Program(vCpuStart, 'Main', forRom=False)
  align(1)        # Forces default maximum ROM size
  zpReset(zpFree) # User variables can start here
  for line in open(gclSource).readlines():
    program.line(line)
  program.end()
  end() # End assembly
  data = getRom1()

  address = program.execute

  # Inject patch for reliable start using ROM v1 Loader application
  # See: https://forum.gigatron.io/viewtopic.php?p=27#p27
  if program.needPatch:
    patchArea = 0x5b86 # Somewhere after the ROMv1 Loader's buffer
    print 'Apply patch %04x' % patchArea
    data = data[:-1] # Remove terminating zero
    data += ''.join(chr(byte) for byte in [
      patchArea>>8, patchArea&255, 6,   # Patch segment, 6 bytes at $5b80
      0x11, address&255, address>>8,    # LDWI address
      0x2b, 0x1a,                       # STW  vLR
      0xff,                             # RET
      0x00
    ])
    address = patchArea

  # Final two bytes are execution address
  print 'Execute at %04x' % address
  data += chr(address>>8)
  data += chr(address&255)

  return data

loadBindings(argv[2])
data = addProgram(argv[1])

#-----------------------------------------------------------------------
#  Write out GT1 file
#-----------------------------------------------------------------------

stem = basename(splitext(argv[1])[0])
gt1File = stem + '.gt1'    # Resulting object file

print 'Create file', gt1File

with open(gt1File, 'wb') as output:
  output.write(data)

print 'OK size', len(data)
