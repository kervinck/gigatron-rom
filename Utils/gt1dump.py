#!/usr/bin/env python3
#-----------------------------------------------------------------------
#
#  gt1dump.py -- Tool to dump GT1 files to ASCII
#
# 2019-05-04 (marcelk) Initial version
# 2019-05-05 (marcelk) Added disassembly option -d
# 2019-05-19 (marcelk) Check for bad segment size; Small numbers in decimal
# 2019-10-20 (marcelk) Disassemble CALLI, CMPHU, CMPHS
# 2020-02-23 (marcelk) Disassemble v6502 instructions
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

opcodes = [
  { # [0] is vCPU
    0x11: ('LDWI',  2), 0x1a: ('LD',    1), 0x1f: ('CMPHS', 1),
    0x21: ('LDW',   1), 0x2b: ('STW',   1), 0x35: ('Bcc',   2),
    0x59: ('LDI',   1), 0x5e: ('ST',    1), 0x63: ('POP',   0),
    0x75: ('PUSH',  0), 0x7f: ('LUP',   1), 0x82: ('ANDI',  1),
    0x85: ('CALLI', 2), 0x88: ('ORI',   1), 0x8c: ('XORI',  1),
    0x90: ('BRA',   1), 0x93: ('INC',   1), 0x97: ('CMPHU', 1),
    0x99: ('ADDW',  1), 0xad: ('PEEK',  0), 0xb4: ('SYS',   1),
    0xb8: ('SUBW',  1), 0xcd: ('DEF',   1), 0xcf: ('CALL',  1),
    0xdf: ('ALLOC', 1), 0xe3: ('ADDI',  1), 0xe6: ('SUBI',  1),
    0xe9: ('LSLW',  0), 0xec: ('STLW',  1), 0xee: ('LDLW',  1),
    0xf0: ('POKE',  1), 0xf3: ('DOKE',  1), 0xf6: ('DEEK',  0),
    0xf8: ('ANDW',  1), 0xfa: ('ORW',   1), 0xfc: ('XORW',  1),
    0xff: ('RET',   0),
  },
  { # [1] is v6502
    0: ('BRK',0),      1: ('ORAIX',1),
    5: ('ORAZ',1),     6: ('ASLZ',1),
    8: ('PHP',0),      9: ('ORAIM',1),    10: ('ASLA',0),
   13: ('ORA',2),     14: ('ASL',2),
   16: ('BPL',1),     17: ('ORAIY',1),
   21: ('ORAZX',1),   22: ('ASLZX',1),
   24: ('CLC',0),     25: ('ORAAY',2),
   29: ('ORAAX',2),   30: ('ASLAX',2),
   32: ('JSR',2),     33: ('ANDIX',1),
   36: ('BITZ',1),    37: ('ANDZ',1),     38: ('ROLZ',1),
   40: ('PLP',0),     41: ('ANDIM',1),    42: ('ROLA',0),
   44: ('BIT',2),     45: ('AND',2),      46: ('ROL',2),
   48: ('BMI',1),     49: ('ANDIY',1),
   53: ('ANDZX',1),   54: ('ROLZX',1),
   56: ('SEC',0),     57: ('ANDAY',2),
   61: ('ANDAX',2),   62: ('ROLAX',2),
   64: ('RTI',0),     65: ('EORIX',1),
   69: ('EORZ',1),    70: ('LSRZ',1),
   72: ('PHA',0),     73: ('EORIM',1),    74: ('LSRA',0),
   76: ('JMP',2),     77: ('EOR',2),      78: ('LSR',2),
   80: ('BVC',1),     81: ('EORIY',1),
   85: ('EORZX',1),   86: ('LSRZX',1),
   88: ('CLI',0),     89: ('EORAY',2),
   93: ('EORAX',2),   94: ('LSRAX',2),
   96: ('RTS',0),     97: ('ADCIX',1),
  101: ('ADCZ',1),   102: ('RORZ',1),
  104: ('PLA',0),    105: ('ADCIM',1),  106: ('RORA',0),
  108: ('JMI',2),    109: ('ADC',2),    110: ('ROR',2),
  112: ('BVS',1),    113: ('ADCIY',1),
  117: ('ADCZX',1),  118: ('RORZX',1),
  120: ('SEI',0),    121: ('ADCAY',2),
  125: ('ADCAX',2),  126: ('RORAX',2),
  129: ('STAIX',1),
  132: ('STYZ',1),   133: ('STAZ',1),  134: ('STXZ',1),
  136: ('DEY',0),    138: ('TXA',0),
  140: ('STY',2),    141: ('STA',2),   142: ('STX',2),
  144: ('BCC',1),    145: ('STAIY',1),
  148: ('STYZX',1),  149: ('STAZX',1), 150: ('STXZY',1),
  152: ('TYA',0),    153: ('STAAY',2), 154: ('TXS',0),
  157: ('STAAX',2),
  160: ('LDYIM',1),  161: ('LDAIX',1), 162: ('LDXIM', 1),
  164: ('LDYZ',1),   165: ('LDAZ',1),  166: ('LDXZ',1),
  168: ('TAY',0),    169: ('LDAIM',1), 170: ('TAX',0),
  172: ('LDY',2),    173: ('LDA',2),   174: ('LDX',2),
  176: ('BCS',1),    177: ('LDAIY',1),
  180: ('LDYZX',1),  181: ('LDAZX',1), 182: ('LDXZY',1),
  184: ('CLV',0),    185: ('LDAAY',2), 186: ('TSX',0),
  188: ('LDYAX',2),  189: ('LDAAX',2), 190: ('LDXAY',2),
  192: ('CPYIM',1),  193: ('CMPIX',1),
  196: ('CPYZ',1),   197: ('CMPZ',1),  198: ('DECZ',1),
  200: ('INY',0),    201: ('CMPIM',1), 202: ('DEX',0),
  204: ('CPY',2),    205: ('CMP',2),   206: ('DEC',2),
  208: ('BNE',1),    209: ('CMPIY',1),
  213: ('CMPZX',1),  214: ('DECZX',1),
  216: ('CLD',0),    217: ('CMPAY',2),
  221: ('CMPAX',2),  222: ('DECAX',2),
  224: ('CPXIM',1),  225: ('SBCIX',1),
  228: ('CPXZ',1),   229: ('SBCZ',1),  230: ('INCZ',1),
  232: ('INX',2),    233: ('SBCIM',1), 234: ('NOP',0),
  236: ('CPX',2),    237: ('SBC',2),   238: ('INC',2),
  240: ('BEQ',1),    241: ('SBCIY',1),
  245: ('SBCZX',1),  246: ('INCZX',1),
  248: ('SED',0),    249: ('SBCAY',2),
  253: ('SBCAX',2),  254: ('INCAX',2)
 }
]

