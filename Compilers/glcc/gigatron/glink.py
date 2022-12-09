#!/usr/bin/env python3

#   Copyright (c) 2021, LB3361
#
#    Redistribution and use in source and binary forms, with or
#    without modification, are permitted provided that the following
#    conditions are met:
#
#    1.  Redistributions of source code must retain the above copyright
#        notice, this list of conditions and the following disclaimer.
#
#    2. Redistributions in binary form must reproduce the above
#       copyright notice, this list of conditions and the following
#       disclaimer in the documentation and/or other materials
#       provided with the distribution.
#
#    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
#    CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,
#    INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
#    MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
#    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
#    BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
#    EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
#    TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
#    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
#    ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
#    OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
#    OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
#    POSSIBILITY OF SUCH DAMAGE.


# -------------- glink proper

import argparse, json, string, functools, fnmatch
import os, sys, traceback, copy, builtins
import builtins
import glccver

args = None
rominfo = None
romtype = None
romcpu = None
lccdir = '/usr/local/lib/gigatron-lcc'
module_dict = {}
module_builtins = {}
module_list = []
segment_list = []
new_modules = []

symdefs = {}
exporters = {}

the_module = None
the_segment = None
the_fragment = None
the_pc = 0
the_pass = 0

final_pass = False
hops_enabled = False
short_function = False
lbranch_counter = 0
error_counter = 0
warning_counter = 0
genlabel_counter = 0
labelchange_counter = 1
dedup_errors = set()

map_modules = None
map_libraries = None
map_segments = None
map_describe = None
map_place = None

zpsize = 0

# --------------- utils

def debug(s, level=1):
    if args.d and args.d >= level:
        print("(glink debug) " + s, file=sys.stderr)

def where(exc=False):
    '''Locate error in a .s/.o/.a file'''
    if exc:
        stb = traceback.extract_tb(sys.exc_info()[2], limit=10)
    else:
        stb = traceback.extract_stack(limit=10)
    for s in stb:
        if isinstance(s[0],str) and not s[0].endswith('.py') \
           and not s[0].endswith('glink') \
           and not s[0].endswith('glcc'):
            fn = s[0] or "<unknown>"
            if fn.startswith(lccdir):
                fn = fn[len(lccdir):].lstrip('/')
            return f"{fn}:{s[1]}"
    return None

class __metaUnk(type):
    wrapped = ''' __abs__ __add__ __and__ __floordiv__ __ge__ __gt__ __invert__
    __le__ __lshift__ __lt__ __mod__ __mul__ __neg__ __or__ __pos__
    __pow__ __radd__ __rand__ __rfloordiv__ __rlshift__ __rmod__
    __rmul__ __ror__ __rpow__ __rrshift__ __rshift__ __rsub__
    __rtruediv__ __rxor__ __sub__ __truediv__ __xor__'''
    def __new__(cls, name, bases, namespace, **kwargs):
        def wrap(f):
            @functools.wraps(f)
            def wrapper(self, *args):
                return Unk(f(int(self), *map(int, args)))
            return wrapper if f else None
        for m in cls.wrapped.split():
            namespace[m] = wrap(getattr(int, m))
        return type(name, bases, namespace)

class Unk(int, metaclass=__metaUnk):
    '''Class to represent unknown symbol values'''
    __slots__= ()
    def __new__(cls,val):
        return int.__new__(cls,val)
    def __repr__(self):
        return f"Unk({hex(int(self))})"
    def __eq__(self, other):
        return False
    def __ne__(self, other):
        return True

def is_zero(x):
    if isinstance(x,int) and not isinstance(x,Unk):
        return int(x) == 0
    return False

def is_zeropage(x, l = 0):
    if isinstance(x,int) and not isinstance(x,Unk):
        if int(x) & 0xff00 == 0:
            return l < 1 or int(x + l) & 0xff00 == 0
    return False

def is_not_zeropage(x):
    if isinstance(x,int) and not isinstance(x,Unk):
        return int(x) & 0xff00 != 0
    return False

def is_pcpage(x):
    if isinstance(x,int) and not isinstance(x,Unk):
        return int(x) & 0xff00 == pc() & 0xff00
    return False

def is_not_pcpage(x):
    if isinstance(x,int) and not isinstance(x,Unk):
        return int(x) & 0xff00 != pc() & 0xff00
    return False

def check_zp(x):
    x = v(x)
    if final_pass and is_not_zeropage(x):
        warning(f"zero page address overflow")
    return x & 0xff

def check_imm8(x):
    x = v(x)
    if final_pass and isinstance(x,int):
        if x < 0 or x > 255:
            warning(f"immediate byte argument overflow")
    return x & 0xff

def check_im8s(x):
    x = v(x)
    if final_pass and isinstance(x,int):
        if x < -128 or x > 255:
            warning(f"immediate byte argument overflow")
    return x & 0xff

def check_br(x):
    x = v(x)
    if is_not_pcpage(x) and final_pass:
        error(f"short branch overflow")
    return (int(x)-2) & 0xff

def check_cpu(v):
    if args.cpu < v and final_pass:
        stb = traceback.extract_stack(limit=3)
        name = stb[1].name
        if name != name.upper():
            name = stb[0].name
        error(f"opcode {name} not implemented by cpu={args.cpu}", dedup=True)

def resolve(s, ignore=None):
    '''Resolve a global symbol and return its value or None.
       Symbols named '__glink_weak_xxx' are weak references to 'xxx'.
       They resolve to the value of 'xxx' if 'xxx' is defined,
       and to zero otherwise.'''
    if s in exporters:
        exporter = exporters[s]
        if exporter != ignore:
            if s in exporter.symdefs:
                return exporter.symdefs[s]
            elif final_pass:
                error(f"module {exporter.fname} exports '{s}' but does not define it", dedup=True)
    if s in symdefs:
        return symdefs[s]
    if s.startswith('__glink_weak_'):
        return resolve(s[13:]) or 0
    return None

class Fragment:
    "Class for representing the code/data fragments in a module"
    __slots__ = ('segment', 'name','func', 'size', 'align', 'nohop', 'amin', 'amax')
    def __init__(self, segment, name, func, size = None, align = None):
        self.segment = segment     # CODE, DATA, BSS, COMMON
        self.name = name           # fragment name
        self.func = func           # fragment code
        self.size = size           # fragment size (data)
        self.align = align         # fragment alignment (data)
        self.nohop = False         # short function
        self.amin = None           # min address range
        self.amax = None           # max address range
    def __repr__(self):
        return f"Fragment({self.segment},'{self.name}',...)"

class Module:
    '''Class for assembly modules read from .s/.o/.a files.'''
    def __init__(self, name=None, cpu=None, code=None):
        global args, current_module
        self.cpu = cpu if cpu != None else args.cpu
        self.code = []
        self.name = name
        self.fname = name
        self.library = False
        self.used = False
        self.exports = []
        self.imports = []
        self.cimports = []
        self.symrefs = {}
        self.symdefs = {}
        self.sympass = {}
        # inner function to process a placement fragment
        def placement(tp):
            matches = [f for f in self.code if fnmatch.fnmatchcase(f.name, tp[1])]
            for match in matches:
                if tp[0] == 'NOHOP':
                    match.nohop = True
                elif match.amin == None:
                    match.amax = tp[3] if len(tp) > 3 else None
                    match.amin = tp[2]
                elif len(matches) < 2:
                    error(f"Conflicting placement constraints {tp}")
            return len(matches)
        # process code list
        for tp in code:
            if tp[0] == 'EXPORT':
                self.exports.append(tp[1])                    # ('EXPORT', "symbolname")
            elif tp[0] == 'IMPORT' and len(tp) == 2:
                self.imports.append(tp[1])                    # ('IMPORT', "symbolname")
            elif tp[0] == 'IMPORT' and len(tp) > 3 and tp[2] == 'IF':
                self.cimports.append(tp)                      # ('IMPORT', "symbolname", 'IF', ...)
            elif tp[0] == 'IMPORT' and len(tp) == 4 and tp[2] == 'AT':
                self.symdefs[tp[1]] = int(tp[3])              # ('IMPORT', "symbolname", 'AT', address)
            elif tp[0] == 'CODE':
                self.code.append(Fragment(*tp))               # ('CODE', "name", func)
            elif tp[0] == 'DATA' or tp[0] == 'BSS' or tp[0] == 'COMMON':
                self.code.append(Fragment(*tp))               # ('DATA|BSS|COMMON', "name", func, size, align)
            elif tp[0] in ('ORG','PLACE','NOHOP'):            # ('PLACE', "pattern", minaddr, maxaddr)
                if placement(tp) < 1:                         # ('ORG', "pattern", addr)
                    error(f"Cannot locate fragment for {tp}") # ('NOHOP', "pattern")
            elif tp[0] != 'NOP':                              # ('NOP',)
                error(f"Unrecognized fragment specification {tp}")
        # placement overlay
        if map_place:
            fragnames = [f.name for f in self.code]
            for tp in map_place(self.name, fragnames) or []:
                if tp[0] in ('ORG', 'PLACE', 'NOHOP'):
                    n = placement(tp)
                    if n == 0:
                        warning(f"map_place directive {tp} does not match any fragment");
                    elif args.d > 0:
                        debug(f"map_place directive {tp} matches {n} fragment(s)")
                elif tp[0] != 'NOP':
                    error(f"Unrecognized map_place() specification {tp}")
    def __repr__(self):
        return f"Module('{self.fname or self.name}',...)"
    def label(self, sym, val):
        '''Define a label within a module.
           Increment counter when label value has changed relative to the previous pass.'''
        if the_pass > 0:
            if sym in self.symdefs and val == self.symdefs[sym]:
                self.sympass[sym] = the_pass
            elif sym in self.symdefs and self.sympass[sym] == the_pass:
                error(f"multiple definitions of label '{sym}'", dedup=True)
            else:
                if sym in self.symdefs and args.d >= 3:
                    debug(f"pass {the_pass}: symbol '{sym}' went from {hex(self.symdefs[sym])} to {hex(val)}")
                global labelchange_counter
                labelchange_counter += 1
                self.symdefs[sym] = val
                self.sympass[sym] = the_pass

class Segment:
    '''Represent memory segments to be populated with code/data'''
    __slots__ = ('saddr', 'eaddr', 'pc', 'flags', 'buffer', 'nbss')
    def __init__(self, saddr, eaddr, flags=False):
        self.saddr = saddr
        self.eaddr = eaddr
        self.pc = saddr
        self.flags = flags or False # 0x1: no code, 0x2 : no data, 0x4 : no heap
        self.buffer = None
        self.nbss = None
    def __repr__(self):
        d = f",flags={hex(self.flags)}" if self.flags else ''
        return f"Segment({hex(self.saddr)},{hex(self.eaddr)}{d})"

def emit(*args):
    global final_pass, the_pc, the_segment
    if final_pass:
        if not the_segment.buffer:
            the_segment.buffer = bytearray()
        for b in args:
            the_segment.buffer.append(b)
    the_pc += len(args)

def extern(sym):
    '''Adds a symbol to the import list of a module.
       This happens when `measure_code_fragment' is called.
       Pseudo-instructons need this to make sure the linker
       inserts the appropriate runtime routines.'''
    if the_pass == 0 and sym not in the_module.imports:
        the_module.imports.append(sym)

fraginfo = {}
addrinfo = {}

def record_fragment_address(addr):
    # record fragment addresses for option --frags
    global fraginfo, addrinfo
    if args.fragments and final_pass:
        fid = id(the_fragment)
        finfo = fraginfo.get(fid, (0,) )
        if len(finfo) < 2:
            fraginfo[fid] = (finfo[0] + 1, addr)
        else:
            addrinfo[(finfo[1],addr)] = (finfo[0], the_fragment, the_module)
            fraginfo[fid] = (finfo[0],)


# ------------- jumps and hops

def bytes_left():
    '''Tells how many bytes are left in the current segment'''
    if the_pass > 0:
        return the_segment.eaddr - the_pc
    return 256

def size_long_jump():
    return 3 if args.cpu >= 5 else 10

def emit_long_jump(d):
    if args.cpu >= 5:
        CALLI(d)          # 3 bytes
    else:
        STLW(-2); LDWI(d); STW(vLR);
        LDLW(-2); RET()   # <10 bytes

