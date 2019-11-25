from copy import copy
from re import findall
import sys

global_labels = {
    'vPC': 0x0016,
    'vAC': 0x0018,
    'vACH': 0x0019,
    'vLR': 0x001a,
    'vLRH': 0x001b,
    'sysFn': 0x0022,
    'r1': 0x0030,
    'r2': 0x0032,
    'r3': 0x0034,
    'r4': 0x0036,
    'r5': 0x0038,
    'r6': 0x003a,
    'r7': 0x003c,
    'r8': 0x003e,
    'r9': 0x0040,
    'r10': 0x0042,
    'r11': 0x0044,
    'r12': 0x0046,
    'r13': 0x0048,
    'r14': 0x004a,
    'r15': 0x004c,
    'SYS_LSRW1_48' : 0x0600,
}

class Log:
    def __init__(self, f):
        self.f = f

log = Log(sys.stderr)

class Segment:
    def __init__(self, address, size):
        self.address = address
        self.size = size
        self.buffer = bytearray()
        self.relocs = {}

    def pc(self):
        return self.address + len(self.buffer)

    def remaining(self):
        return self.size - len(self.buffer)

    def emit(self, data):
        assert(len(self.buffer) + len(data) <= self.size)
        self.buffer += data

    def emitb(self, opcode, operand):
        assert(operand >= -128 and operand < 256)
        self.emit(bytes([opcode, operand]))

    def emitw(self, opcode, operand):
        assert(operand >= -32768 and operand < 65536)
        self.emit(bytes([opcode, operand & 0xff, (operand >> 8) & 0xff]))

    def reloc(self, addr, symbol):
        assert(addr >= self.address and addr < self.address + self.size)
        self.relocs[addr - self.address] = symbol

    def write(self, stream):
        if len(self.buffer) != 0:
            print(f'writing segment {self.address:x}:{self.pc():x}', file=log.f)
            stream.write(bytes([self.address >> 8 & 0xff, self.address & 0xff, len(self.buffer) & 0xff]))
            stream.write(self.buffer)

def prev(address, step=2):
    """Subtract 2 while staying in the same page

      This is needed for target calculations because vCPU always
      increments [vPC] by 2 *before* fetching the next opcode."""
    return (address & 0xff00) | ((address - step) & 0x00ff)

