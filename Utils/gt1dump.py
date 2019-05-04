#!/usr/bin/env python3
#-----------------------------------------------------------------------
#
#  gt1dump.py -- Tool to dump GT1 files to ASCII
#
# 2019-05-04 (marcelk) Initial version
#
#-----------------------------------------------------------------------

import argparse

import sys

# One-for-all error handler (don't throw scary stack traces at the user)
sys.excepthook = lambda exType, exValue, exTrace: print('%s: %s: %s' % (__file__, exType.__name__,  exValue), file=sys.stderr)

#-----------------------------------------------------------------------
#       Command line arguments
#-----------------------------------------------------------------------

parser = argparse.ArgumentParser(description='Dump GT1 file')
parser.add_argument('filename', help='GT1 file', nargs='?')

args = parser.parse_args()

#-----------------------------------------------------------------------
#       Functions
#-----------------------------------------------------------------------

def readByte(fp):
  byte = fp.read(1)
  if len(byte) == 0:
    raise Exception('Unexpected end of file')
  return byte[0]

#-----------------------------------------------------------------------
#       Main
#-----------------------------------------------------------------------

if args.filename:
  fp = open(args.filename, 'rb')
else:
  fp = sys.stdin

hiAddress = readByte(fp)

while True:
  # Dump next segment
  loAddress = readByte(fp)
  address = (hiAddress << 8) + loAddress

  segmentSize = readByte(fp)                    # Segment length
  if segmentSize == 0:
    segmentSize = 256

  text = ''
  for i in range(segmentSize):

    if i == 0 or (address + i) & 15 == 0:
      print('%04x ' % (address + i), end='')    # Address on each line

    byte = readByte(fp)                         # Data byte
    print(' %02x' % byte,  end='')
    text += chr(byte) if 32 <= byte and byte < 127 else '.'

    if len(text) == 8:                          # Visual divider
      print(' ', end='')

    if (address + i) & 15 == 15 or i == segmentSize - 1:
      align = (16 - len(text)) * 3 + 2
      if len(text) < 8:
        align += 1
      print('%s|%s|' % (align* ' ', text))      # As text
      text = ''

  print('* %d byte%s' % (segmentSize, '' if segmentSize == 1 else 's'))
  print()

  hiAddress = readByte(fp)
  if hiAddress == 0:
    break

# Dump start address
hiAddress, loAddress = readByte(fp), readByte(fp)
address = (hiAddress << 8) + loAddress
if address:
  print('* start at 0x%04x' % (address))
  print()

if len(fp.read()) > 0:
  raise Exception('Excess bytes')

if args.filename:
  fp.close()

#-----------------------------------------------------------------------
#
#-----------------------------------------------------------------------

