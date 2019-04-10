from copy import copy
from sys import stdout, stderr

global_labels = {
    'vPC': 0x0016,
    'vAC': 0x0018,
    'vACH': 0x0019,
    'vLR': 0x001a,
    'vLRH': 0x001b,
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
    'ha': 0x004e,
    'pvpc': 0x0050,
    'ldloc': 0x0052,
    'stloc': 0x0054,
    'thunk0': 0x0056,
    'lsh': 0x0058,
    'rsh': 0x005a,
    'mul': 0x005c,
    'mod': 0x005e,
    'div': 0x0060,
    'sp': 0x0062,
    'ht': 0x0064,
    'thunk1': 0x0066,
    'enter': 0x0068,
    'leave': 0x006a,
}

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
            stream.write(bytes([self.address >> 8 & 0xff, self.address & 0xff, len(self.buffer)]))
            stream.write(self.buffer)

functions = {}
func = None

def defun(name):
    global func
    func = []
    functions[name] = func

def displacement(operand):
    operand = operand & 0xff
    return operand - 2 if operand >= 2 else 254 + operand

def emitjcc(segment, inst, near, far):
    if inst.operand & 0xff00 != inst.addr & 0xff00:
        # far jump
        assert(inst.size == 8)
        print(f'emitting far branch from {inst.addr:x} to {inst.operand:x}', file=stderr)
        skip = inst.addr + 8
        segment.emit(bytes([0x35, far, displacement(skip)])) # bcc <far> <skip>
        segment.emitw(0x11, inst.operand)             # ldwi <target>
        segment.emitb(0xf3, global_labels['pvpc'])    # doke vpc
    else:
        # near jump
        assert(inst.size == 3)
        print(f'emitting near branch from {inst.addr:x} to {inst.operand:x}', file=stderr)
        segment.emit(bytes([0x35, near, displacement(inst.operand)]))

def label(name):
    func.append(Inst('label', name, 0, False, lambda i, s: None))

def ldwi(con):
    func.append(Inst('ldwi', con, 3, False, lambda i, s: s.emitw(0x11, i.operand)))

def ld(d):
    func.append(Inst('ld', d, 2, False, lambda i, s: s.emitb(0x1a, i.operand)))

def ldw(d):
    func.append(Inst('ldw', d, 2, False, lambda i, s: s.emitb(0x21, i.operand)))

def stw(d):
    func.append(Inst('stw', d, 2, False, lambda i, s: s.emitb(0x2b, i.operand)))

def jeq(l):
    func.append(Inst('jeq', l, 8, True, lambda i, s: emitjcc(s, i, 0x3f, 0x72)))

def jne(l):
    func.append(Inst('jne', l, 8, True, lambda i, s: emitjcc(s, i, 0x72, 0x3f)))

def jge(l):
    func.append(Inst('jge', l, 8, True, lambda i, s: emitjcc(s, i, 0x53, 0x50)))

def jgt(l):
    func.append(Inst('jgt', l, 8, True, lambda i, s: emitjcc(s, i, 0x4d, 0x56)))

def jle(l):
    func.append(Inst('jle', l, 8, True, lambda i, s: emitjcc(s, i, 0x56, 0x4d)))

def jlt(l):
    func.append(Inst('jlt', l, 8, True, lambda i, s: emitjcc(s, i, 0x50, 0x53)))

def ldi(con):
    func.append(Inst('ldi', con, 2, False, lambda i, s: s.emitb(0x59, i.operand)))

def st(d):
    func.append(Inst('st', d, 2, False, lambda i, s: s.emitb(0x5e, i.operand)))

def pop():
    func.append(Inst('pop', None, 1, False, lambda i, s: s.emit(bytes([0x63]))))

def push():
    func.append(Inst('push', None, 1, False, lambda i, s: s.emit(bytes([0x75]))))

def lup(d):
    func.append(Inst('lup', d, 2, False, lambda i, s: s.emitb(0x7f, i.operand)))

def andi(con):
    func.append(Inst('andi', con, 2, False, lambda i, s: s.emitb(0x82, i.operand)))

def ori(con):
    func.append(Inst('ori', con, 2, False, lambda i, s: s.emitb(0x88, i.operand)))

def xori(con):
    func.append(Inst('xori', con, 2, False, lambda i, s: s.emitb(0x8c, i.operand)))

def jr():
    # Check for a preceding ldwi. If one exists, snip it out and create a 'j' instead of a 'jr'.
    if len(func) > 0 and func[len(func)-1].opcode == 'ldwi':
        def e(inst, segment):
            if inst.operand & 0xff00 == inst.addr & 0xff00:
                print(f'emitting near jump from {inst.addr:x} to {inst.operand:x}', file=stderr)
                segment.emitb(0x90, displacement(inst.operand))
            else:
                # TODO: does the operand need to be adjusted? probably.
                print(f'emitting far jump from {inst.addr:x} to {inst.operand:x}', file=stderr)
                ldwi(inst.operand).emit(segment)
                segment.emitb(0xf3, global_labels['pvpc'])
        func[len(func)-1] = Inst('j', func[len(func)-1].operand, 5, False, e)
    else:
        func.append(Inst('jr', None, 2, False, lambda i, s: s.emitb(0xf3, global_labels['pvpc'])))