class Inst:
    def __init__(self, opcode, operand, size, branch, emit):
        self.addr = None
        self.opcode = opcode
        self.operand = operand
        self.size = size
        self.branch = branch
        self._emit = emit

    def emit(self, segment):
        self._emit(self, segment)

    def emitjcc(self, segment, near, far):
        # 'near' is the condition code for local branches, 'far' is its inverse condition
        if self.operand & 0xff00 != self.addr & 0xff00:
            # far jump
            assert(self.size == 8)
            print(f'emitting far branch from {self.addr:x} to {self.operand:x}', file=log.f)
            skip = prev(self.addr, step=2-8);
            segment.emit(bytes([0x35, far, skip & 0xff]))               # BCC <far> <skip>
            segment.emitw(0x11, prev(self.operand))                     # LDWI <target>
            segment.emitb(0xf3, global_labels['pvpc'])                  # DOKE pvpc
        else:
            # near jump
            assert(self.size == 3)
            print(f'emitting near branch from {self.addr:x} to {self.operand:x}', file=log.f)
            segment.emit(bytes([0x35, near, prev(self.operand) & 0xff]))# BCC <near> <target>

    def emitj(self, segment):
        if self.operand & 0xff00 == self.addr & 0xff00:
            print(f'emitting near jump from {self.addr:x} to {self.operand:x}', file=log.f)
            segment.emitb(0x90, prev(self.operand) & 0xff)              # BRA <target>
        else:
            print(f'emitting far jump from {self.addr:x} to {self.operand:x}', file=log.f)
            Inst.ldwi(prev(self.operand)).emit(segment)
            segment.emitb(0xf3, global_labels['pvpc'])                  # DOKE pvpc

    @staticmethod
    def glob(name): return Inst('glob', name, 0, False, lambda i, s: None)
    @staticmethod
    def label(name): return Inst('label', name, 0, False, lambda i, s: None)
    @staticmethod
    def ldwi(con): return Inst('ldwi', con, 3, False, lambda i, s: s.emitw(0x11, i.operand))
    @staticmethod
    def ld(d): return Inst('ld', d, 2, False, lambda i, s: s.emitb(0x1a, i.operand))
    @staticmethod
    def ldw(d): return Inst('ldw', d, 2, False, lambda i, s: s.emitb(0x21, i.operand))
    @staticmethod
    def stw(d): return Inst('stw', d, 2, False, lambda i, s: s.emitb(0x2b, i.operand))
    @staticmethod
    def ldlw(d): return Inst('ldlw', d, 2, False, lambda i, s: s.emitb(0xee, i.operand))
    @staticmethod
    def stlw(d): return Inst('stlw', d, 2, False, lambda i, s: s.emitb(0xec, i.operand))
    @staticmethod
    def alloc(d): return Inst('alloc', d, 2, False, lambda i, s: s.emitb(0xdf, i.operand))
    @staticmethod
    def jeq(l): return Inst('jeq', l, 8, True, lambda i, s: i.emitjcc(s, 0x3f, 0x72))
    @staticmethod
    def jne(l): return Inst('jne', l, 8, True, lambda i, s: i.emitjcc(s, 0x72, 0x3f))
    @staticmethod
    def jge(l): return Inst('jge', l, 8, True, lambda i, s: i.emitjcc(s, 0x53, 0x50))
    @staticmethod
    def jgt(l): return Inst('jgt', l, 8, True, lambda i, s: i.emitjcc(s, 0x4d, 0x56))
    @staticmethod
    def jle(l): return Inst('jle', l, 8, True, lambda i, s: i.emitjcc(s, 0x56, 0x4d))
    @staticmethod
    def jlt(l): return Inst('jlt', l, 8, True, lambda i, s: i.emitjcc(s, 0x50, 0x53))
    @staticmethod
    def ldi(con): return Inst('ldi', con, 2, False, lambda i, s: s.emitb(0x59, i.operand))
    @staticmethod
    def st(d): return Inst('st', d, 2, False, lambda i, s: s.emitb(0x5e, i.operand))
    @staticmethod
    def pop(): return Inst('pop', None, 1, False, lambda i, s: s.emit(bytes([0x63])))
    @staticmethod
    def popret(): return Inst('popret', None, 2, False, lambda i, s: s.emit(bytes([0x63, 0xff])))
    @staticmethod
    def push(): return Inst('push', None, 1, False, lambda i, s: s.emit(bytes([0x75])))
    @staticmethod
    def lup(d): return Inst('lup', d, 2, False, lambda i, s: s.emitb(0x7f, i.operand))
    @staticmethod
    def andi(con): return Inst('andi', con, 2, False, lambda i, s: s.emitb(0x82, i.operand))
    @staticmethod
    def ori(con): return Inst('ori', con, 2, False, lambda i, s: s.emitb(0x88, i.operand))
    @staticmethod
    def xori(con): return Inst('xori', con, 2, False, lambda i, s: s.emitb(0x8c, i.operand))
    @staticmethod
    def j(l): return Inst('j', l, 5, False, lambda i, s: i.emitj(s))
    @staticmethod
    def jr(): return Inst('jr', None, 2, False, lambda i, s: s.emitb(0xf3, global_labels['pvpc']))
    @staticmethod
    def inc(d): return Inst('inc', d, 2, False, lambda i, s: s.emitb(0x93, i.operand))
    @staticmethod
    def addw(d): return Inst('addw', d, 2, False, lambda i, s: s.emitb(0x99, i.operand))
    @staticmethod
    def peek(): return Inst('peek', None, 1, False, lambda i, s: s.emit(bytes([0xad])))
    @staticmethod
    def sys(con): return Inst('sys', con, 2, False, lambda i, s: s.emitb(0xb4, i.operand))
    @staticmethod
    def subw(d): return Inst('subw', d, 2, False, lambda i, s: s.emitb(0xb8, i.operand))
    @staticmethod
    def call(d): return Inst('call', d, 2, False, lambda i, s: s.emitb(0xcf, i.operand))
    @staticmethod
    def addi(con): return Inst('addi', con, 2, False, lambda i, s: s.emitb(0xe3, i.operand))
    @staticmethod
    def subi(con): return Inst('subi', con, 2, False, lambda i, s: s.emitb(0xe6, i.operand))
    @staticmethod
    def lslw(): return Inst('lslw', None, 1, False, lambda i, s: s.emit(bytes([0xe9])))
    @staticmethod
    def poke(d): return Inst('poke', d, 2, False, lambda i, s: s.emitb(0xf0, i.operand))
    @staticmethod
    def doke(d): return Inst('doke', d, 2, False, lambda i, s: s.emitb(0xf3, i.operand))
    @staticmethod
    def deek(): return Inst('deek', None, 1, False, lambda i, s: s.emit(bytes([0xf6])))
    @staticmethod
    def andw(d): return Inst('andw', d, 2, False, lambda i, s: s.emitb(0xf8, i.operand))
    @staticmethod
    def orw(d): return Inst('orw', d, 2, False, lambda i, s: s.emitb(0xfa, i.operand))
    @staticmethod
    def xorw(d): return Inst('xorw', d, 2, False, lambda i, s: s.emitb(0xfc, i.operand))
    @staticmethod
    def ret(): return Inst('ret', None, 1, False, lambda i, s: s.emit(bytes([0xff])))
    @staticmethod
    def db(con): return Inst('db', con, 1, False, lambda i, s: s.emit(bytes([i.operand])))
    @staticmethod
    def dw(con): return Inst('dw', con, 2, False, lambda i, s: s.emit(bytes([i.operand & 0xff, (i.operand >> 8) & 0xff])))
    @staticmethod
    def dx(x): return Inst('dx', x, len(x), False, lambda i, s: s.emit(bytes(x)))
    @staticmethod
    def dc(l): return Inst('dc', l, 2, False, lambda i, s: s.emit(bytes([prev(i.operand) & 0xff, prev(i.operand) >> 8])))
    @staticmethod
    def dl(l): return Inst('dl', l, sum([i.size for i in l]), False, None)

