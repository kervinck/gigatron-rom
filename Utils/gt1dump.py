#!/usr/bin/env python3
#-----------------------------------------------------------------------
#
#  gt1dump.py -- Tool to dump GT1 files to ASCII
#
# 2019-05-04 (marcelk) Initial version
# 2019-05-05 (marcelk) Added disassembly option -d
# 2019-05-19 (marcelk) Check for bad segment size; Small numbers in decimal
#
#-----------------------------------------------------------------------

import argparse
import pathlib
import re
import sys

# One-for-all error handler (don't throw scary stack traces at the user)
sys.excepthook = lambda exType, exValue, exTrace: print('%s: %s: %s' % (__file__, exType.__name__,  exValue), file=sys.stderr)

#-----------------------------------------------------------------------
#       Command line arguments
#-----------------------------------------------------------------------

parser = argparse.ArgumentParser(description='Dump GT1 file')
parser.add_argument('-d', '--disassemble', dest='disassemble',
                    help='disassmble as vCPU code',
                    action='store_true', default=False)
parser.add_argument('-f', '--force', dest='force',
                    help='accept any filename extension',
                    action='store_true', default=False)
parser.add_argument('filename', help='GT1 file', nargs='?')

args = parser.parse_args()

#-----------------------------------------------------------------------
#       Functions
#-----------------------------------------------------------------------

opcodes = {
  0x11: ('LDWI',  2), 0x1a: ('LD',    1), 0x21: ('LDW',   1),
  0x2b: ('STW',   1), 0x35: ('BCC',   2), 0x59: ('LDI',   1),
  0x5e: ('ST',    1), 0x63: ('POP',   0), 0x75: ('PUSH',  0),
  0x7f: ('LUP',   1), 0x82: ('ANDI',  1), 0x88: ('ORI',   1),
  0x8c: ('XORI',  1), 0x90: ('BRA',   1), 0x93: ('INC',   1),
  0x99: ('ADDW',  1), 0xad: ('PEEK',  0), 0xb4: ('SYS',   1),
  0xb8: ('SUBW',  1), 0xcd: ('DEF',   1), 0xcf: ('CALL',  1),
  0xdf: ('ALLOC', 1), 0xe3: ('ADDI',  1), 0xe6: ('SUBI',  1),
  0xe9: ('LSLW',  0), 0xec: ('STLW',  1), 0xee: ('LDLW',  1),
  0xf0: ('POKE',  1), 0xf3: ('DOKE',  1), 0xf6: ('DEEK',  0),
  0xf8: ('ANDW',  1), 0xfa: ('ORW',   1), 0xfc: ('XORW',  1),
  0xff: ('RET',   0),
}

bccCodes = {
  0x3f: 'EQ', 0x4d: 'GT', 0x50: 'LT', 0x53: 'GE', 0x56: 'LE', 0x72: 'NE',
}


zpInstructions =  [
  'LD', 'LDW', 'STW', 'ST', 'INC', 'ADDW', 'SUBW', 'CALL', 'POKE', 'DOKE',
  'ANDW', 'ORW', 'XORW']

zeroPageSyms = {
 0x00: "zeroConst",
 0x01: "memSize",
 0x06: "entropy",
 0x09: "videoY",
 0x0e: "frameCount",
 0x0f: "serialRaw",
 0x11: "buttonState",
 0x14: "xoutMask",
 0x16: "vPC",
 0x17: "vPC+1",
 0x18: "vAC",
 0x19: "vAC+1",
 0x1a: "vLR",
 0x1b: "vLR+1",
 0x1c: "vSP",
 0x21: "romType",
 0x22: "sysFn",
 0x23: "sysFn+1",
 0x24: "sysArgs",
 0x25: "sysArgs+1",
 0x26: "sysArgs+2",
 0x27: "sysArgs+3",
 0x28: "sysArgs+4",
 0x29: "sysArgs+5",
 0x2a: "sysArgs+6",
 0x2b: "sysArgs+7",
 0x2c: "soundTimer",
 0x2e: "ledState_v2",
 0x2f: "ledTempo",
 0x80: "oneConst",
}

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
  if not args.force:
    suffix = pathlib.Path(args.filename).suffix
    if suffix not in ['.gt1', '.gt1x']:
      raise Exception('Bad extension %s' % repr(suffix))
  print('* file: %s' % args.filename)
  print()
