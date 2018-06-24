#!/usr/bin/env python
#-----------------------------------------------------------------------
#
#  compilegcl.py -- Compile GCL source to GT1 object file
#
# 2018-05-04 (Cwiiis)  Initial version
# 2018-06-11 (marcelk) Use JSON interface file instead of symbol table dump
# 2018-06-24 (at67)    Optional output directory
#
#-----------------------------------------------------------------------

from __future__ import print_function
import argparse
import sys
from sys import argv
from os  import getenv
from os.path import basename, splitext

import asm
import gcl0x as gcl

# One-for-all error handler (don't throw scary stack traces at the user)
sys.excepthook = lambda exType, exValue, exTrace: print('%s: %s' % (exType.__name__,  exValue))

#-----------------------------------------------------------------------
#       Command line arguments
#-----------------------------------------------------------------------

parser = argparse.ArgumentParser(description='Compile GCL source to GT1 object file')
parser.add_argument('-s', '--sym', dest='sym',
                    help='Symbol file for interface bindings (default interface.json)',
                    default='interface.json')
parser.add_argument('gclSource', help='GCL file')
parser.add_argument('outputDir', nargs='?', default='.', help='Optional output directory')
args = parser.parse_args()

#-----------------------------------------------------------------------
#       Compile
#-----------------------------------------------------------------------

asm.loadBindings(args.sym)

userCode = asm.symbol('userCode')
userVars = asm.symbol('userVars')

print('Compiling file %s' % args.gclSource)
program = gcl.Program(userCode, 'Main', forRom=False)
asm.align(1)          # Forces default maximum ROM size
asm.zpReset(userVars) # User variables can start here
for line in open(args.gclSource).readlines():
  program.line(line)
program.end()
asm.end() # End assembly
data = asm.getRom1()

#-----------------------------------------------------------------------
#       Append ending
#-----------------------------------------------------------------------

address = program.execute

# Inject patch for reliable start using ROM v1 Loader application
# See: https://forum.gigatron.io/viewtopic.php?p=27#p27
if program.needPatch:
  patchArea = 0x5b86 # Somewhere after the ROMv1 Loader's buffer
  print('Apply patch %04x' % patchArea)
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
print('Execute at %04x' % address)
data += chr(address>>8)
data += chr(address&255)

#-----------------------------------------------------------------------
#       Write out GT1 file
#-----------------------------------------------------------------------

stem = basename(splitext(args.gclSource)[0])
gt1File = args.outputDir + '/' + stem + '.gt1' # Resulting object file

print('Create file', gt1File)

with open(gt1File, 'wb') as output:
  output.write(data)

print('OK size', len(data))
print()

#-----------------------------------------------------------------------
#
#-----------------------------------------------------------------------