bccCodes = {
  0x3f: 'EQ', 0x4d: 'GT', 0x50: 'LT', 0x53: 'GE', 0x56: 'LE', 0x72: 'NE',
}

bccIns6502 = ['BNE', 'BEQ', 'BCC', 'BCS', 'BVC', 'BVS', 'BMI', 'BPL']

def zpMode(ins):
  # vCPU instructions that work on zero page
  if ins in ['LD', 'LDW', 'STW', 'ST',
             'INC', 'ADDW', 'SUBW', 'CALL',
             'POKE', 'DOKE', 'ANDW', 'ORW',
             'XORW', 'CMPHS', 'CMPHU']:
    return True
  # v6502 instructions that work on zero page
  if ins[3:] in ['Z', 'ZX', 'ZY', 'IX', 'IY']:
    return True
  # Negative for all others
  return False

zeroPageSyms = {
 0x00: 'zeroConst',
 0x01: 'memSize',
 0x06: 'entropy',
 0x09: 'videoY',
 0x0e: 'frameCount',
 0x0f: 'serialRaw',
 0x11: 'buttonState',
 0x14: 'xoutMask',
 0x16: 'vPC',
 0x17: 'vPC+1',
 0x18: 'vAC',
 0x19: 'vAC+1',
 0x1a: 'vLR',
 0x1b: 'vLR+1',
 0x1c: 'vSP',
 0x21: 'romType',
 0x22: 'sysFn',
 0x23: 'sysFn+1',
 0x24: 'sysArgs+0',
 0x25: 'sysArgs+1',
 0x26: 'sysArgs+2',
 0x27: 'sysArgs+3',
 0x28: 'sysArgs+4',
 0x29: 'sysArgs+5',
 0x2a: 'sysArgs+6',
 0x2b: 'sysArgs+7',
 0x2c: 'soundTimer',
 0x2e: 'ledState_v2',
 0x2f: 'ledTempo',
 0x80: 'oneConst',
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
  cpuType = 0                                   # 0 for vCPU, 1 for v6502
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

        # Decide what instruction set we're in
        ins = None
        if byte in opcodes[cpuType].keys():     # First probe the last ISA
          ins, ops = opcodes[cpuType][byte]
        else:
          xCpu = 1 - cpuType
          if byte in opcodes[xCpu].keys():      # Probe the other if not valid
            cpuType = xCpu                      # Swap on success
            ins, ops = opcodes[cpuType][byte]

        if ins:
          if j > 0:                             # Force new line
            print('%s|%s|' % ((51-j)*' ', text))
            j, text = 0, ''
          asm = '%-5s' % ins                    # Format mnemonic

      else:                                     # Already in instruction
        if len(asm) > 5 and ops == 1:
          asm = asm[:-2] + ('%02x' % byte) + asm[-2:]
        elif zpMode(ins) and ops == 1 and byte in zeroPageSyms:
          if byte == 0x00 and ins[-1] == 'W':
            asm += ' ' + zeroPageSyms[1] + '-1' # Special case top of memory
          else:
            asm += ' ' + zeroPageSyms[byte]     # Known address
        elif ins == 'Bcc' and ops == 2 and byte in bccCodes:
          asm = 'B%-4s' % bccCodes[byte]
        elif ins in ['BRA', 'DEF'] or (ins == 'Bcc' and ops == 1):# Branch vCPU
          asm += ' $%02x%02x' % (hiAddress, (byte+2)&255)
        elif ins in bccIns6502:                 # Relative branch v6502
          to = address + i + 1 + (byte ^ 128) - 128
          asm += ' $%04x' % (to & 0xffff)
        elif ins == 'SYS':                      # Operand to SYS
          if byte != 128:
            maxCycles = 28-2*((byte^128)-128)
            asm += ' %d' % maxCycles            # maxCycles in decimal
          else:
            asm = 'HALT'                        # Never executes
        else:
          asm += ' $%02x' % byte                # Default as hex byte
        ops -= 1

    if j == 0:                                  # Address on each new line
      print('%04x ' % (address + i), end='')

    print(' %02x' % byte,  end='')              # Print byte as hex value
    j += 3

    if j == 8*3:                                # Visual divider
      print(' ', end='')
      j += 1

    text += chr(byte & 127) if 32<=(byte & 127)<127 else '.' # ASCII

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