functions = {}
func = None

def defun(name):
    global func
    func = []
    functions[name] = func

def glob(name): func.append(Inst.glob(name))
def label(name): func.append(Inst.label(name))
def ldwi(con): func.append(Inst.ldwi(con))
def ld(d): func.append(Inst.ld(d))
def ldw(d): func.append(Inst.ldw(d))
def stw(d): func.append(Inst.stw(d))
def ldlw(d): func.append(Inst.ldlw(d))
def stlw(d): func.append(Inst.stlw(d))
def alloc(d): func.append(Inst.alloc(d))
def jeq(l): func.append(Inst.jeq(l))
def jne(l): func.append(Inst.jne(l))
def jge(l): func.append(Inst.jge(l))
def jgt(l): func.append(Inst.jgt(l))
def jle(l): func.append(Inst.jle(l))
def jlt(l): func.append(Inst.jlt(l))
def ldi(con): func.append(Inst.ldi(con))
def st(d): func.append(Inst.st(d))
def pop(): func.append(Inst.pop())
def popret(): func.append(Inst.popret())
def push(): func.append(Inst.push())
def lup(d): func.append(Inst.lup(d))
def andi(con): func.append(Inst.andi(con))
def ori(con): func.append(Inst.ori(con))
def xori(con): func.append(Inst.xori(con))

def j(l): func.append(Inst.j(l))

def jr():
    # Check for a preceding ldwi. If one exists, snip it out and create a 'j' instead of a 'jr'.
    if len(func) > 0 and func[len(func)-1].opcode == 'ldwi':
        func[len(func)-1] = Inst.j(func[len(func)-1].operand)
    else:
        func.append(Inst.jr())

def inc(d): func.append(Inst.inc(d))
def addw(d): func.append(Inst.addw(d))
def peek(): func.append(Inst.peek())
def sys(con): func.append(Inst.sys(con))
def subw(d): func.append(Inst.subw(d))
def call(d): func.append(Inst.call(d))
def addi(con): func.append(Inst.addi(con))
def subi(con): func.append(Inst.subi(con))
def lslw(): func.append(Inst.lslw())
def poke(d): func.append(Inst.poke(d))
def doke(d): func.append(Inst.doke(d))
def deek(): func.append(Inst.deek())
def andw(d): func.append(Inst.andw(d))
def orw(d): func.append(Inst.orw(d))
def xorw(d): func.append(Inst.xorw(d))
def ret(): func.append(Inst.ret())
def db(con): func.append(Inst.db(con))
def dw(con): func.append(Inst.dw(con))
def dx(x): func.append(Inst.dx(x))
def dc(l): func.append(Inst.dc(l))

