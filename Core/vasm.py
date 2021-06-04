# SYNOPSIS: from vasm import *
#
# This is the (back end for) a minimalistic vCPU assembler in Python.
# Use Python itself as the front end.
#
# As always vCPU mmenomnics are upper case
# Special words:
#       ORG(address)    Start new segment
#       L('Name')       Create a label
#       BYTE(byte,...)  Insert data
#       END(address)    Finish assembly, address is execution address

import io
import json
import sys

# The _gt1 variable holds all the segments of the resulting GT1 file.
# It is organized as a list of 4-tuples (one such 4-tuple per segment).
# The format of the 4-tuple is `(start_addr, size, labels, content)`,
# where:
#   - start_addr : is the start address of the segment. This is typically
#                  a number, but it can be None (for floating segments that
#                  will be placed/resolved later via the RESOLVE_SEGMENTS()
#                  function);
#   - size : the size of the segment;
#   - labels : the dictionary of labels defined within the segment;
#              The format of this dictionary is { label : offset, ... },
#              where label is the string of the label, and offset is
#              an integer offset where the label was defined (relative
#              to the start of the segment).
#              Per-segment label/symbol tables are used because of the
#              floating segments that might not yet have fixed locations;
#   - content: the list with the contents of the segment. Each entry
#              is a byte, a string, or a pair. Strings and pairs are
#              resolved by _eval() at the end of the assembly process
#              (when the user invokes END()). Strings are resolved
#              via the _symbols table, and pairs are resolved as follows.
#              When an entry is a pair (say x), the first entry x[0]
#              of the pair is  a callback that will be invoked by _eval()
#              with the argument _eval(x[1]). Note that x[1] itself can
#              be a byte, a string, or a pair. This will allow for
#              recursive evaluations of these pairs.
_gt1 = [(0x200, 0x100, {}, []) ]
# [ start_addr, size, {label : offset, ...}, [byte, ...],
               #   start_addr, size, {label : offset, ...}, [byte, ...], ...]
               # `byte' can be int, symbol string, or expression tuple

_symbols = {}           # name -> value

# Preload system-defined symbols
with open('interface.json') as file:
  for (name, value) in json.load(file).items():
    _symbols[name] = value if isinstance(value, int) else int(value, base=0)

_emit_callback = None

def ORG(addr, size=0x100, callback=None):
  # The callback is invoked before every _emit() within
  # the segment defined by this ORG().
  # The callback should accept two parameters: (gt1, ins)
  # where gt1 is a copy of the current _gt1 and ins is
  # the tuple containing the instructions to be inserted
  # in the contents of the last segment.
  # The callback can be used, for example, to automatically
  # create new segments when current segment is full.
  global _emit_callback
  _gt1.append((addr, size, {}, []))
  _emit_callback = callback