def hop(sz, jump):
    '''Ensure, possibly with a hop, that there are at
       least sz bytes left in the segment. '''
    if bytes_left() < sz:
        global hops_enabled
        if not hops_enabled:
            error(f"internal error: cannot honor hop({sz}) because hops are disabled")
        elif jump and bytes_left() < size_long_jump():
            error(f"internal error: cannot hop because there is no space for a jump")
        else:
            global the_segment, the_pc
            hops_enabled = False
            the_segment.pc = the_pc
            lfss = args.lfss or 32
            ns = find_code_segment(max(lfss, sz))
            if not ns:
                fatal(f"map memory exhausted while fitting function `{the_fragment.name}'")
            if jump:
                emit_long_jump(ns.pc)
            hops_enabled = True
            the_segment.pc = the_pc
            if args.fragments and final_pass:
                record_fragment_address(the_pc)
                record_fragment_address(ns.pc)
            the_segment = ns
            the_pc = ns.pc
            if args.d >= 2 or final_pass:
                debug(f"- continuing code fragment '{the_fragment.name}' at {hex(the_pc)} in {ns}")

def emitjump(d):
    global hops_enabled, lbranch_counter
    save_hops_enabled = hops_enabled
    hops_enabled = False
    if short_function or is_pcpage(d):
        BRA(d)
    else:
        lbranch_counter += size_long_jump() - 2
        emit_long_jump(d)
    hops_enabled = save_hops_enabled
    tryhop(jump=False)

def emitjcc(BCC, BNCC, JCC, d):
    global hops_enabled, lbranch_counter
    save_hops_enabled = hops_enabled
    hops_enabled = False
    short_ok = bytes_left() >= 3 + size_long_jump()
    long_ok = bytes_left() >= 3 + 2 * size_long_jump()
    lbl = genlabel()
    while True:
        if short_function:
            BCC(d)
            break;
        elif is_pcpage(d) and short_ok:
            BCC(d)
            break;
        elif args.cpu >= 6 and JCC and short_ok:
            JCC(d)
            break;
        elif not is_pcpage(d) and long_ok:
            BNCC(lbl)
            emit_long_jump(d)
            label(lbl, hop=0)
            lbranch_counter += size_long_jump()
            break;
        else:
            hops_enabled = True
            hop(3 + 2 * size_long_jump(), jump=True)
            short_ok = long_ok = True
            hops_enabled = False
    hops_enabled = save_hops_enabled

# ------------- opcode helpers

def emit_op(*args):
    '''Calls emits with strings replaced by opcodes according to interface.json.
       This displaces the knowledge of the correct opcodes into inteface[-dev].json
       but one still has to provide the right arguments.'''
    bytes=[]
    for arg in args:
        if not isinstance(arg, str):
            bytes.append(arg)
        elif not arg in symdefs:
            error(f"emit_op: opcode {arg} not defined in interface.json")
        else:
            if arg[-3:-1] == "_v":
                check_cpu(int(arg[-1:]))
            op = symdefs[arg]
            oq = op >> 8
            if oq == 0x35:
                bytes.append(oq)
            elif oq != 0x3:
                error(f"emit_op: cannot emit opcode {arg} defined as {hex(op)}")
            bytes.append(op & 0xff)
    tryhop(len(bytes))
    emit(*bytes)


# ------------- mulq

mulq_map = {}

def create_mulq_map():
    global mulq_map
    if args.cpu >= 7:
        mulq_n = {}
        for kk in range(256):
            p = c = 2
            k = kk
            while k != 0:
                if k & 0x80 == 0x80:
                    p = p << 1; c += 1; k <<= 1; k &= 0xff
                elif k & 0xc0 == 0x40:
                    p += 1; c += 1; k <<= 2; k &= 0xff
                else:
                    p -= 1; c += 1; k <<= 3; k &= 0xff
            if not p in mulq_n or mulq_n[p] >= c:
                mulq_n[p] = c
                mulq_map[p] = kk
    mulq_map[0] = lambda: LDI(0)
    mulq_map[1] = lambda: None
    mulq_map[2] = lambda: LSLW()
    mulq_map[4] = lambda: LSLW(repeat=2)


# ------------- map of page zero

zpage_map = [ None for i in range(0,256) ]

def zpage_reserve(rng, lbl, error_on_conflict=True):
    for i in rng:
        if i < 0 or i >= 256:
            fatal(f"Cannot reserve address {hex(i)} in page zero")
        elif zpage_map[i] and zpage_map[i] != lbl:
            if error_on_conflict:
                fatal(f"Zero page address {hex(i)} is both {zpage_map[i]} and {lbl}")
        else:
            zpage_map[i] = lbl

def zpage_alloc(sz, label, fromAddr=0):
    for i in range(fromAddr, 256):
        if zpage_map[i:i+sz].count(None) == sz:
            zpage_reserve(range(i, i+sz), label)
            return i

def create_zpage_map():
    zpage_reserve(range(0xd0,0x100), "STACK")
    zpage_reserve(range(0,0x30), "V4")
    zpage_reserve(range(0x80,0x81), "V4")
    if args.cpu < 7:
        zpage_reserve(range(0x30,0x42), "LOADER")
    elif args.cpu >= 5:
        zpage_reserve(range(0x30,0x36), "VIRQ")
    if args.cpu == 6:
        zpage_reserve(range(0xc0,0xd0), "VX0")
    if args.cpu >= 7:
        zpage_reserve(range(0x81,0x8c), "V7")

def create_zpage_segments():
    segs = []
    last = None
    for i in range(256):
        if last and zpage_map[i]:
            segs.append(Segment(last, i-1, 7))
            last = None
        elif not last and not zpage_map[i]:
            last = i
    return segs

# ------------- usable vocabulary for .s/.o/.a files

# Each .s/.o/.a files is exec with a fresh global dictionary and a
# restricted set of builtins.  This prevents a module from
# accidentally changing the global state in other ways than defining
# new modules.  Note that this is not expected to protect against
# malicious modules, just prevent accidental corruption.

module_builtins_okay = '''None True False abs all any ascii bin bool chr
dict divmod enumerate filter float format frozenset getattr hasattr hash
hex id int isinstance issubclass iter len list map max min next oct ord
pow print property range repr reversed set setattr slice sorted str sum
tuple type zip'''
module_builtins = {}
for s in module_builtins_okay.split():
    if isinstance(__builtins__, dict) and s in __builtins__:
        module_builtins[s] = __builtins__[s]
    elif hasattr(__builtins__,s):
        module_builtins[s] = getattr(__builtins__, s)

def create_register_names(base):
    d = { "vPC":  0x0016, "vAC":  0x0018,
          "vACL": 0x0018, "vACH": 0x0019,
          "vLR":  0x001a, "vSP":  0x001c,
          "FAC":  0xFACFACFAC }
    # ROM-dependent registers
    t0t1 = t2t3 = b0b1 = flac = rsp = None
    if 'registerFLAC' in rominfo:
        flac = int(str(rominfo['registerFLAC']),0)
        zpReserve(flac,flac+6,"REGS:FLAC")
    elif args.cpu >= 7:
        flac = symdefs['vFAS_v7']
    if 'registerT2T3' in rominfo:
        t2t3 = int(str(rominfo['registerT2T3']),0)
        zpReserve(t2t3,t2t3+3,"REGS:T2T3")
    elif args.cpu >= 7:
        t2t3 = symdefs['vT2_v7']
    if 'registerB0B1' in rominfo:
        b0b1 = int(str(rominfo['registerB0B1']),0)
        zpReserve(b0b1,b0b1+1,"REGS:B0B1")
    if 'registerTOT1' in rominfo:
        t0t1 = int(str(rominfo['registerT0T1']),0)
        zpReserve(t0t1,t0t1+3,"REGS:T0T1")
    if 'registerSP' in rominfo:
        assert args.cpu < 7
        rsp  = int(str(rominfo['registerSP']),0)
        zpReserve(rsp,rsp+1,"REGS:SP")
    elif args.cpu >= 7:
        rsp = d['vSP'] ## USE 16BITS STACK
    flac = flac or zpage_alloc(7,"REGS:FLAC", 0x80)
    t0t1 = t0t1 or symdefs['sysArgs0']
    t2t3 = t2t3 or zpage_alloc(4,"REGS:T2T3", 0x80)
    b0b1 = b0b1 or zpage_alloc(2,"REGS:B0B1", 0x80)
    rsp  = rsp or zpage_alloc(2,"REGS:SP", 0x80)
    # GLCC registers
    if base == None and 'registerBase' in rominfo:
        base = int(str(rominfo['registerBase']),0)
    if base == None:
        base = zpage_alloc(0x30, "REGS:R0-23", 0x90)
    elif base < 0 or base + 0x30 > 0x100:
        fatal(f"Illegal register location {hex(base)}-{hex(base+0x2f)}.")
    else:
        zpage_reserve(range(base,base+0x30), "REGS:R0-23")
    for i in range(0,24): d[f'R{i}'] = base + i + i
    for i in range(0,23): d[f'L{i}'] = d[f'R{i}']
    for i in range(0,22): d[f'F{i}'] = d[f'R{i}']
    rsp = rsp or d['R23']
    debug(f"Registers: base:{hex(base)} T01:{hex(t0t1)} T23:{hex(t2t3)}")
    debug(f"Registers: LAC:{hex(flac+3)} B012:{hex(b0b1)} SP:{hex(rsp)}")
    d.update({'T0':t0t1, 'T1':t0t1+2, 'T2':t2t3, 'T3':t2t3+2,
              'B0':b0b1, 'B1':b0b1+1, 'LAX':flac+2, 'LAC':flac+3,
              'FAS':flac, 'FAE':flac+1, 'SP':rsp })
    # Publish register names
    for (k,v) in d.items():
        module_dict[k] = v
        globals()[k] = v

def new_globals():
    '''Return a pristine global symbol table to read .s/.o/.a files.'''
    global module_dict
    g = module_dict.copy()
    g['args'] = copy.copy(args)
    g['rominfo'] = copy.copy(rominfo)
    g['__builtins__'] = module_builtins.copy()
    return g

def vasm(func):
    '''Decorator to mark functions usable in .s/.o/.a files'''
    module_dict[func.__name__] = func
    return func

@vasm
def error(s, dedup=False):
    global the_pass, final_pass, error_counter
    dedup = dedup or final_pass
    if the_pass == 0 or final_pass:
        if dedup and s in dedup_errors: return
        dedup_errors.add(s)
        error_counter += 1
        w = where()
        w = "" if w == None else w + ": "
        print(f"glink: {w}error: {s}", file=sys.stderr)
@vasm
def warning(s, dedup=False):
    global the_pass, final_pass, warning_counter
    dedup = dedup or final_pass
    if the_pass == 0 or final_pass:
        if dedup and s in dedup_errors: return
        dedup_errors.add(s)
        warning_counter += 1
        w = where()
        w = "" if w == None else w + ": "
        print(f"glink: {w}warning: {s}", file=sys.stderr)
@vasm
def fatal(s, exc=False):
    w = where(exc)
    w = "" if w == None else w + ": "
    print(f"glink: {w}fatal error: {s}", file=sys.stderr)
    sys.exit(1)

@vasm
def module(code=None,name=None,cpu=None):
    '''Called from .s/.o/.a files to declare a module.
       This should be the only way for a .s/.o/.a file
       to change the linker state.'''
    if not name:
        name = "[unknown]"
        tb = traceback.extract_stack(limit=2)
        if len(tb) > 1 and isinstance(tb[0][0], str):
            name = os.path.basename(tb[0][0])
    global new_modules
    if the_module or the_fragment:
        warning("module() should not be called from a code fragment")
    else:
        new_modules.append(Module(name,cpu,code))

@vasm
def genlabel():
    '''Generate a label for use in a pseudo-instruction.
       One should make sure to request the same number
       of labels regardless of the code path.'''
    global genlabel_counter
    genlabel_counter += 1
    return f".LL{genlabel_counter}"

@vasm
def zpReserve(addr0,addr1,lbl):
    if the_pass == 0:
        rng = range(addr0, addr1+1)
        zpage_reserve(rng, lbl, error_on_conflict=False)
@vasm
def pc():
    return the_pc
@vasm
def v(x):
    '''Possible resolve symbol `x'.'''
    if not isinstance(x,str):
        return x
    if the_module:
        the_module.symrefs[x] = the_pass
        if x in the_module.symdefs:
            return the_module.symdefs[x]
    r = resolve(x)
    if final_pass and r == None:
        error(f"undefined symbol '{x}'", dedup=True)
    return Unk(0xDEAD) if r == None else r
@vasm
def lo(x):
    return v(x) & 0xff
@vasm
def hi(x):
    return (v(x) >> 8) & 0xff

@vasm
def org(addr1, addr2=None):
    '''Force a code fragment to be placed at a specific location.
       The fragment must fit in the page and the required space
       must be available. This currently piggybacks on nohop()
       and does not work for code fragments. I have to find
       a better way of doing this.'''
    global short_function
    # this information is collected in measure_code_fragment()
    if the_pass == 0:
        the_fragment.nohop = True
        the_fragment.amin = int(addr1)
        the_fragment.amax = int(addr2) if addr2 else None
