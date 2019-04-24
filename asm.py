from copy import copy
from sys import stderr

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
}

class Log:
    def __init__(self, f):
        self.f = f

log = Log(stderr)

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
            stream.write(bytes([self.address >> 8 & 0xff, self.address & 0xff, len(self.buffer)]))
            stream.write(self.buffer)

def displacement(operand):
    operand = operand & 0xff
    return operand - 2 if operand >= 2 else 254 + operand

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
        if self.operand & 0xff00 != self.addr & 0xff00:
            # far jump
            assert(self.size == 8)
            print(f'emitting far branch from {self.addr:x} to {self.operand:x}', file=log.f)
            skip = self.addr + 8
            segment.emit(bytes([0x35, far, displacement(skip)])) # bcc <far> <skip>
            segment.emitw(0x11, self.operand - 2)         # ldwi <target>
            segment.emitb(0xf3, global_labels['pvpc'])    # doke vpc
        else:
            # near jump
            assert(self.size == 3)
            print(f'emitting near branch from {self.addr:x} to {self.operand:x}', file=log.f)
            segment.emit(bytes([0x35, near, displacement(self.operand)]))

    def emitj(self, segment):
        if self.operand & 0xff00 == self.addr & 0xff00:
            print(f'emitting near jump from {self.addr:x} to {self.operand:x}', file=log.f)
            segment.emitb(0x90, displacement(self.operand))
        else:
            print(f'emitting far jump from {self.addr:x} to {self.operand:x}', file=log.f)
            Inst.ldwi(self.operand - 2).emit(segment)
            segment.emitb(0xf3, global_labels['pvpc'])

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
def jeq(l): func.append(Inst.jeq(l))
def jne(l): func.append(Inst.jne(l))
def jge(l): func.append(Inst.jge(l))
def jgt(l): func.append(Inst.jgt(l))
def jle(l): func.append(Inst.jle(l))
def jlt(l): func.append(Inst.jlt(l))
def ldi(con): func.append(Inst.ldi(con))
def st(d): func.append(Inst.st(d))
def pop(): func.append(Inst.pop())
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

    # Set up the segment map.
    segments = [
        Segment(0x004e, 0x32),
        Segment(0x0200, 0xfa),
        Segment(0x0300, 0xfa),
        Segment(0x0400, 0xfa),
    ]
    for i in range(0, 0x80-0x8):
        segments.append(Segment(0x08a0 + (i << 8), 96))

    # TODO: function-granularity DCE
    #
    # lay out the current function
    # first set its offset
    # then do its initial layout:
    # - assume all jumps are far unless they are provably close
    #     - basically, all forward jumps must be far jumps if the function may cross a page boundary
    # - place page fallthough code as necessary
    # then attempt to shorten jumps and iterate until reaching a fixed point

    labels = {}

    def near(target, pc):
        if type(target) is str:
            target = labels.get(target)
        return target is not None and target & 0xff00 == pc & 0xff00

    def shorten(inst, pc):
        if not inst.branch and inst.opcode != 'j':
            return

        target = inst.operand
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

    def layout(seg, sidx, func, emitting):
        pc, remaining = seg.pc(), seg.remaining()
        changed = False
        for i in range(0, len(func)):
            inst = func[i]

            if inst.opcode == 'label' or inst.opcode == 'glob':
                inst.addr = pc
                labels[inst.operand] = pc
                if inst.opcode == 'glob':
                    print(f'defining global label {inst.operand}', file=log.f)
                    global_labels[inst.operand] = pc
                continue

            # if this is a branch, we may be able to shorten it. check for that here.
            shorten(inst, pc)

            # if there is not enough space remaining for this instruction and a page thunk call, jump to the next
            # page.
            if remaining < inst.size + 2:
                # if there is enough space for the rest of the instructions, we're ok.
                nr = remaining - inst.size
                if nr >= 0 and sum(ins.size for ins in func[i+1:]) <= nr:
                    pass
                else:
                    # If this is segment 0, fail.
                    assert(sidx != 0)

                    sidx += 1
                    nextseg = segments[sidx]

                    print(f'moving to segment {sidx} @ {nextseg.pc():x}', file=log.f)
                    if emitting:
                        if nextseg.pc() == 0x08a0:
                            Inst.call(global_labels['thunk2']).emit(seg)
                        elif nextseg.pc() & 0xff == 0:
                            Inst.call(global_labels['thunk0']).emit(seg)
                        else:
                            assert(nextseg.pc() & 0xff == 0xa0)
                            Inst.call(global_labels['thunk1']).emit(seg)

                    seg = nextseg
                    pc, remaining = seg.pc(), seg.remaining()
                    shorten(inst, pc)

            if inst.addr != pc:
                changed = True
            inst.addr = pc

            if emitting:
                if type(inst.operand) is str:
                    if inst.operand in labels:
                        inst.operand = labels[inst.operand]
                    elif inst.opcode == 'ldwi':
                        seg.reloc(pc + 1, inst.operand)
                        inst.operand = 0x102e
                    else:
                        print(f'{inst.opcode} {inst.operand}', file=log.f)
                        assert(inst.opcode == 'dw')
                        seg.reloc(pc, inst.operand)
                        inst.operand = 0x102e
                inst.emit(seg)

            pc += inst.size
            remaining -= inst.size

        return (pc, changed, seg, sidx)

    def dofunc(seg, sidx, func, name):
        print(f'laying out function {name}', file=log.f)
        while True:
            _, changed, _, _ = layout(seg, sidx, func, False)
            if not changed:
                break
        pc = seg.pc()
        print(f'emitting function {name} @ {pc:x}', file=log.f)
        _, _, seg, sidx = layout(seg, sidx, func, True)
        return pc, seg, sidx

    # The linker treats "@globals" as a special function. This is the only function that can live in the zero
    # page, and must consist of static data only.
    if '@globals' in functions:
        labels = copy(global_labels)
        dofunc(segments[0], 0, functions['@globals'], '@globals')
        del functions['@globals']

    funclabels = {}
    segment, sidx = segments[1], 1
    for name, f in functions.items():
        while segment.remaining() == 0:
            sidx += 1
            segment = segments[sidx]

        labels = copy(global_labels)
        pc, segment, sidx = dofunc(segment, sidx, f, name)
        funclabels[name] = pc

    for s in segments:
        for offset, label in s.relocs.items():
            target = funclabels[label]
            print(f'reloc: {label} -> {target:x} @ {offset:x}', file=log.f)
            s.buffer[offset] = target & 0xff
            s.buffer[offset + 1] = (target >> 8) & 0xff
        s.write(outf)

    start = funclabels[entry]
    outf.write(bytes([0x00, (start >> 8) & 0xff, start & 0xff]))