def LDWI(op):  return _emit((0x11, (LO,op), (HI,op)))
def LD(op):    return _emit((0x1a, op))
def LDW(op):   return _emit((0x21, op))
def STW(op):   return _emit((0x2b, op))
def BEQ(op):   return _emit((0x35, 0x3f, (_br,op)))
def BGT(op):   return _emit((0x35, 0x4d, (_br,op)))
def BLT(op):   return _emit((0x35, 0x50, (_br,op)))
def BGE(op):   return _emit((0x35, 0x53, (_br,op)))
def BLE(op):   return _emit((0x35, 0x56, (_br,op)))
def LDI(op):   return _emit((0x59, op))
def ST(op):    return _emit((0x5e, op))
def POP():     return _emit((0x63,))
def BNE(op):   return _emit((0x35, 0x72, (_br,op)))
def PUSH():    return _emit((0x75,))
def LUP(op):   return _emit((0x7f, op))
def ANDI(op):  return _emit((0x82, op))
def ORI(op):   return _emit((0x88, op))
def XORI(op):  return _emit((0x8c, op))
def BRA(op):   return _emit((0x90, (_br,op)))
def INC(op):   return _emit((0x93, op))
def ADDW(op):  return _emit((0x99, op))
def PEEK():    return _emit((0xad,))
def SYS(op):   return _emit((0xb4, 270-op//2 if op>28 else 0))
def SUBW(op):  return _emit((0xb8, op))
def DEF(op):   return _emit((0xcd, (_br,op)))
def CALL(op):  return _emit((0xcf, op))
def ALLOC(op): return _emit((0xdf, op))
def ADDI(op):  return _emit((0xe3, op))
def SUBI(op):  return _emit((0xe6, op))
def LSLW():    return _emit((0xe9,))
def STLW(op):  return _emit((0xec, op))
def LDLW(op):  return _emit((0xee, op))
def POKE(op):  return _emit((0xf0, op))
def DOKE(op):  return _emit((0xf3, op))
def DEEK():    return _emit((0xf6,))
def ANDW(op):  return _emit((0xf8, op))
def ORW(op):   return _emit((0xfa, op))
def XORW(op):  return _emit((0xfc, op))
def RET():     return _emit((0xff,))
def CALLI(op): return _emit((0x85, (LO,op), (HI,op)))
def CMPHS(op): return _emit((0x1f, op))
def CMPHU(op): return _emit((0x97, op))
def BYTE(*op): return _emit(op)

def L(name):
  if name in _symbols:
    ERR('Redefined %s' % repr(name))

  _emit([]) # To call the emit callback before we define labels
            # This is done here to prevent labels from being
            # created at the end of full segments (in case
            # the callback creates a new segment).

  segment = _gt1[-1]
  segment[2][name] = len(segment[3])
  if segment[0] is not None:
    # If the symbol can be resolved now, resolve it already
    _symbols[name] = segment[0] + len(segment[3])
  else:
    _symbols[name] = None

def ALIGN(nbytes=2):
  segment = _gt1[-1]
  addr = segment[0] + len(segment[3])
  rem = addr % nbytes
  if rem != 0:
    _emit(tuple([0] * (nbytes - rem)))

def RESOLVE_SEGMENTS(callback):
  # The RESOLVE_SEGMENTS() function is mainly used to resolve
  # the `floating` segments, i.e. segments without a defined start address.
  # The callback is then responsible for modifying _gt1 to place and
  # resolve these segments.
  return callback(_gt1, _symbols)

def END(start=0x200, filename='out.gt1', resolve_callback=None):
  if resolve_callback is not None:
    RESOLVE_SEGMENTS(resolve_callback)
  if filename is not None:
    _f = open(filename, 'wb')
  else:
    _f = io.BytesIO()
  with _f as f:
    for segment in _gt1:
      address, size, labels, contents = segment
      if len(contents) > 0:
        if len(contents) > size:
          ERR('Segment too large at 0x%04X' % address)
        if address + len(contents) > (address | 255) + 1:
          ERR('Page overrun in segment 0x%04X' % address)
        resolved = [_byte(_eval(x)) for x in contents]
        f.write(bytes([address >> 8, address & 255, len(resolved) & 255]))
        f.write(bytes(resolved))
    start = _eval(start)
    f.write(bytes([0, start >> 8, start & 255]))
    if filename is None:
        return f.getvalue()

def _emit(ins):
  if _emit_callback is not None:
    _emit_callback(_gt1, ins)
  segment = _gt1[-1]
  segment[3].extend(ins)
  return 0

def LO(x): return _eval(x) & 255        # Low byte of word
def HI(x): return _eval(x) >> 8         # High byte of word
def ADDR(x): return _eval(x)            # The address of the label
def _br(x): return (_eval(x) - 2) & 255 # Adjust for pre-increment of vPC

def _eval(x):
  fn = lambda x: x                      # No operation
  if isinstance(x, tuple):              # Tuple expressions
    fn, x = x[0], _eval(x[1])
  if isinstance(x, str):                # Resolve symbol strings
    x = _symbols[x] if x in _symbols else ERR('Undefined %s' % repr(x))
  return fn(x)

def _byte(x):
  if x < -128 or x > 255:
    ERR('Out of byte range: %s' % repr(x))
  return x & 255

def ERR(*args):
  line = 'Error: ' + ' '.join(args)
  print('\033[1m' + line + '\033[0m' if sys.stdout.isatty() else line)
  sys.exit(1)