@vasm
def nohop():
    '''Force a code fragment to be fit in a single page.
       An error will be signaled if no page can fit it.'''
    global short_function
    # this information is collected in measure_code_fragment()
    if the_pass == 0:
        the_fragment.nohop = True
@vasm
def tryhop(sz=None, jump=True):
    '''Hops to a new page if the current page cannot hold a long jump
       plus `sz' bytes. This also ensures that no hop will occur during
       the next `sz' bytes. A long jump is generated when `jump' is True.'''
    if hops_enabled:
        sz = sz if sz != None else 4 # max size of an instruction
        sz = sz + size_long_jump()
        if bytes_left() < sz:
            hop(sz, jump=jump)
@vasm
def align(d):
    while the_pc & (d-1):
        emit(0)
@vasm
def bytes(*args):
    for w in args:
        w = v(w)
        if isinstance(w,int):
            emit(v(w))
        elif isinstance(w,builtins.bytes) or isinstance(w,bytearray):
            for b in w:
                emit(b)
        else:
            error(f"Illegal argument {w} for instruction 'bytes()'")
@vasm
def words(*args):
    for w in args:
        emit(lo(w), hi(w))
@vasm
def space(d):
    for i in range(0,d):
        emit(0)
@vasm
def label(sym, val=None, hop=None):
    '''Define label `sym' to the value of PC or to `val'.
       This function tries to be smart about locating the
       label before or after a page hop. Nothing happens
       if hop==0 or if the label has already been referenced
       in the current module (forward branch). However,
       if the label has not yet been referenced (backward branch),
       this function tries to hop to a new page before
       defining the label. Argument 'hop' then indicates
       how many instruction bytes after the label should
       be available in the new page. '''
    if the_pass > 0:
        referenced = False
        if sym in the_module.symrefs:
            referenced = (the_module.symrefs[sym] == the_pass)
        if hop != 0 and not referenced:
            tryhop(hop or 12)
        the_module.label(sym, v(val) if val != None else the_pc)

@vasm
def ST(d):
    emit_op("ST", check_zp(d))
@vasm
def STW(d):
    emit_op("STW", check_zp(d))
@vasm
def STLW(d):
    emit_op("STLW", check_im8s(d))
@vasm
def LD(d):
    emit_op("LD", check_zp(d))
@vasm
def LDI(d, hop=True):
    emit_op("LDI", check_im8s(d))
@vasm
def LDWI(d):
    d=int(v(d)); emit_op("LDWI", lo(d), hi(d))
@vasm
def LDW(d):
    emit_op("LDW", check_zp(d))
@vasm
def LDLW(d):
    emit_op("LDLW", check_im8s(d))
@vasm
def ADDW(d):
    emit_op("ADDW", check_zp(d))
@vasm
def SUBW(d):
    emit_op("SUBW", check_zp(d))
@vasm
def ADDI(d):
    emit_op("ADDI", check_imm8(d))
@vasm
def SUBI(d):
    emit_op("SUBI", check_imm8(d))
@vasm
def LSLW(repeat = 1):
    for _ in range(repeat): emit_op("LSLW")
@vasm
def INC(d):
    emit_op("INC", check_zp(d))
@vasm
def ANDI(d):
    emit_op("ANDI", check_imm8(d))
@vasm
def ANDW(d):
    emit_op("ANDW", check_zp(d))
@vasm
def ORI(d):
    emit_op("ORI", check_imm8(d))
@vasm
def ORW(d):
    emit_op("ORW", check_zp(d))
@vasm
def XORI(d):
    emit_op("XORI", check_imm8(d))
@vasm
def XORW(d):
    emit_op("XORW", check_zp(d))
@vasm
def PEEK():
    emit_op("PEEK")
@vasm
def DEEK():
    emit_op("DEEK")
@vasm
def POKE(d):
    emit_op("POKE", check_zp(d))
@vasm
def DOKE(d):
    emit_op("DOKE", check_zp(d))
@vasm
def LUP(d):
    emit_op("LUP", check_zp(d))
@vasm
def BRA(d):
    emit_op("BRA", check_br(d))
    tryhop(jump=False)
@vasm
def BEQ(d):
    emit_op("BCC", "EQ", check_br(d))
@vasm
def BNE(d):
    emit_op("BCC", "NE", check_br(d))
@vasm
def BLT(d):
    emit_op("BCC", "LT", check_br(d))
@vasm
def BGT(d):
    emit_op("BCC", "GT", check_br(d))
@vasm
def BLE(d):
    emit_op("BCC", "LE", check_br(d))
@vasm
def BGE(d):
    emit_op("BCC", "GE", check_br(d))
@vasm
def CALL(d):
    emit_op("CALL", check_zp(d))
@vasm
def RET():
    emit_op("RET"); tryhop(jump=False)
@vasm
def PUSH():
    emit_op("PUSH")
@vasm
def POP():
    emit_op("POP")
@vasm
def ALLOC(d):
    emit_op("ALLOC", check_im8s(d))