def link(entry, outf, logf):
    log.f = logf

    # Before laying out any functions, garbage collect those that are not used.
    marked = {'@globals', '@thunk0', '@thunk1', '@thunk2', entry}
    for name, func in functions.items():
        if name == '@globals':
            continue

        labels = set()
        for inst in func:
            if inst.opcode == 'label' or inst.opcode == 'glob':
                labels.add(inst.operand)
            elif type(inst.operand) is str and inst.operand not in labels:
                marked.add(inst.operand)

    for name in list(functions.keys()):
        if name[0] == '@':
            if name not in marked and name[1:] not in marked:
                print(f'removing function {name}', file=log.f)
                functions[name] = []
        elif name not in marked:
            print(f'removing function {name}', file=log.f)
            del functions[name]

    # After garbage collection, coalesce adjacent data instructions into contiguous lists. This is necessary in
    # order to avoid arrays being split across segment discontinuities.
    for name, func in functions.items():
        result = []

        coalesced = None
        for inst in func:
            if inst.opcode in { 'db', 'dw', 'dx', 'dc' }:
                if coalesced is None:
                    coalesced = []
                coalesced.append(inst)
            else:
                if coalesced is not None:
                    result.append(Inst.dl(coalesced))
                    coalesced = None
                result.append(inst)
        if coalesced is not None:
            result.append(Inst.dl(coalesced))
        functions[name] = result

    # Set up the segment map.
    segments = [
        Segment(0x004e, 0x32),
        Segment(0x0200, 0xfa),
        Segment(0x0300, 0xfa),
        Segment(0x0400, 0xfa),
    ]
    for i in range(0, 0x80-0x8):
        segments.append(Segment(0x08a0 + (i << 8), 96))

    # lay out the current function
    # first set its offset
    # then do its initial layout:
    # - assume all jumps are far unless they are provably close
    #     - basically, all forward jumps must be far jumps if the function may cross a page boundary
    # - place page fallthough code as necessary
    # then attempt to shorten jumps and iterate until reaching a fixed point

    labels = {}

    def deflabel(inst, pc):
        assert inst.opcode == 'label' or inst.opcode == 'glob'
        print(f'defining label {inst.operand}', file=log.f)
        inst.addr = pc
        labels[inst.operand] = pc
        if inst.opcode == 'glob':
            print(f'defining global label {inst.operand}', file=log.f)
            global_labels[inst.operand] = pc

    def near(target, pc):
        if type(target) is str:
            target = labels.get(target)
        return target is not None and target & 0xff00 == pc & 0xff00

    def shorten(inst, pc):
        if not inst.branch and inst.opcode != 'j':
            return 0

        target, oldsize = inst.operand, inst.size
        if type(target) is str:
            target = labels.get(target)
        near = target is not None and target & 0xff00 == pc & 0xff00

        if inst.branch:
            if near:
                print(f'near branch from {pc:x} to {target:x}', file=log.f)
                inst.size = 3
            else:
                print(f'far branch from {pc:x} to {0 if target is None else target:x}', file=log.f)
                inst.size = 8
        else:
            if near:
                print(f'near jump from {pc:x} to {target:x}', file=log.f)
                inst.size = 2
            else:
                print(f'far jump from {pc:x} to {0 if target is None else target:x}', file=log.f)
                inst.size = 5
        return oldsize - inst.size

    def calctarget(symbolexpr, symboltable):
        """Resolve symbol or symbol expression such as '.L99+100-1'"""
        terms = findall(r'[^+-]+|[+-][0-9]+', symbolexpr)
        if terms[0] in symboltable:
            return symboltable[terms[0]] + sum([int(t) for t in terms[1:]])
        else:
            return None

    def resolve_operand(inst, seg, pc):
        if type(inst.operand) is str:
            target = calctarget(inst.operand, labels)
            if target is not None:
                inst.operand = target
            elif inst.opcode == 'ldwi':
                seg.reloc(pc + 1, inst.operand)
                inst.operand = 0 # Operand goes through relocs{} from here
            else:
                assert(inst.opcode == 'dw')
                seg.reloc(pc, inst.operand)
                inst.operand = 0 # Operand goes through relocs{} from here

    def layout(seg, sidx, func, emitting, name):
        todo = sum(ins.size for ins in func)
        startpc, remaining = seg.pc(), seg.remaining()
        pending_labels = []
        changed, has_push, expect_pop, ret_is_safe = False, False, False, True

        if remaining <= 2:
            sidx += 1
            seg = segments[sidx]
            startpc, remaining = seg.pc(), seg.remaining()
        assert remaining > 2
        pc = startpc

        for i in range(0, len(func)):
            inst = func[i]

            if inst.opcode == 'label' or inst.opcode == 'glob':
                pending_labels.append(inst)
                continue

            # Define any labels that precede this instruction and attempt branch shortening.
            for l in pending_labels:
                deflabel(l, pc)
            todo -= shorten(inst, pc)

            # If there is not enough space remaining for this instruction and a page thunk call,
            # jump to the next page (unless there is enough space for the rest of the instructions).
            # This also ensures that any final 'POP+RET' pair won't be split.
            if inst.size + 2 > remaining and todo > remaining:
                # If this is segment 0, fail.
                assert(sidx != 0)

                sidx += 1
                nextseg = segments[sidx]

                print(f'moving to segment {sidx} @ {nextseg.pc():x}', file=log.f)
                if emitting:
                    if not has_push:
                        print('Warning: unsafe thunk CALL in', name)
                    if nextseg.pc() == 0x08a0:
                        Inst.call(global_labels['thunk2']).emit(seg)
                    elif nextseg.pc() & 0xff == 0:
                        Inst.call(global_labels['thunk0']).emit(seg)
                    else:
                        assert(nextseg.pc() & 0xff == 0xa0)
                        Inst.call(global_labels['thunk1']).emit(seg)
                    ret_is_safe = False

                seg = nextseg
                pc, remaining = seg.pc(), seg.remaining()

                # Redefine any labels that precede this instruction and retry branch shortening since the PC
                # has changed.
                for l in pending_labels:
                    deflabel(l, pc)
                todo -= shorten(inst, pc)

            pending_labels = []

            if inst.addr != pc:
                changed = True
            inst.addr = pc

            if emitting:
                if inst.opcode == 'dl':
                    ipc = pc
                    for i in inst.operand:
                        resolve_operand(i, seg, ipc)
                        i.emit(seg)
                        ipc += i.size
                else:
                    resolve_operand(inst, seg, pc)
                    inst.emit(seg)

                # Simple heuristic to check for vLR clobbering
                if inst.opcode == 'push':
                    has_push, expect_pop = True, True
                if inst.opcode == 'call':
                    ret_is_safe = False
                if inst.opcode in ['pop', 'popret']:
                    if not has_push:
                        print('Warning: POP without PUSH in', name)
                    expect_pop, ret_is_safe = False, True
                if inst.opcode in ['ret', 'popret']:
                    if expect_pop:
                        print('Warning: PUSH without POP in', name)
                    if not ret_is_safe:
                        print('Warning: unsafe RET in', name)
                    ret_is_safe, expect_pop = True, has_push

            pc += inst.size
            remaining -= inst.size
            todo -= inst.size

        # define any remaining labels
        for l in pending_labels:
            deflabel(l, pc)

        return (startpc, pc, changed, seg, sidx)

    def dofunc(seg, sidx, func, name):
        print(f'laying out function {name}', file=log.f)
        while True:
            startpc, _, changed, _, _ = layout(seg, sidx, func, False, None)
            if not changed:
                break
        print(f'emitting function {name} @ {startpc:x}', file=log.f)
        startpc, _, _, seg, sidx = layout(seg, sidx, func, True, name)
        return startpc, seg, sidx

    # The linker treats "@globals" as a special function. This is the only function that can live in the zero
    # page, and must consist of static data only.
    if '@globals' in functions:
        labels = copy(global_labels)
        dofunc(segments[0], 0, functions['@globals'], '@globals')
        del functions['@globals']

    funclabels = {}
    segment, sidx = segments[1], 1
    for name, f in functions.items():
        if len(f) > 0 and f[0].opcode == 'dl':
            while segment.remaining() < f[0].size:
                sidx += 1
                segment = segments[sidx]
        else:
            while segment.remaining() < 2:
                sidx += 1
                segment = segments[sidx]

        labels = copy(global_labels)
        pc, segment, sidx = dofunc(segment, sidx, f, name)
        funclabels[name] = pc

    for s in segments:
        for offset, label in s.relocs.items():
            target = calctarget(label, funclabels)
            if target is None:
                print(f'Undefined symbol: {label}')
                sys.exit(1)
            print(f'reloc: {label} -> {target} @ {offset}', file=log.f)
            s.buffer[offset] = target & 0xff
            s.buffer[offset + 1] = (target >> 8) & 0xff
        s.write(outf)

    start = funclabels[entry]
    outf.write(bytes([0x00, (start >> 8) & 0xff, start & 0xff]))