else:
  fp = sys.stdin.buffer # Note: `buffer` for binary mode

hiAddress = readByte(fp)

while True:
  # Dump next segment
  loAddress = readByte(fp)
  address = (hiAddress << 8) + loAddress

  segmentSize = readByte(fp)                    # Segment size
  if segmentSize == 0:
    segmentSize = 256
  if loAddress + segmentSize > 256:
    raise Exception('Bad size %d for segment $%04x' % (segmentSize, address))

  j, text = 0, ''
  ins, ops = None, 0
  for i in range(segmentSize):

    byte = readByte(fp)                         # Data byte

    if args.disassemble and hiAddress > 0:      # Attempt disassembly
      if ops == 0:                              # Not in instruction yet
        if byte in opcodes.keys():
          ins, ops = opcodes[byte]
          if j > 0:                             # Force new line
            print('%s|%s|' % ((51-j)*' ', text))
            j, text = 0, ''
          asm = '%-5s' % ins
      else:                                     # Already in instruction
        if ins == 'LDWI' and ops == 1:
          asm = asm[:-2] + ('%02x' % byte) + asm[-2:]
        elif ins in zpInstructions and ops == 1 and byte in zeroPageSyms:
          if byte == 0x00 and ins[-1] == 'W':
            asm += ' ' + zeroPageSyms[1] + '-1' # Special case top of memory
          else:
            asm += ' ' + zeroPageSyms[byte]     # Known address
        elif ins == 'BCC' and ops == 2 and byte in bccCodes:
          asm = 'B%-4s' % bccCodes[byte]
        elif ins in ['BRA', 'DEF'] or (ins == 'BCC' and ops == 1):
          asm += ' $%02x%02x' % (hiAddress, (byte+2)&255)
        elif ins == 'SYS':
          asm += ' %d' % (28-2*((byte^128)-128))
        else:
          asm += ' $%02x' % byte
        ops -= 1

    if j == 0:                                  # Address on each new line
      print('%04x ' % (address + i), end='')

    print(' %02x' % byte,  end='')              # Print byte as hex value
    j += 3

    if j == 8*3:                                # Visual divider
      print(' ', end='')
      j += 1

    text += chr(byte) if 32<=byte and byte<127 else '.' # ASCII

    if ops == 0:
      if ins:
        # Print as disassembled instruction
        # Convert single digit operand to decimal
        asm = re.sub(r'\$0([0-9])$', r'\1', asm)
        print('%s%-25s|%s|' % ((26 - j) * ' ', asm, text))
        ins, j, text = None, 0, ''

      elif (address + i) & 15 == 15 or i == segmentSize - 1:
        # Print as pure data
        print('%s|%s|' % ((51-j)*' ', text))
        j, text = 0, ''

  if ops > 0: # incomplete instruction
    print('%s%-25s|%s|' % ((26 - j) * ' ', asm + ' ??', text))
    print('* incomplete instruction')

  print('* %d byte%s' % (segmentSize, '' if segmentSize == 1 else 's'))
  print()

  hiAddress = readByte(fp)
  if hiAddress == 0:
    break

# Dump start address
hiAddress, loAddress = readByte(fp), readByte(fp)
address = (hiAddress << 8) + loAddress
if address:
  print('* start at $%04x' % address)
  print()

if len(fp.read()) > 0:
  raise Exception('Excess bytes')

if args.filename:
  fp.close()

#-----------------------------------------------------------------------
#
#-----------------------------------------------------------------------