@vasm
def SYS(op):
    op = v(op)
    if not isinstance(op ,Unk):
        if op & 1 != 0 or op < 0 or op >= 284:
            error(f"illegal argument {op} for SYS opcode")
        # If maxTicks is not defined, use 14.
        # All ROMs should ensure that 14 works.
        maxTicks = 14
        if 'maxTicks' in rominfo:
            maxTicks = int(rominfo['maxTicks'])
        op = min(0, maxTicks - op // 2) & 0xff
    emit_op("SYS", op)
@vasm
def HALT():
    tryhop(2); emit(0xb4, 0x80); tryhop(jump = False)
@vasm
def DEF(d):
    emit_op("DEF", check_br(d))

# cpu 5 opcodes
@vasm
def CALLI(d):
    d=int(v(d)); emit_op("CALLI_v5", lo(d), hi(d))
@vasm
def CMPHS(d):
    if args.cpu == 6:
        tryhop(3); emit(0x2f, check_zp(d), 0x37)
    else:
        emit_op("CMPHS_v5", check_zp(d))
@vasm
def CMPHU(d):
    if args.cpu == 6:
        tryhop(3); emit(0x2f, check_zp(d), 0x3a)
    else:
        emit_op("CMPHU_v5", check_zp(d))

# cpu 7 opcodes (with cpu 6 aliases when known)
@vasm
def MOVQB(imm,d):
    if args.cpu == 6:
        tryhop(3);emit(0x16, check_zp(imm), check_zp(d))
    else:
        emit_op("MOVQB_v7", check_zp(d), check_zp(imm))
@vasm
def MOVQW(imm,d):
    if args.cpu == 6:
        tryhop(3);emit(0x4d, check_zp(imm), check_zp(d))
    else:
        emit_op("MOVQW_v7", check_zp(d), check_zp(imm))
@vasm
def POKEQ(d):
    if args.cpu == 6:
        tryhop(2);emit(0x25, check_zp(d)) # aka POKEI
    else:
        emit_op("POKEQ_v7", check_zp(d))
@vasm
def DOKEQ(d):
    emit_op("DOKEQ_v7", check_zp(d))
@vasm
def DOKEI(d):
    d = int(v(d))
    if args.cpu == 6:
        tryhop(3);emit(0x77, (d>>8)&0xff, d&0xff)
    else:
        emit_op("DOKEI_v7", (d>>8)&0xff, d&0xff)
@vasm
def POKEA(d):
    if args.cpu == 6:
        tryhop(2);emit(0x69, check_zp(d))
    else:
        emit_op("POKEA_v7", check_zp(d))
@vasm
def DOKEA(d):
    if args.cpu == 6:
        tryhop(2);emit(0x7d, check_zp(d))
    else:
        emit_op("DOKEA_v7", check_zp(d))
@vasm
def DEEKV(d):
    if args.cpu == 6:
        tryhop(2);emit(0x3b, check_zp(d))
    else:
        emit_op("DEEKV_v7", check_zp(d))
@vasm
def PEEKV(d):
    if args.cpu == 6:
        tryhop(2);emit(0x5b, check_zp(d))
    else:
        emit_op("PEEKV_v7", check_zp(d))
@vasm
def DEEKA(d):
    if args.cpu == 6:
        tryhop(2);emit(0x6f, check_zp(d))
    else:
        emit_op("DEEKA_v7", check_zp(d))
@vasm
def PEEKA(d):
    if args.cpu == 6:
        tryhop(2);emit(0x67, check_zp(d))
    else:
        emit_op("PEEKA_v7", check_zp(d))
@vasm
def LDNI(d):
    if args.cpu == 6:
        tryhop(2);emit(0x9c, check_zp(-d))
    else:
        emit_op("LDNI_v7", check_zp(d ^ 0xff00))
@vasm
def JEQ(d):
    tryhop(3); d=int(v(d));
    if args.cpu == 6:
        emit(0xbb, lo(d-2), hi(d))
    else:
        emit_op("JEQ_v7", lo(d-2), hi(d))
@vasm
def CMPWS(d):
    emit_op("CMPWS_v7", check_zp(d))
@vasm
def CMPWU(d):
    emit_op("CMPWU_v7", check_zp(d))
@vasm
def CMPIS(d):
    emit_op("CMPIS_v7", check_zp(d))
@vasm
def CMPIU(d):
    emit_op("CMPIU_v7", check_zp(d))
@vasm
def JNE(d):
    tryhop(3); d=int(v(d));
    if args.cpu == 6:
        emit(0xbd, lo(d-2), hi(d))
    else:
        emit_op("JNE_v7", lo(d-2), hi(d))
@vasm
def JLT(d):
    tryhop(3); d=int(v(d));
    if args.cpu == 6:
        emit(0xbf, lo(d-2), hi(d))
    else:
        emit_op("JLT_v7", lo(d-2), hi(d))
@vasm
def JGT(d):
    tryhop(3); d=int(v(d));
    if args.cpu == 6:
        emit(0xc1, lo(d-2), hi(d))
    else:
        emit_op("JGT_v7", lo(d-2), hi(d))
@vasm
def JLE(d):
    tryhop(3); d=int(v(d));
    if args.cpu == 6:
        emit(0xc3, lo(d-2), hi(d))
    else:
        emit_op("JLE_v7", lo(d-2), hi(d))
@vasm
def JGE(d):
    tryhop(3); d=int(v(d));
    if args.cpu == 6:
        emit(0xc5, lo(d-2), hi(d))
    else:
        emit_op("JGE_v7", lo(d-2), hi(d))
@vasm
def MOVIW(d,x):
    d=int(v(d))
    emit_op('MOVIW_v7', check_zp(x), hi(d), lo(d))
@vasm
def MULQ(kod):
    emit_op('MULQ_v7', check_zp(kod))
@vasm
def MOVL(s,d):
    if args.cpu == 6:
        tryhop(4);emit(0xc7, check_zp(d), 0xcd, check_zp(s))
    else:
        emit_op("MOVL_v7", check_zp(d), check_zp(s))
@vasm
def MOVF(s,d):
    if args.cpu == 6:
        tryhop(4);emit(0xc7, check_zp(d), 0xd0, check_zp(s))
    else:
        emit_op("MOVF_v7", check_zp(d), check_zp(s))
@vasm
def COPY():
    emit_op("COPY_v7")
@vasm
def COPYN(n):
    n = check_zp(n)
    if args.cpu == 6:
        LDW(T3);tryhop(3);emit(0x2f,n,0xcd);STW(T3)
    else:
        emit_op("COPYN_v7", n)
@vasm
def NEGV(d):
    if args.cpu == 6:
        tryhop(3);emit(0x2f,check_zp(d),0x17)
    else:
        emit_op("NEGV_v7", check_zp(d))
@vasm
def MULW(d):
    emit_op("MULW_v7", check_zp(d))
@vasm
def RDIVU(d):
    emit_op("RDIVU_v7", check_zp(d))
@vasm
def RDIVS(d):
    emit_op("RDIVS_v7", check_zp(d))
@vasm
def INCV(d):
    if args.cpu == 6:
        tryhop(3);emit(0x2f, check_zp(d), 0x6a)
    else:
        emit_op("INCV_v7", check_zp(d))
@vasm
def ADDV(d):
    emit_op("ADDV_v7", check_zp(d))
@vasm
def SUBV(d):
    emit_op("SUBV_v7", check_zp(d))
@vasm
def ADDIV(i,d):
    emit_op("ADDIV_v7", check_zp(i), check_zp(d))
@vasm
def SUBIV(i,d):
    emit_op("SUBIV_v7", check_zp(i), check_zp(d))
@vasm
def ADDL():
    if args.cpu == 6:
        tryhop(2); emit(0xb1, 0x1a)
    else:
        emit_op("ADDL_v7")
@vasm
def ADDX():
    emit_op("ADDX_v7")
@vasm
def SUBL():
    if args.cpu == 6:
        tryhop(2); emit(0xb1, 0x1d)
    else:
        emit_op("SUBL_v7")
@vasm
def ANDL():
    if args.cpu == 6:
        tryhop(2); emit(0xb1, 0x20)
    else:
        emit_op("ANDL_v7")
@vasm
def ORL():
    if args.cpu == 6:
        tryhop(2); emit(0xb1, 0x23)
    else:
        emit_op("ORL_v7")
@vasm
def XORL():
    if args.cpu == 6:
        tryhop(2); emit(0xb1, 0x26)
    else:
        emit_op("XORL_v7")
@vasm
def NEGVL(d):
    if args.cpu == 6:
        tryhop(3); emit(0x2f, check_zp(d), 0xd8)
    else:
        emit_op("NEGVL_v7", check_zp(d))
@vasm
def NEGX():
    emit_op("NEGX_v7")
@vasm
def LSLVL(d):
    if args.cpu == 6:
        LDI(0);tryhop(4);emit(0xc7, check_zp(d), 0xd3, check_zp(d)+4)
    else:
        emit_op('LSLVL_v7', check_zp(d))
@vasm
def LSLXA():
    emit_op('LSLXA_v7')
@vasm
def CMPLS():
    if args.cpu == 6:
        tryhop(2); emit(0xb1, 0x2c)
    else:
        emit_op('CMPLS_v7')
@vasm
def CMPLU():
    if args.cpu == 6:
        tryhop(2); emit(0xb1, 0x29)
    else:
        emit_op('CMPLU_v7')
@vasm
def LSRXA():
    emit_op('LSRXA_v7')
@vasm
def RORX(cpu6exact=True):
    if args.cpu == 6:
        if cpu6exact:           # Move vAC-LSB to vAC-MSB
            ADDI(127);ANDI(128);ST(vACH)
        tryhop(4);emit(0xc7, LAX+5, 0xd6, LAX)
        if cpu6exact:           # Move vAC-MSB to vAC-LSB
            LD(vACH);PEEK()
    else:
        emit_op('RORX_v7')
@vasm
def MACX():
    emit_op('MACX_v7')
@vasm
def LDLAC():
    if args.cpu == 6:
        tryhop(3);emit(0x2f, LAC, 0x3d)
    else:
        emit_op('LDLAC_v7')
@vasm
def STLAC():
    if args.cpu == 6:
        tryhop(3);emit(0x2f, LAC, 0x3f)
    else:
        emit_op('STLAC_v7')
@vasm
def INCVL(d):
    if args.cpu == 6:
        tryhop(3);emit(0x2f, check_zp(d), 0x4f)
    else:
        emit_op('INCVL_v7', check_zp(d))
@vasm
def STFAC():
    emit_op('STFAC_v7')
@vasm
def LDFAC():
    emit_op('LDFAC_v7')
@vasm
def LDFARG():
    emit_op('LDFARG_v7')
@vasm
def PUSHV(d):
    emit_op('PUSHV_v7', check_zp(d))
@vasm
def POPV(d):
    emit_op('POPV_v7', check_zp(d))
@vasm
def LDXW(d,imm):
    emit_op('LDXW_v7', check_zp(d), lo(imm), hi(imm))
@vasm
def STXW(d,imm):
    emit_op('STXW_v7', check_zp(d), lo(imm), hi(imm))
@vasm
def LDSB(d):
    emit_op('LDSB_v7', check_zp(d))

# pseudo instructions used by the compiler
@vasm
def _SP(n):
    '''Pseudo-instruction to compute SP relative addresses'''
    n = v(n)
    if is_zero(n):
        LDW(SP);
    elif args.cpu < 6 and is_zeropage(-n):
        LDW(SP); SUBI(-n)
    else:
        _LDI(n); ADDW(SP)
@vasm
def _LDI(d):
    '''Emit LDI, LDNI or LDWI.'''
    # Warning. In rare cases, using _LDI instead of LDWI can lead to
    # infinite relaxation loops when the argument d is an expression
    # than can construct a small address that depends on a yet unknown
    # data label.
    d = v(d)
    if is_zeropage(d):
        LDI(d)
    elif args.cpu == 6 and is_zeropage(-d):
        LDNI(d)
    elif args.cpu >= 6 and is_zeropage(-d-1):
        LDNI(d)
    else:
        LDWI(d)
@vasm
def _LDW(d):
    '''Emit LDW or LDWI+DEEK.'''
    d = v(d)
    if is_zeropage(d):
        LDW(d)
    else:
        LDWI(d); DEEK()
@vasm
def _LD(d):
    '''Emit LD or LDWI+PEEK.'''
    d = v(d)
    if is_zeropage(d):
        LD(d)
    else:
        LDWI(d); PEEK()
@vasm
def _DEEKV(d):
    '''DEEKV(d) on cpu>=6, LDW(d);DEEK() otherwise'''
    if args.cpu >= 6:
        DEEKV(d)
    else:
        LDW(d); DEEK()
@vasm
def _PEEKV(d):
    '''PEEKV(d) on cpu>=6, LDW(d);PEEK() otherwise'''
    if args.cpu >= 6:
        PEEKV(d)
    else:
        LDW(d); PEEK()
@vasm
def _MOVIW(d,x):
    '''Moves immediate d into word var x.
       - Emits MOVIW, MOVQW, or a _LDI solution.
       - May trash vAC.'''
    d = int(v(d))
    if x == vAC:
        _LDI(d)
    elif args.cpu >= 6 and is_zeropage(d):
        MOVQW(d, check_zp(x))
    elif args.cpu >= 7:
        MOVIW(d, check_zp(x))
    else:
        _LDI(d);STW(x)
@vasm
def _ALLOC(d):
    '''Adds positive of negative immediate d to SP (not vSP).
       - Emits ALLOC, ADDIV, SUBIV or a _SP based solution.
       - May trash vAC.'''
    d = int(v(d))
    if d & 3:
        warning("Unaligned stack can cause serious trouble")
    if args.cpu >= 7:
        if SP == vSP and d >= -128 and d < 128:
            ALLOC(d)
        elif d > 0 and d < 256:
            ADDIV(d,SP)
        elif d < 0 and d > -256:
            SUBIV(-d,SP)
        elif d != 0:
            _LDI(d);ADDV(SP)
    elif d != 0:
        _SP(d);STW(SP)
@vasm
def _LDLW(off):
    '''Load word at offset <off> from SP (not vSP).
       - Emits LDLW LDXW (cpu7) or a DEEK solution'''
    off = int(v(off))
    if args.cpu >= 7 and SP == vSP and is_zeropage(off):
        LDLW(off)
    elif args.cpu > 7:
        LDXW(SP,off)
    else:
        _SP(off);DEEK()
@vasm
def _STLW(off, src=None):
    '''Store word at offset <off> from SP (not vSP).
       - Emits STLW STXW (cpu7) or a DOKE solution (which might
       clobber T2,T3).  Optional argument src can specify a source
       register other than vAC, allowing better DOKE solutions.'''
    off = int(v(off))
    if args.cpu >= 7 and SP == vSP and is_zeropage(off):
        if src != None and src != vAC: LDW(src)
        STLW(off)
    elif args.cpu >= 7:
        if src != None and src != vAC: LDW(src)
        STXW(SP,off)
    elif args.cpu >= 6:
        if src == None or src == vAC: STW(T3); src=T3
        _SP(off);DOKEA(src)
    else:
        if src == None or src == vAC: STW(T3); src=T3
        _SP(off);STW(T2);LDW(src);DOKE(T2)
@vasm
def _SHLI(imm):
    '''Shift vAC left by imm positions'''
    imm &= 0xf
    if args.cpu >= 7 and imm == 0x8:
        ST(vACH);MOVQB(0,vAC)
        imm &= 0x7
    if imm & 0x8:
        ST(vACH);ORI(255);XORI(255)
        imm &= 0x7
    if args.cpu >= 7 and imm == 0x4:
        MOVIW('SYS_LSLW4_46','sysFn');SYS(46)
        imm -= 4
    if args.cpu >= 7 and imm >= 3:
        MULQ(0xff ^ (0xff >> (imm-1)))
        imm = 0
    while imm > 0:
        LSLW()
        imm -= 1
@vasm
def _SHRIS(imm):
    '''Shift vAC right (signed) by imm positions'''
    imm &= 0xf
    if imm & 8:
        if args.cpu >= 7:
            LDSB(vACH)
        else:
            LD(vACH);XORI(128);SUBI(128)
        imm &= 7
    if imm == 1:
        extern("_@_shrs1")
        _CALLI("_@_shrs1")           # T3 >> 1 -> vAC
    elif imm > 0:
        STW(T3); LDI(imm)
        extern('_@_shrs')
        _CALLI('_@_shrs')            # T3 >> AC -> vAC
@vasm
def _SHRIU(imm):
    '''Shift vAC right (unsigned) by imm positions'''
    imm &= 0xf
    if imm & 8:
        LD(vACH)
        imm &= 7
    if args.cpu >= 7 and imm > 0:
        systable = (None,"SYS_LSRW1_48",
                    "SYS_LSRW2_52","SYS_LSRW3_52",
                    "SYS_LSRW4_50","SYS_LSRW5_50",
                    "SYS_LSRW6_48","SYS_LSRW7_30")
        MOVIW(systable[imm],'sysFn')
        SYS(int(systable[imm][-2:]))
    elif imm == 1:
        extern("_@_shru1")
        _CALLI("_@_shru1")
    elif imm > 0:
        STW(T3); LDI(imm)
        extern('_@_shru')
        _CALLI('_@_shru')       # T3 >> AC -> vAC
@vasm
def _SHL(d):
    '''Shift vAC left by [d] positions'''
    STW(T3); LDW(d)
    extern('_@_shl')
    _CALLI('_@_shl')            # T3 << AC -> vAC
@vasm
def _SHRS(d):
    '''Shift vAC right, signed, by [d] positions'''
    STW(T3); LDW(d)
    extern('_@_shrs')
    _CALLI('_@_shrs')           # T3 >> AC --> vAC
@vasm
def _SHRU(d):
    '''Shift vAC right, unsigned, by [d] positions'''
    STW(T3); LDW(d)
    extern('_@_shru')
    _CALLI('_@_shru')           # T3 >> AC --> vAC
@vasm
def _MUL(d):
    if args.cpu >= 7:
        MULW(d)
    elif 'has_at67_SYS_Multiply_s16' in rominfo:
        STW('sysArgs0'); LDW(d)
        extern('_@_at67_mul')
        _CALLI('_@_at67_mul')   # sysArg0 * AC --> vAC
    else:
        STW(T3); LDW(d)
        extern('_@_mul')
        _CALLI('_@_mul')        # T3 * AC --> vAC
@vasm
def _MULI(d):
    d = int(v(d))
    if abs(d) in mulq_map:
        if d < 0:
            d = -d
            if args.cpu >= 6:
                NEGV(vAC)
            else:
                STW(T3);LDI(0);SUBW(T3)
        c = mulq_map[abs(d)]
        if callable(c):
            c()
        else:
            MULQ(c)
    elif d != 1:
        STW(T3);_LDI(d);
        if args.cpu >= 7:
            MULW(T3)
        else:
            extern('_@_mul')
            _CALLI('_@_mul')    # T3 * AC --> vAC
        return

@vasm
def _DIVS(d):
    STW(T3); LDW(d)
    extern('_@_divs')
    _CALLI('_@_divs')           # T3 / AC --> vAC
@vasm
def _DIVU(d):
    STW(T3); LDW(d)
    extern('_@_divu')
    _CALLI('_@_divu')           # T3 / AC --> vAC
@vasm
def _MODS(d):
    STW(T3); LDW(d)
    extern('_@_mods')
    _CALLI('_@_mods')           # T3 % vAC --> vAC
@vasm
def _MODU(d):
    STW(T3); LDW(d)
    extern('_@_modu')
    _CALLI('_@_modu')           # T3 % vAC --> vAC
@vasm
def _DIVIS(d):
    d = int(v(d))
    ld = d.bit_length() - 1
    if d > 0 and ld > 0 and d == (1 << ld):
        _SHRIS(ld)
    else:
        STW(T3);_LDI(d)
        extern('_@_divs')
        _CALLI('_@_divs')       # T3 / AC --> vAC
@vasm
def _DIVIU(d):
    d = int(v(d))
    ld = d.bit_length() - 1
    if d > 0 and ld > 0 and d == (1 << ld):
        _SHRIU(ld)
    else:
        STW(T3);_LDI(d)
        extern('_@_divu')
        _CALLI('_@_divu')       # T3 / AC --> vAC
@vasm
def _MODIS(d):
    STW(T3);_LDI(d)
    extern('_@_mods')
    _CALLI('_@_mods')           # T3 % vAC --> vAC
@vasm
def _MODIU(d):
    STW(T3);_LDI(d)
    extern('_@_modu')
    _CALLI('_@_modu')           # T3 % vAC --> vAC
@vasm
def _BRA(d):
    emitjump(v(d))
@vasm
def _BEQ(d):
    if args.cpu >= 6:
        JEQ(d)
    else:
        emitjcc(BEQ, BNE, JEQ, v(d))
@vasm
def _BNE(d):
    if args.cpu >= 6:
        JNE(d)
    else:
        emitjcc(BNE, BEQ, JNE, v(d))
@vasm
def _BLT(d):
    if args.cpu >= 6:
        JLT(d)
    else:
        emitjcc(BLT, BGE, JLT, v(d))
@vasm
def _BGT(d):
    if args.cpu >= 6:
        JGT(d)
    else:
        emitjcc(BGT, BLE, JGT, v(d))
@vasm
def _BLE(d):
    if args.cpu >= 6:
        JLE(d)
    else:
        emitjcc(BLE, BGT, JLE, v(d))
@vasm
def _BGE(d):
    if args.cpu >= 6:
        JGE(d)
    else:
        emitjcc(BGE, BLT, JGE, v(d))
@vasm
def _CMPIS(d):
    '''Compare vAC (signed) with immediate in range 0..255'''
    if args.cpu >= 7:
        CMPIS(d)
    elif args.cpu >= 5 and not "without_CmpOps" in rominfo:
        CMPHS(0); SUBI(d)
    else:
        lbl = genlabel()
        tryhop(5)
        BLT(lbl)
        SUBI(d)
        label(lbl, hop=0)
@vasm
def _CMPIU(d):
    '''Compare vAC (unsigned) with immediate in range 0..255'''
    if args.cpu >= 7:
        CMPIU(d)
    elif args.cpu >= 5 and not "without_CmpOps" in rominfo:
        CMPHU(0); SUBI(d)
    else:
        lbl = genlabel()
        tryhop(8)
        BGE(lbl)
        LDWI(0x100)
        label(lbl, hop=0)
        SUBI(d)
@vasm
def _CMPWS(d):
    '''Compare vAC (signed) with register.'''
    if args.cpu >= 7:
        CMPWS(d)                # 36 cycles
    elif args.cpu >= 5 and not "without_CmpOps" in rominfo:
        d = v(d)
        CMPHS(d+1); SUBW(d)     # 28+28 cycles
    else:
        lbl1 = genlabel()
        lbl2 = genlabel()
        tryhop(16)
        XORW(d); BGE(lbl1)
        XORW(d); ORI(1); BRA(lbl2)
        label(lbl1, hop=0)
        XORW(d); SUBW(d)
        label(lbl2, hop=0)
@vasm
def _CMPWU(d):
    '''Compare vAC (unsigned) with register.'''
    if args.cpu >= 7:
        CMPWU(d)                # 36 cycles
    elif args.cpu >= 5 and not "without_CmpOps" in rominfo:
        d = v(d)
        CMPHU(d+1); SUBW(d)     # 28+28 cycles
    else:
        lbl1 = genlabel()
        lbl2 = genlabel()
        # no hops because cpu4 long jumps also use -2(vSP)
        tryhop(16)
        XORW(d); BGE(lbl1)
        LDW(d); ORI(1); BRA(lbl2)
        label(lbl1, hop=0)
        XORW(d); SUBW(d)
        label(lbl2, hop=0)
@vasm
def _MOVM(s,d,n,align=1): # was _BMOV
    '''Move memory block of size n from addr s to d.
       One of s or d can be either [vAC] or [SP,offset].
       Argument d can also be [T2].
       Trashes vAC, T0-T2.'''
    d = v(d)
    s = v(s)
    n = v(n)
    if s != d:
        if type(s) == list and len(s) == 2 and s[0] == SP:
            _SP(s[1]); s = [vAC]
        elif type(d) == list and len(d) == 2 and d[0] == SP:
            _SP(d[1]); d = [vAC]
        if args.cpu >= 6:
            if d == [vAC]:
                STW(T2)
            if s == [vAC]:
                STW(T3)
            if d != [vAC] and d != [T2]:
                _MOVIW(d,T2)
            if s != [vAC]:
                _MOVIW(s,T3)
            if n > 0 and n <= 256:
                COPYN(n & 0xff)
            elif n > 0:
                _LDI(n)
                lbl = genlabel(); label(lbl)
                COPY();_BNE(lbl)
        else:
            if d == [vAC]:
                STW(T2)
            if s == [vAC]:
                STW(T0)
            if d != [vAC] and d != [T2]:
                _LDI(d); STW(T2)
            if s != [vAC]:
                _LDI(s); STW(T0)
            _LDI(n);ADDW(T0);STW(T1)
            if align == 2:
                extern('_@_wcopy_')
                _CALLI('_@_wcopy_')         # [T0..T1) --> [T2..]
            else:
                extern('_@_bcopy_')
                _CALLI('_@_bcopy_')         # [T0..T1) --> [T2..]
@vasm
def _MOVL(s,d): # was _LMOV
    '''Move long from reg/addr s to d.
       One of s or d can be either [vAC] or [SP,offset].
       Argument d can be [T2].
       Can trash vAC, T0-T3'''
    s = v(s)
    d = v(d)
    if s != d:
        extern('_@_using_lmov')
        if type(s) == list and len(s) == 2 and s[0] == SP:
            _SP(s[1]); s = [vAC]
        elif type(d) == list and len(d) == 2 and d[0] == SP:
            _SP(d[1]); d = [vAC]
        if args.cpu >= 6:
            if is_zeropage(d,3) and is_zeropage(s,3):
                MOVL(s,d)
            elif is_zeropage(d,3):
                if s != [vAC]:
                    _LDI(s)
                if d == LAC:
                    LDLAC()
                else:
                    DEEKA(d);ADDI(2);DEEKA(d+2)
            elif is_zeropage(s,3):
                if d == [T2]:
                    LDW(T2)
                elif d != [vAC]:
                    _LDI(d)
                if s == LAC:
                    STLAC()
                else:
                    DOKEA(s);ADDI(2);DOKEA(s+2)
            else:
                if d == [vAC]:
                    STW(T2)
                if s == [vAC]:
                    STW(T3)
                if d != [T2] and d != [vAC]:
                    _MOVIW(d, T2)
                if s != [vAC]:
                    _MOVIW(s, T3)
                COPYN(4)        # 5-9 bytes
        else:
            if is_zeropage(d,3) and is_zeropage(s,3):
                if args.cpu >= 5:
                    LDWI(((d & 0xff) << 8) | (s & 0xff))
                    extern('_@_lcopyz_')
                    _CALLI('_@_lcopyz_')  # 6 bytes
                else:
                    LDW(s);STW(d)
                    LDW(s+2);STW(d+2)     # 8 bytes
            else:
                if d == [vAC]:
                    STW(T2)
                if s == [vAC]:
                    STW(T0)
                if d != [vAC] and d != [T2]:
                    _LDI(d); STW(T2)
                if s != [vAC]:            # 5-13 bytes
                    _LDI(s); STW(T0)
                extern('_@_lcopy_')
                _CALLJ('_@_lcopy_')
@vasm
def _LADD():
    if args.cpu >= 6:
        ADDL()
    else:
        extern('_@_ladd')
        _CALLI('_@_ladd')       # LAC+[vAC] --> LAC
@vasm
def _LSUB():
    if args.cpu >= 6:
        SUBL()
    else:
        extern('_@_lsub')
        _CALLI('_@_lsub')       # LAC-[vAC] --> LAC
@vasm
def _LMUL():
    extern('_@_lmul')
    _CALLI('_@_lmul')           # LAC*[vAC] --> LAC
@vasm
def _LDIVS():
    extern('_@_ldivs')
    _CALLI('_@_ldivs')          # LAC/[vAC] --> LAC
@vasm
def _LDIVU():
    extern('_@_ldivu')
    _CALLI('_@_ldivu')          # LAC/[vAC] --> LAC
@vasm
def _LMODS():
    extern('_@_lmods')
    _CALLI('_@_lmods')          # LAC%[vAC] --> LAC
@vasm
def _LMODU():
    extern('_@_lmodu')
    _CALLI('_@_lmodu')          # LAC%[vAC] --> LAC
@vasm
def _LSHL():
    if args.cpu >= 7:
        MOVQB(0,LAX);LSLXA()
    else:
        extern('_@_lshl')
        _CALLI('_@_lshl')       # LAC<<vAC --> LAC
@vasm
def _LSHRS():
    extern('_@_lshrs')
    _CALLI('_@_lshrs')          # LAC>>vAC --> LAC
@vasm
def _LSHRU():
    if args.cpu >= 7:
        LSRXA()
    else:
        extern('_@_lshru')
        _CALLI('_@_lshru')      # LAC>>vAC --> LAC
@vasm
def _LNEG():
    if args.cpu >= 6:
        NEGVL(LAC)
    else:
        extern('_@_lneg')
        _CALLJ('_@_lneg')       # -LAC --> LAC
@vasm
def _LCOM():
    extern('_@_lcom')
    _CALLJ('_@_lcom')           # ~LAC --> LAC
@vasm
def _LAND():
    if args.cpu >= 6:
        ANDL()
    else:
        extern('_@_land')
        _CALLI('_@_land')       # LAC&[vAC] --> LAC
@vasm
def _LOR():
    if args.cpu >= 6:
        ORL()
    else:
        extern('_@_lor')
        _CALLI('_@_lor')        # LAC|[vAC] --> LAC
@vasm
def _LXOR():
    if args.cpu >= 6:
        XORL()
    else:
        extern('_@_lxor')
        _CALLI('_@_lxor')       # LAC^[vAC] --> LAC
@vasm
def _LCMPS():
    if args.cpu >= 6:
        CMPLS()
    else:
        extern('_@_lcmps')
        _CALLI('_@_lcmps')      # SGN(LAC-[vAC]) --> vAC
@vasm
def _LCMPU():
    if args.cpu >= 6:
        CMPLU()
    else:
        extern('_@_lcmpu')
        _CALLI('_@_lcmpu')      # SGN(LAC-[vAC]) --> vAC
@vasm
def _LCMPX():
    if args.cpu >= 6:
        CMPLS()
    else:
        extern('_@_lcmpx')
        _CALLI('_@_lcmpx')      # TST(LAC-[vAC]) --> vAC
@vasm
def _STLU(d):
    STW(d);LDI(0);STW(d+2);
@vasm
def _STLS(d):
    if args.cpu >= 7:
        STW(d);LDSB(vACH);LDSB(vACH);STW(d+2)
    else:
        extern('_@_lexts')      # (vAC<0) ? -1 : 0 --> vAC
        STW(d);_CALLI('_@_lexts');STW(d+2)
@vasm
def _MOVF(s,d): # was _FMOV
    '''Move float from reg s to d with special cases when s or d is FAC.
       One of s or d can be [vAC] or [SP, offset].
       Argument d can also be [T2].
       Can trash vAC, T0-T2, and T3 if s or d is FAC.'''
    s = v(s)
    d = v(d)
    if s != d:
        extern('_@_using_fmov')
        if type(s) == list and len(s) == 2 and s[0] == SP:
            _SP(s[1]); s = [vAC]
        elif type(d) == list and len(d) == 2 and d[0] == SP:
            _SP(d[1]); d = [vAC]
        if d == FAC:
            if s != [vAC]:
                _LDI(s)
            if args.cpu >= 7:
                LDFAC()
            else:
                extern('_@_fldfac')
                _CALLI('_@_fldfac')
        elif s == FAC:
            if d == [T2]:
                LDW(T2)
            elif d != [vAC]:
                _LDI(d)
            if args.cpu >= 7:
                STFAC()
            else:
                extern('_@_fstfac')
                _CALLI('_@_fstfac')
        elif is_zeropage(d, 4) and is_zeropage(s, 4):
            if args.cpu >= 6:
                MOVF(s,d)
            elif args.cpu >= 5:
                LDWI(((d & 0xff) << 8) | (s & 0xff))
                extern('_@_fcopyz_')
                _CALLI('_@_fcopyz_')
            else:
                LDW(s);STW(d);LDW(s+2);STW(d+2)
                LD(s+4);ST(d+4)
        elif args.cpu >= 6:
            if d == [vAC]:
                STW(T2)
            if s == [vAC]:
                STW(T3)
            if d != [vAC] and d != [T2]:
                _MOVIW(d,T2)
            if s != [vAC]:
                _MOVIW(s,T3)
            COPYN(5)
        else:
            maycross=False
            if d == [vAC]:
                STW(T2)
                maycross = True
            if s == [vAC]:
                STW(T0)
                maycross = True
            if d != [vAC] and d != [T2]:
                _LDI(d); STW(T2)
                maycross = maycross or (int(d) & 0xfc == 0xfc)
            if s != [vAC]:
                _LDI(s); STW(T0)
                maycross = maycross or (int(s) & 0xfc == 0xfc)
            if maycross:
                extern('_@_fcopy_')       # [T0..T0+5) --> [T2..]
                _CALLJ('_@_fcopy_')
            else:
                extern('_@_fcopync_')     # [T0..T0+5) --> [T2..]
                _CALLJ('_@_fcopync_')     # without page crossing!
@vasm
def _FADD():
    extern('_@_fadd')
    _CALLI('_@_fadd')               # FAC+[vAC] --> FAC
@vasm
def _FSUB():
    extern('_@_fsub')
    _CALLI('_@_fsub')               # FAC-[vAC] --> FAC
@vasm
def _FMUL():
    extern('_@_fmul')
    _CALLI('_@_fmul')               # FAC*[vAC] --> FAC
@vasm
def _FDIV():
    extern('_@_fdiv')
    _CALLI('_@_fdiv')               # FAC/[vAC] --> FAC
@vasm
def _FDIVR():
    extern('_@_fdivr')
    _CALLI('_@_fdivr')              # [vAC]/FAC --> FAC
@vasm
def _FNEG():
    extern('_@_fneg')
    _CALLJ('_@_fneg')               # -FAC --> FAC
@vasm
def _FCMP():
    extern('_@_fcmp')
    _CALLI('_@_fcmp')               # FAC?[vAC] --> vAC (-1/0/+1)
@vasm
def _FSGN():
    extern('_@_fsign')
    _CALLI('_@_fsign')               # SGN(FAC) --> vAC (-1/0/+1)
@vasm
def _FTOU():
    extern('_@_ftou')
    _CALLJ('_@_ftou')
@vasm
def _FTOI():
    extern('_@_ftoi')
    _CALLJ('_@_ftoi')
@vasm
def _FCVI():
    extern('_@_fcvi')
    _CALLJ('_@_fcvi')
@vasm
def _FCVU():
    extern('_@_fcvu')
    _CALLJ('_@_fcvu')
@vasm
def _FSCALB():
    extern('_@_fscalb')
    _CALLI('_@_fscalb')

@vasm
def _CALLI(d):
    '''Call subroutine at far location d.
       - For cpu >= 5. this function just emits a CALLI instruction
       - For cpu < 5, this function can trash sysArgs[67].'''
    if args.cpu >= 5:
        CALLI(d)
    else:
        # no hops because cpu4 long jumps also use -2(vSP)
        tryhop(11);STLW(-2);LDWI(d);STW('sysArgs6');LDLW(-2);CALL('sysArgs6')
@vasm
def _CALLJ(d):
    '''Call subroutine at far location d.
       - For cpu >= 5. this function just emits a CALLI instruction
       - For cpu < 5, this function trashes vAC.'''
    if args.cpu >= 5:
        CALLI(d)
    else:
        tryhop(5);LDWI(d);CALL(vAC)
@vasm
def _PROLOGUE(framesize,maxargoffset,mask):
    '''Function prologue'''
    tryhop(2);LDW(vLR);STW(B0)
    if args.cpu >= 7:
        _ALLOC(-framesize)
        if maxargoffset == 0:
            LDW(SP)
        else:
            _LDI(maxargoffset);ADDW(SP)
    else:
        _SP(-framesize);STW(SP)
        if maxargoffset != 0:
            ADDI(maxargoffset)
    if mask == 0 and args.cpu >= 6:
        DOKEA(B0)
    elif args.cpu >= 5:
        extern('_@_save_%02x' % mask)
        CALLI('_@_save_%02x' % mask)
    else:
        extern('_@_save_%02x' % mask)
        STW(T2);LDWI('_@_save_%02x' % mask);CALL(vAC)
@vasm
def _EPILOGUE(framesize,maxargoffset,mask,saveAC=False):
    '''Function epilogue'''
    if saveAC:
        STW(R8);
    if args.cpu >= 7:
        _ALLOC(framesize)
        _SP(maxargoffset-framesize)
    else:
        _SP(framesize);STW(SP)
        if framesize - maxargoffset < 256:
            SUBI(framesize - maxargoffset)
        else:
            _SP(maxargoffset-framesize)
    if args.cpu >= 5:
        extern('_@_rtrn_%02x' % mask)
        CALLI('_@_rtrn_%02x' % mask)
    else:
        extern('_@_rtrn_%02x' % mask)
        STW(T3);LDWI('_@_rtrn_%02x' % mask);CALL(vAC)


# compatibility

module_dict['_LMOV'] = _MOVL
module_dict['_FMOV'] = _MOVF
module_dict['_BMOV'] = _MOVM

# ------------- reading .s/.o/.a files

def read_file(f):
    '''Reads a .s/.o/.a file in a pristine environment'''
    global the_module, the_fragment, new_modules, module_list
    debug(f"reading '{f}'")
    with open(f, 'r') as fd:
        s = fd.read()
        try:
            c = compile(s, f, 'exec')
        except SyntaxError as err:
            fatal(str(err))
    the_module = None
    the_fragment = None
    new_modules = []
    exec(c, new_globals())
    if len(new_modules) == 0:
        warning(f"file {f} did not define any module")
    if f.endswith(".a") or len(new_modules) > 1:
        libid = id(new_modules[0])
        libname = os.path.basename(f)
        for m in new_modules:
            m.library = libid
            m.fname = f"{libname}({m.name})"
    module_list += new_modules
    new_modules = []

def search_file(fn, path):
    '''Searches a file along a given path.'''
    for d in path:
        f = os.path.join(d, fn)
        if os.access(f, os.R_OK):
            return f
    return None

def read_lib(l):
    '''Search a library file along the library path and read it.'''
    f = search_file(f"lib{l}.a", args.L)
    if not f:
        fatal(f"library -l{l} not found!")
    return read_file(f)

def read_map(mn, overlays = None):
    '''Read a linker map file.'''
    fn = search_file(f"map{mn}/map.py", args.mapdir)
    if not fn:
        fatal(f"cannot find linker map '{mn}'")
    with open(fn, 'r') as fd:
        exec(compile(fd.read(), fn, 'exec'), globals())
    if not map_segments:
        fatal(f"map '{mn}' does not define 'map_segments'")
    if not map_modules:
        fatal(f"map '{mn}' does not define 'map_modules'")
    md = os.path.dirname(fn)
    args.L.append(md)
    for ov in overlays or []:
        fn = ov if '/' in ov else os.path.join(md, f"x-{ov}.py")
        if not os.access(fn, os.R_OK):
            fatal(f"cannot load map overlay '{ov}'")
        with open(fn, 'r') as fd:
            exec(compile(fd.read(), fn, 'exec'), globals())

def read_interface():
    '''Read `interface.json' as known symbols.'''
    global symdefs
    with open(os.path.join(lccdir,'interface.json')) as file:
        for (name, value) in json.load(file).items():
            symdefs[name] = value if isinstance(value, int) else int(value, base=0)
    with open(os.path.join(lccdir,'interface-dev.json')) as file:
        for (name, value) in json.load(file).items():
            symdefs[name] = value if isinstance(value, int) else int(value, base=0)

def get_rominfo(roms, rom):
    if rom in roms:
        ri = roms[rom]
        if 'inherits' in ri:
            if ri['inherits'] not in roms:
                fatal(f"roms.json: rom '{rom}' inherits from an unknown rom")
            else:
                rj = get_rominfo(roms, ri['inherits'])
                for k in rj:
                    if k not in ri:
                        ri[k] = rj[k]
                ri.pop('inherits')
        return ri
    return None

def read_rominfo(rom):
    '''Read `rom.jsom' to translate rom names into romType byte and cpu version.'''
    global rominfo, romtype, romcpu
    with open(os.path.join(lccdir,'roms.json')) as file:
        rominfo = get_rominfo(json.load(file), rom)
    if rominfo and 'romType' in rominfo and 'cpu' in rominfo:
        romtype = int(str(rominfo['romType']),0)
        romcpu = int(str(rominfo['cpu']),0)
    else:
        print(f"glink: warning: rom '{args.rom}' is not recognized", file=sys.stderr)
        rominfo = {}
    if romcpu and not args.cpu:
        args.cpu = romcpu
    if romcpu and args.cpu and args.cpu > romcpu:
        print(f"glink: warning: rom '{args.rom}' does not implement cpu{args.cpu}", file=sys.stderr)
    if 'warning' in rominfo:
        warning(rominfo['warning'])



# ------------- compute code closure from import/export information

def find_exporters(sym):
    elist = []
    # collect all modules that export sym
    for m in module_list:
        if sym in m.exports:
            elist.append(m)
    if not elist:
        # otherwise find an input file(not a library) that has a common named sym.
        for m in module_list:
            if not m.library:
                for f in m.code:
                    if f.segment == 'COMMON' and f.name == sym:
                        return [ m ]
    return elist

def measure_data_fragment(m, frag):
    global the_module, the_fragment, the_pc
    the_module = m
    the_fragment = frag
    the_pc = 0
    try:
        frag.func()
    except Exception as err:
        fatal(str(err), exc=True)
    frag.size = the_pc

def measure_code_fragment(m, frag):
    global the_module, the_fragment, the_pc
    global lbranch_counter, short_function
    the_module = m
    the_fragment = frag
    the_pc = 0
    lbranch_counter = 0
    short_function = False
    try:
        frag.func()
    except Exception as err:
        fatal(str(err), exc=True)
    function_size = the_pc - lbranch_counter
    if frag.nohop:
        debug(f"- code fragment '{frag.name}' is {function_size} bytes long")
        if function_size >= 256:
            error("code fragment '{frag.name}' is declared short but is too long")
    else:
        debug(f"- code fragment '{frag.name}' is {function_size}+{lbranch_counter} bytes long")
    frag.size = function_size

def measure_fragments(m):
    for frag in m.code:
        if frag.segment in ('DATA', 'BSS') and not frag.size:
            measure_data_fragment(m, frag)
        elif frag.segment in ('CODE'):
            measure_code_fragment(m, frag)
    the_module = None
    the_fragment = None

def check_conditional_import(tp):
    if len(tp) < 4 or tp[3] != 'IF':
        return False
    for sym in tp[4:]:
        if not sym in exporters:
            return False
    return True

def compute_closure():
    global module_list, exporters
    # compute closure from start symbol
    implist = [ args.e ] + args.r
    cimplist = []
    for sym in implist:
        if sym in exporters:
            pass
        elif sym in symdefs:
            pass
        else:
            e = None
            elist = find_exporters(sym)
            for m in elist:
                if m.library:                      # rules for selecting one of many library
                    if e and not e.library:        # modules exporting a same required symbol:
                        pass                       # -- cannot override a non-library module
                    elif m.cpu > args.cpu:         # -- ignore exports when module targets too high a cpu.
                        pass                       # -- prefers exports targeting a higher cpu within a same library.
                    elif not e or (m.library == e.library and m.cpu > e.cpu):
                        e = m
                else:                              # complain when a required symbol is exported
                    if e and not e.library:        # by multiple non-library files.
                        error(f"symbol '{sym}' is exported by both '{e.fname}' and '{m.fname}'", dedup=True)
                    e = m
            if e and not e.used:
                debug(f"including module '{e.fname}' for symbol '{sym}'")
                e.used = True
                for sym in e.exports:              # register all symbols exported by the selected module
                    if sym in exporters:           # -- warn about possible conflicts
                        error(f"symbol '{sym}' is exported by both '{e.fname}' and '{exporters[sym].fname}'", dedup=True)
                    if sym not in exporters or exporters[sym].library:
                        exporters[sym] = e
                measure_fragments(e)               # -- check all fragment code, compute missing lengths or exports
                for sym in e.imports:              # -- add all its imports to the list of required imports
                    implist.append(sym)
                for tp in e.cimports:              # -- process conditional imports
                    cimplist.append((e, *tp))
                if cimplist:
                    for i in range(len(cimplist)):
                        tp = cimplist[i]
                        if check_conditional_import(tp):
                            tp[0].imports.append(tp[2])
                            implist.append(tp[2])
                            del cimplist[i]
    # recompute module_list
    nml = []
    for m in module_list:
        if m.used:
            nml.append(m)
        elif not m.library:
            warning(f"file '{m.fname}' was not used")
    return nml

def convert_common_symbols():
    '''Common symbols are instanciated in one of the module
       and referenced by the other modules.'''
    for m in module_list:
        for decl in m.code:
            if decl.segment == 'COMMON':
                sym = decl.name
                if sym in exporters:
                    pass
                else:
                    debug(f"instantiating common '{sym}' in '{m.fname}'")
                    decl.segment = 'BSS'
                    exporters[sym] = m

def check_undefined_symbols():
    und = {}
    comma = ", "
    def check(s):
        if s not in exporters and s not in symdefs:
            mn = f"'{m.fname}'"
            if s in und:
                und[s].append(mn)
            else:
                und[s] = [mn]
    for m in module_list:
        for s in m.imports:
            check(s)
    for s in args.r:
        check(s)
    for s in und:
        error(f"undefined symbol '{s}' imported by {comma.join(und[s])}", dedup=True)



# ------------- passes

class Stop(Exception):
    def __init__(self, msg):
        self.msg = msg

def round_used_segments():
    '''Split all segments containing code or data into
       a used segment and a free segment starting on
       a page boundary. Marks used segment as non-BSS.'''
    for (i,s) in enumerate(segment_list):
        epage = (s.pc + 0xff) & ~0xff
        if s.pc > s.saddr and s.eaddr > epage:
            segment_list.insert(i+1, Segment(epage, s.eaddr, s.flags))
            s.eaddr = epage
            if args.d >= 2:
                debug(f"rounding {segment_list[i:i+2]}")
        if s.pc > s.saddr:
            s.nbss = True

def aligned(addr, align):
    if align and align > 1:
        addr = align * ((addr + align - 1) // align)
    return addr

def find_data_segment(size, align=None):
    amin = the_fragment.amin
    amax = the_fragment.amax
    for (i,s) in enumerate(segment_list):
        if amin == None and (s.flags & 0x2):  # not a data segment
            continue
        addr = aligned(s.pc, align)
        if amin != None and amin > addr:
            addr = aligned(amin, align)
        if the_fragment.nohop and (addr ^ (addr + size - 1)) & 0xff00 != 0:
            addr = aligned(addr, 256)
        if amin != None and amax == None:
            if not (amin >= s.saddr and amin < s.eaddr):
                continue
            if amin < s.pc:
                raise Stop(f"Requested address for fragment {the_fragment.name}@{hex(amin)} is busy")
            if addr > amin:
                raise Stop(f"Requested address for fragment {the_fragment.name}@{hex(amin)} is misaligned")
            if addr + size > s.eaddr:
                raise Stop(f"Fragment {the_fragment.name}@{hex(amin)} does not fit at the requested address")
        if addr + size > s.eaddr:
            continue
        if amax != None and addr + size > amax + 1:
            continue
        while addr > s.pc and s.pc > s.saddr and addr < s.pc + 4:
            while s.pc < addr:                  # not worth splitting
                s.buffer.append(0) if s.buffer else None
                s.pc += 1
            s.pc = addr
        if addr > s.pc:                         # split the segment
            ns = Segment(addr, s.eaddr, s.flags)
            s.eaddr = addr
            segment_list.insert(i+1, ns)
            s = ns
            i = i+1
        return s

def find_code_segment(size):
    size = min(256, size)
    amin = the_fragment.amin
    amax = the_fragment.amax
    for (i,s) in enumerate(segment_list):
        if amin == None and s.flags & 0x1:  # not a code segment
            continue
        if amin and amax and amin < 0x100 and amax >= 0x100:
            amin = 0x100                    # do not place code in page zero
        addr = s.pc
        if amin != None and amin > s.pc:
            addr = amin
        epage = (addr | 0xff) + 1
        if amin != None and amax == None:
            if not (amin >= s.saddr and amin < s.eaddr):
                continue
            if amin >= s.saddr and amin < s.pc:
                raise Stop(f"Requested address for fragment {the_fragment.name}@{hex(amin)} is busy")
            if amin < s.eaddr and amin + size > min(epage, s.eaddr):
                raise Stop(f"Fragment {the_fragment.name}@{hex(amin)} does not fit at the requested address")
        if addr + size > epage:
            addr = epage
            epage = (addr | 0xff) + 1
        if addr + size > min(epage, s.eaddr):
            continue
        if amax != None and addr + size > amax + 1:
            continue
        # possibly carve segment before address addr
        if addr > s.pc:
            ns = Segment(addr, s.eaddr, s.flags)
            s.eaddr = addr
            segment_list.insert(i+1, ns)
            s = ns
            i = i+1
        # since code segments cannot cross page boundaries
        # it is sometimes necessary to carve a code segment from a larger one
        if s.eaddr > epage:
            ns = Segment(epage, s.eaddr, s.flags)
            s.eaddr = epage
            segment_list.insert(i+1, ns)
        return s
    # not found
    return None

def assemble_code_fragments(m, placed=False, absolute=False):
    global the_module, the_fragment, the_segment, the_pc
    global hops_enabled, short_function
    the_module = m
    for frag in m.code:
        the_fragment = frag
        if frag.segment == 'CODE':
            if bool(frag.amin) != bool(placed):
                continue
            if placed and bool(frag.amax) == absolute:
                continue
            funcsize = frag.size
            the_segment = None
            sfst = min(256, args.sfst or 96)
            if frag.nohop or funcsize <= sfst:
                short_function = True
                hops_enabled = False
                the_segment = find_code_segment(funcsize)
                if frag.nohop and not the_segment:
                    error(f"cannot find a segment for short code fragment '{frag.name}' of length {funcsize}")
                if the_segment and (args.d >= 2 or final_pass):
                    debug(f"assembling code fragment '{frag.name}' at {hex(the_segment.pc)} in {the_segment}")
            if not the_segment:
                short_function = False
                hops_enabled = True
                lfss = args.lfss or 32
                the_segment = find_code_segment(min(lfss, 256))
                if not the_segment:
                    raise Stop(f"cannot fit code fragment '{frag.name}'")
                if the_segment and (args.d >= 2 or final_pass):
                    debug(f"assembling code fragment '{frag.name}' at {hex(the_segment.pc)} in {the_segment}")
            the_pc = the_segment.pc
            if args.fragments and final_pass:
                record_fragment_address(the_pc)
            try:
                frag.func()
            except Exception as err:
                fatal(str(err), exc=True)
            the_segment.pc = the_pc
            if args.fragments and final_pass:
                record_fragment_address(the_pc)
            if args.rpth and labelchange_counter > args.rpth and not final_pass:
                raise Stop(f"{labelchange_counter} changed labels already: restarting a new pass.")

def assemble_data_fragments(m, cseg, placed=False):
    global the_module, the_fragment, the_segment, hops_enabled, the_pc
    global labelchange_counter
    the_module = m
    for frag in m.code:
        the_fragment = frag
        if bool(frag.amin) != bool(placed):
            continue
        if frag.segment == cseg:
            hops_enabled = False
            the_segment = find_data_segment(frag.size, align=frag.align)
            if not the_segment:
                raise Stop(f"cannot fit {cseg} fragment '{frag.name}'")
            elif args.d >= 2 or final_pass:
                debug(f"assembling {cseg} fragment '{frag.name}' at {hex(the_segment.pc)} in {the_segment}")
            the_pc = the_segment.pc
            if args.fragments and final_pass:
                record_fragment_address(the_pc)
            try:
                if isinstance(frag.func, (builtins.bytes, bytearray)):
                    emit(*frag.func)
                else:
                    frag.func()
            except Exception as err:
                fatal(str(err), exc=True)
            the_segment.pc = the_pc
            if args.fragments and final_pass:
                record_fragment_address(the_pc)

def run_pass():
    global the_pass, the_module, the_fragment
    global labelchange_counter, genlabel_counter
    global segment_list, symdefs
    # initialize
    the_pass += 1
    labelchange_counter = 0
    genlabel_counter = 0
    segment_list = create_zpage_segments()
    for (s,e,d) in map_segments():
        segment_list.append(Segment(s,e,d))
    debug(f"pass {the_pass}")
    try:
        # code segments with explicit address or placement constraints
        for m in module_list:
            assemble_code_fragments(m, placed=True, absolute=True)
        for m in module_list:
            assemble_code_fragments(m, placed=True, absolute=False)
        for m in module_list:
            assemble_data_fragments(m, 'DATA', placed=True)
        for m in module_list:
            assemble_data_fragments(m, 'BSS', placed=True)
        # remaining code segments
        for m in module_list:
            assemble_code_fragments(m, placed=False)
        # data segments
        for m in module_list:
            assemble_data_fragments(m, 'DATA')
        round_used_segments()
        # bss segments
        for m in module_list:
            assemble_data_fragments(m, 'BSS')
    except Stop as stop:
        if final_pass or not labelchange_counter:
            fatal(stop.msg)
        elif args.d >= 2:
            print("(glink debug) " + stop.msg, file=sys.stderr)
    # cleanup
    the_module = None
    the_fragment = None

def run_passes():
    global final_pass
    final_pass = False
    while labelchange_counter:
        run_pass()
    final_pass = True
    run_pass()


# ------------- final

address_to_segment_cache = {}

def find_segment_for_address(addr):
    if addr in address_to_segment_cache:
        return address_to_segment_cache[addr]
    for s in segment_list:
        if addr >= s.saddr and addr < s.pc:
            address_to_segment_cache[addr] = s
            return s
    fatal(f"internal error: no segment for address {hex(addr)}")

def deek_gt1(addr):
    s = find_segment_for_address(addr)
    o = addr - s.saddr
    return s.buffer[o] + (s.buffer[o+1] << 8)

def doke_gt1(addr, val):
    s = find_segment_for_address(addr)
    o = addr - s.saddr
    s.buffer[o] = val & 0xff
    s.buffer[o+1] = (val >> 8) & 0xff

def process_magic_bss(s, head_module, head_addr):
    '''Construct a linked list of sizeable bss segments to be cleared at runtime.'''
    for s in segment_list:
        if s.pc > s.saddr + 4 and not s.nbss:
            debug(f"BSS segment {hex(s.saddr)}-{hex(s.pc)} will be cleared at runtime")
            size = s.pc - s.saddr
            s.buffer = bytearray(4)
            doke_gt1(s.saddr, size)
            doke_gt1(s.saddr + 2, deek_gt1(head_addr))
            doke_gt1(head_addr, s.saddr)

def process_magic_heap(s, head_module, head_addr):
    '''Construct a linked list of heap segments.'''
    for s in segment_list:
        if s.flags & 0x4:
            continue
        a0 = (s.pc + 3) & ~0x3
        a1 = s.eaddr &  ~0x3
        if a1 - a0 >= max(24, args.mhss or 24):
            s.pc = a0 + 4
            if not s.buffer:
                s.buffer = bytearray(4)
            else:
                s.buffer.extend(bytearray(s.pc - s.saddr - len(s.buffer)))
            doke_gt1(a0, a1 - a0)
            doke_gt1(a0 + 2, deek_gt1(head_addr))
            doke_gt1(head_addr, a0)

def process_magic_list(s, head_module, head_addr):
    '''Constructs a linked list of structures defined in modules.'''
    for m in module_list:
        if m != head_module:
            if s in m.symdefs:
                cons_addr = m.symdefs[s]
                for frag in m.code:
                    if frag.name == s and frag.segment == 'DATA':
                        break
                if not frag or frag.size < 4 or frag.align < 2:
                    return warning(f"ignoring magic symbol '{s}' in {m.fname} (wrong type)")
                doke_gt1(cons_addr + 2, deek_gt1(head_addr))
                doke_gt1(head_addr, cons_addr)

def process_magic_symbols():
    '''
    Magic symbols have names like '__glink_magic_xxx' and cause glink
    to construct a linked list of arbitrary data entries. The head of
    the list must be an *exported* pointer named '__glink_magic_xxx'
    and initialized to the value 0xBEEF. When this happens, glink
    searches each module for a *static* data item named
    '__glink_magic_xxx' large enough to contain at least two
    pointers. The first pointer (car) is left untouched.  The second
    pointer (cdr) is used to construct a linked list.

    The library uses two magic lists for which the
    first pointer is a function pointer:
     * '__glink_magic_init' is a list of initialization
       functions called before main().
     * '__glink_magic_init' is a list of finalization
       functions called by exit().

    In addition, there are two magic lists whose records are
    not found in modules but allocated by the linker.
     * '__glink_magic_bss' is a linked list of BSS segments
       that must be cleared at runtime. Each list record occupies
       the first 4 bytes of a segment. The first pointer contains
       the segment size. This is used by '_init1.c'.
     * '__glink_magic_heap' is a linked list of heap segments
       for the malloc() function. Each list record occupies
       the first 4 bytes of a segment. The first pointer contains
       the segment size.
     * '__glink_magic_egt1' is not a list but merely a pointer
       that marks the end of the highest gt1 segment.
    '''
    egt1_addr = None
    for s in exporters:
        if s.startswith("__glink_magic_"):
            head_module = exporters[s]
            head_addr = head_module.symdefs[s]
            for frag in head_module.code:
                if frag.name == s and frag.segment == 'DATA':
                    if frag.size != 2 or frag.align != 2:
                        return warning(f"ignoring magic symbol '{s}' (list head not a pointer)")
                if deek_gt1(head_addr) != 0xBEEF:
                    return warning(f"ignoring magic symbol '{s}' (list head not 0xBEEF)")
            doke_gt1(head_addr, 0)
            if s == '__glink_magic_bss':
                process_magic_bss(s, head_module, head_addr)
            elif s == '__glink_magic_heap':
                process_magic_heap(s, head_module, head_addr)
            elif s == '__glink_magic_egt1':
                egt1_addr = head_addr
            else:
                process_magic_list(s, head_module, head_addr)
    if egt1_addr != None:
        egt1 = 0
        for s in segment_list:
            if s.buffer and s.saddr + len(s.buffer) > egt1:
                egt1 = s.saddr + len(s.buffer)
        debug(f"Last GT1 segments ends at address {hex(egt1)}\n")
        doke_gt1(egt1_addr, egt1)

def save_gt1(fname, start):
    with open(fname,"wb") as fd:
        seglist = segment_list.copy()
        seglist.sort(key = lambda x : x.saddr)
        # collapse zeropage segments
        zpseg = None
        for s in seglist:
            if not s.buffer:
                continue
            elif s.saddr & 0xff00:
                break
            elif not zpseg:
                zpseg = s
            else:
                pc = zpseg.saddr + len(zpseg.buffer)
                assert s.saddr >= pc
                zpseg.buffer += builtins.bytes(s.saddr - pc)
                if pc < 0x80 and s.saddr > 0x80:
                    zpseg.buffer[0x80 - zpseg.saddr] = 1
                zpseg.buffer += s.buffer
                zpseg.eaddr = s.eaddr
                s.buffer = None
        # save segments
        for s in seglist:
            if not s.buffer:
                continue
            a0 = s.saddr
            pc = s.saddr + len(s.buffer)
            while a0 < pc:
                a1 = min(s.eaddr, (a0 | 0xff) + 1)
                buffer = s.buffer[(a0-s.saddr):(a1-s.saddr)]
                fd.write(builtins.bytes((hi(a0),lo(a0),len(buffer)&0xff)))
                fd.write(buffer)
                a0 = a1
        fd.write(builtins.bytes((0, hi(start), lo(start))))

def print_symbols(allsymbols=False):
    syms = []
    for m in module_list:
        for s in m.symdefs:
            if allsymbols or not s.startswith('.'):
                exported = (s in exporters) and (exporters[s] == m)
                syms.append((m.symdefs[s], s, exported, m.fname))
    syms.sort(key = lambda x : x[0] )
    syms.sort(key = lambda x : x[1] )
    print("\nSymbol table")
    for s in syms:
        pp="public" if s[2] else "private"
        print(f"\t{s[0]:04x} {pp:<8s}  {s[1]:<24s}  {s[3]:<24s}")

def print_fragments():
    addrs = list(addrinfo.keys())
    addrs.sort(key = lambda x : x[0])
    print("\nFragment map")
    for rng in addrs:
        (part, frag, m) = addrinfo[rng]
        cseg = frag.segment
        name = frag.name
        if cseg == 'CODE':
            nparts = fraginfo[id(frag)][0]
            if nparts > 1:
                name = name + f" ({part}/{nparts})"
        plen = rng[1] - rng[0]
        if plen > 0:
            blen = f"({plen} byte{'s' if plen > 1 else ''})"
            print(f"\t{rng[0]:04x}-{rng[1]-1:04x} {blen:<14s} {cseg:<5s} {name:<28s} {m.fname:<22s}")


# ------------- main function


def glink(argv):

    '''Main entry point'''
    global lccdir, args, symdefs, module_list
    try:
        # Obtain LCCDIR
        lccdir = os.path.dirname(os.path.realpath(__file__))
        lccdir = os.getenv("LCCDIR", lccdir)

        ## Parse arguments
        parser = argparse.ArgumentParser(
            conflict_handler='resolve',
            usage='glink [options] {<files.o>} -l<lib> -o <outfile.gt1>',
            description='Collects gigatron .{s,o,a} files into a .gt1 file.',
            epilog='''
            	This program accepts the modules generated by
                gigatron-lcc/rcc (suffix .s or .o). These files are
                text files with a python syntax that construct functions
                and data structures that defines all the VCPU instructions,
                labels and data for this module.  Glink also accepts
                concatenation of such files forming a library (suffix
                .a).  The -cpu, -rom, and -map options provide values
                than handcrafted code inside a module can test to
                select different implementations.
                * The -rom option informs the libraries about
                  the availability of natively implemented SYS functions.
                * The -cpu option enables instructions that were added
                  in successive implementations of the Gigatron VCPU.
                  Its default value depends on the -rom option.
                * The -map option tells at which addresses the program,
                  the data, and the stack should be located. It also tells
                  which runtime libraries should be loaded by default.
                  The map argument can be a map name followed by comma
                  separated overlay names. Overlays are python files
                  that tweak the map. These files are searched in
                  the map directory or in the current directory
                  if the overlay name starts with './'.
                The final output file includes the module that exports
                the entry point symbol, then the modules that exports
                all the symbols that it imports, then recursively all
                the modules that are needed to resolve imported
                symbols.''')
        parser.add_argument('files', type=str, nargs='*',
                            help='input files')
        parser.add_argument('-o', type=str, default='a.gt1', metavar='GT1FILE',
                            help='select the output filename (default: a.gt1)')
        parser.add_argument('-cpu', "--cpu", type=int, action='store',
                            help='select the target cpu version: 4, 5, 6.')
        parser.add_argument('-rom', "--rom", type=str, action='store', default='v5a',
                            help='select the target rom version: v4, v5a (default: v5a).')
        parser.add_argument('-map', "--map", type=str, action='store',
                            help='select a linker map')
        parser.add_argument('-info', "--info", action='store_true',
                            help='describe the selected map, cpu, rom')
        parser.add_argument('-V', "--version", action='store_true',
                            help='report glcc/glink version')
        parser.add_argument('-l', type=str, action='append', metavar='LIB',
                            help='library files. -lxxx searches for libxxx.a')
        parser.add_argument('-L', type=str, action='append', metavar='LIBDIR',
                            help='specify an additional directory to search for libraries')
        parser.add_argument('--symbols', '--syms', action='store_const', dest='symbols', const=1,
                            help='outputs a sorted list of symbols')
        parser.add_argument('--all-symbols', '--all-syms', action='store_const', dest='symbols', const=2,
                            help='outputs a sorted list of all symbols, including generated ones')
        parser.add_argument('--fragments', '--frags', action='store_const', dest='fragments', const=2,
                            help='outputs a memory map with all the allocated fragments')
        parser.add_argument('--entry', '-e', dest='e', metavar='START',
                            type=str, action='store', default='_start',
                            help='select the entry point symbol (default _start)')
        parser.add_argument('--gt1-exec-address', dest='gt1exec', metavar='ADDR',
                            type=str, action='store', default='_gt1exec',
                            help='select the gt1 execution address (default _gt1exec)')
        parser.add_argument('-r', '--require', type=str, action='append', dest='r', metavar='SYM',
                            help='enter a symbol as undefined and require it to be resolved by the link')
        parser.add_argument('--onload', type=str, action='append', dest='onload', metavar='SYM',
                            help='define an early initialization function')
        parser.add_argument('--short-function-size-threshold', dest='sfst',
                            metavar='SIZE', type=int, action='store',
                            help='attempts to fit functions smaller than this threshold into a single page.')
        parser.add_argument('--long-function-segment-size', dest='lfss',
                            metavar='SIZE', type=int, action='store',
                            help='minimal segment size for functions split across segments.')
        parser.add_argument('--no-runtime-bss-initialization', action='store_true',
                            help='cause all bss segments to go as zeroes in the gt1 file')
        parser.add_argument('--minimal-heap-segment-size', dest='mhss',
                            metavar='SIZE', type=int, action='store',
                            help='minimal heap segment size for __glink_magic_heap.')
        parser.add_argument('--labelchange-threshold', dest='rpth',
                            metavar='LBLCHG', type=int, action='store', default=200,
                            help='restart a pass whenever the label change counter reach this threshold')
        parser.add_argument('--register-base', dest='regbase', metavar='ADDR',
                            type=lambda x: int(x,0), action='store', default=None,
                            help='set base address of register block')
        parser.add_argument("--mapdir", type=str, action='append', metavar='MAPDIR',
                            help='add directories to search linker maps')
        parser.add_argument('--debug-messages', '-d', dest='d', action='count', default=0,
                            help='enable debugging output. repeat for more.')

        args = parser.parse_args(argv)

        # process args
        read_rominfo(args.rom)
        args.cpu = args.cpu or romcpu or 5
        args.files = args.files or []
        read_interface()
        create_zpage_map()
        create_mulq_map()
        create_register_names(args.regbase)
        args.map = args.map or '32k'
        sm = args.map.split(',')
        args.map = sm[0]
        args.mapdir = args.mapdir or []
        args.mapdir.append(lccdir)
        args.e = args.e or "_start"
        args.l = args.l or []
        args.L = args.L or []
        args.r = args.r or []
        args.onload = args.onload or []
        read_map(args.map, sm[1:])
        args.L.append(os.path.join(lccdir,f"cpu{args.cpu}"))
        args.L.append(lccdir)

        # info request only
        if args.version:
            print(glccver.ver)
            return 0
        elif args.info:
            print('================= ROM INFO')
            if rominfo and romtype and romcpu:
                print(f"  Rom '{args.rom}' (romType={hex(romtype)}) implements cpu {romcpu}")
                print(f"  Keys: {[k for k in rominfo if k not in ('cpu', 'romType')]}")
            else:
                print(f" No information found on rom '{args.rom}'")
            print()
            print('================= CPU INFO')
            if args.cpu == 7:
                print('  vCPU 7 comes with the DEV7 roms and adds new opcodes to vCPU 5.')
            elif args.cpu == 6:
                print('  vCPU 6 comes with at67'"'"'s ROMvX0 and contains many new opcodes\n'
                      ' whose encoding might change from release to release. vCPU 6 is not\n'
                      ' backward compatible with vCPU 5 because it gives different encoding\n'
                      ' to the CMPHU and CMPHS opcodes. Program compiled for vCPU 5 do not\n'
                      ' run reliably on ROMvX0 and programs compiled for vCPU 6 only run\n'
                      ' on ROMvX0. Programs compiled with -rom=vx0 -cpu=5 try to navigate\n'
                      ' these constraints and might run on the dev6 rom (proposed v6)\n'
                      ' and later if one disables the rom check. Your mileage can vary.')
            elif args.cpu == 5:
                print('  vCPU 5 was introduced in ROMv5a with opcodes CALLI, CMPHU, CMPHS.')
            elif args.cpu == 4:
                print('  vCPU 4 is the version that comes with ROMv4.')
            print()
            print('================= MAP INFO')
            if map_describe:
                map_describe()
            else:
                print(f"  No information found on map '{args.map}'")
            return 0

        # load all .s/.o/.a files
        if not args.files:
            fatal(f"no input files were specified")
        for f in args.files:
            read_file(f)
        for m in module_list:
            if m.cpu > args.cpu and not m.library:
                warning(f"module '{m.name}' was compiled for cpu {m.cpu} > {args.cpu}")

        # load modules synthetized by the map
        if map_modules:
            global new_modules
            new_modules = []
            map_modules(romtype)
            module_list += new_modules

        # load libraries requested by the map
        global map_libraries
        if map_libraries:
            for n in map_libraries(romtype):
                read_lib(n)

        # load user-specified libraries
        for f in args.l:
            read_lib(f)

        # resolve import/exports/common and prune unused modules
        module_list = compute_closure()
        convert_common_symbols()
        check_undefined_symbols()
        if error_counter > 0:
            print(f"glink: {error_counter} error(s) {warning_counter} warning(s)")
            return 1

        # generate
        run_passes()
        if error_counter > 0:
            print(f"glink: {error_counter} error(s) {warning_counter} warning(s)")
            return 1

        # magic happens here
        process_magic_symbols()

        # verification
        for s in segment_list:
            if s.pc > s.eaddr:
                fatal(f"internal error: segment overflow in {s} (final pc={hex(s.pc)})")

        # output
        save_gt1(args.o, args.gt1exec)
        if args.symbols:
            print_symbols(allsymbols=args.symbols>1)
        if args.fragments:
            print_fragments()
        return 0

    except FileNotFoundError as err:
        fatal(str(err), exc=True)
    except Exception as err:
        fatal(repr(err), exc=True)


if __name__ == '__main__':
    sys.exit(glink(sys.argv[1:]))

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