def inc(d):
    func.append(Inst('inc', d, 2, False, lambda i, s: s.emitb(0x93, i.operand)))

def addw(d):
    func.append(Inst('addw', d, 2, False, lambda i, s: s.emitb(0x99, i.operand)))

def peek():
    func.append(Inst('peek', None, 1, False, lambda i, s: s.emit(bytes([0xad]))))

def sys(con):
    func.append(Inst('sys', con, 2, False, lambda i, s: s.emitb(0xb4, i.operand)))

def subw(d):
    func.append(Inst('subw', d, 2, False, lambda i, s: s.emitb(0xb8, i.operand)))

def call(d):
    func.append(Inst('call', d, 2, False, lambda i, s: s.emitb(0xcf, i.operand)))

def addi(con):
    func.append(Inst('addi', con, 2, False, lambda i, s: s.emitb(0xe3, i.operand)))

def subi(con):
    func.append(Inst('subi', con, 2, False, lambda i, s: s.emitb(0xe6, i.operand)))

def lslw():
    func.append(Inst('lslw', None, 1, False, lambda i, s: s.emit(bytes([0xe9]))))

def poke(d):
    func.append(Inst('poke', d, 2, False, lambda i, s: s.emitb(0xf0, i.operand)))

def doke(d):
    func.append(Inst('doke', d, 2, False, lambda i, s: s.emitb(0xf3, i.operand)))

def deek():
    func.append(Inst('deek', None, 1, False, lambda i, s: s.emit(bytes([0xf6]))))

def andw(d):
    func.append(Inst('andw', d, 2, False, lambda i, s: s.emitb(0xf8, i.operand)))

def orw(d):
    func.append(Inst('orw', d, 2, False, lambda i, s: s.emitb(0xfa, i.operand)))

def xorw(d):
    func.append(Inst('xorw', d, 2, False, lambda i, s: emnitb(0xfc, i.operand)))

def ret():
    func.append(Inst('ret', None, 1, False, lambda i, s: s.emit(bytes([0xff]))))

def link(entry):
    # Set up the segment map.
    segments = [
        Segment(0x200, 0xfa),
        Segment(0x300, 0xfa),
        Segment(0x400, 0xfa),
        Segment(0x500, 0x100),
        Segment(0x600, 0x100),
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
        target = inst.operand
        if type(target) is str:
            target = labels.get(target)
        near = target is not None and target & 0xff00 == pc & 0xff00

        if inst.branch:
            if near:
                print(f'near branch from {pc:x} to {target:x}', file=stderr)
                inst.size = 3
            else:
                print(f'far branch from {pc:x} to {0 if target is None else target:x}', file=stderr)
                inst.size = 8
        elif inst.opcode == 'j':
            if near:
                print(f'near jump from {pc:x} to {target:x}', file=stderr)
                inst.size = 2
            else:
                print(f'far jump from {pc:x} to {0 if target is None else target:x}', file=stderr)
                inst.size = 5

    def layout(seg, sidx, func, emitting):
        pc, remaining = seg.pc(), seg.remaining()
        changed = False
        for i in range(0, len(func)):
            inst = func[i]

            if inst.opcode == 'label':
                inst.addr = pc
                labels[inst.operand] = pc
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
                    sidx += 1
                    nextseg = segments[sidx]

                    print(f'moving to segment {sidx} @ {nextseg.pc()}', file=stderr)
                    if emitting:
                        if pc & 0xff == 0:
                            call('thunk0').emit(seg)
                        else:
                            assert(pc & 0xff == 0xa0)
                            call('thunk1').emit(seg)

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
                    else:
                        assert(inst.opcode == 'ldwi')
                        seg.reloc(pc + 1, inst.operand)
                        inst.operand = 0x102e
                inst.emit(seg)

            pc += inst.size
            remaining -= inst.size

        return (pc, changed, seg, sidx)

    def dofunc(seg, sidx, func):
        while True:
            _, changed, _, _ = layout(seg, sidx, func, False)
            if not changed:
                break
        pc = seg.pc()
        _, _, seg, sidx = layout(seg, sidx, func, True)
        return pc, seg, sidx

    funclabels = {}
    segment, sidx = segments[0], 0
    for name, f in functions.items():
        while segment.remaining() == 0:
            sidx += 1
            segment = segments[sidx]

        labels = copy(global_labels)
        pc, segment, sidx = dofunc(segment, sidx, f)
        funclabels[name] = pc

    for s in segments:
        for offset, label in s.relocs.items():
            target = funclabels[label]
            s.buffer[offset] = target & 0xff
            s.buffer[offset + 1] = (target >> 8) & 0xff
        s.write(stdout.buffer)

    start = funclabels[entry]
    stdout.buffer.write(bytes([0x00, (start >> 8) & 0xff, start & 0xff]))
