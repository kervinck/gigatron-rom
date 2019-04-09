from copy import copy
from sys import stdout, stderr

global_labels = {
    'vAC': 0x0018,
    'vACH': 0x0019,
    'r1': 0x0030,
    'r2': 0x0032,
    'r3': 0x0034,
    'r4': 0x0035,
    'r5': 0x0036,
    'r6': 0x0038,
    'r7': 0x003a,
    'r8': 0x003c,
    'r9': 0x003e,
    'r10': 0x0040,
    'r11': 0x0042,
    'r12': 0x0044,
    'r13': 0x0046,
    'r14': 0x0048,
    'r15': 0x004a,
    'ha': 0x004c,
    'pvpc': 0x004e,
    'ldloc': 0x0050,
    'stloc': 0x0052,
    'thunk': 0x0054,
    'lsh': 0x0056,
    'rsh': 0x0058,
    'mul': 0x005a,
    'mod': 0x005c,
    'div': 0x005e,
    'sp': 0x0060,
}

class Inst:
    def __init__(self, opcode, operand, size, branch, emit):
        self.addr = None
        self.opcode = opcode
        self.operand = operand
        self.size = size
        self.branch = branch
        self._emit = emit

    def emit(self):
        self._emit(self)

class Segment:
    def __init__(self, address):
        assert(address & 0xff == 0)
        self.address = address
        self.buffer = bytearray()

    def emit(self, data):
        assert(len(self.buffer) + len(data) <= 256)
        self.buffer += data

    def write(self, stream):
        stream.write(bytes([self.address >> 8 & 0xff, self.address & 0xff, len(self.buffer)]))
        stream.write(self.buffer)

func = None
segment = None

def defun(name):
    global func
    func = []

def emit(data):
    segment.emit(data)

def emitb(opcode, operand):
    assert(operand >= -128 and operand < 256)
    emit(bytes([opcode, operand]))

def emitw(opcode, operand):
    assert(operand >= -32768 and operand < 65536)
    emit(bytes([opcode, operand & 0xff, (operand >> 8) & 0xff]))

def displacement(operand):
    operand = operand & 0xff
    return operand - 2 if operand >= 2 else 254 + operand

def emitjcc(inst, near, far):
    if inst.operand & 0xff00 != inst.addr & 0xff00:
        # far jump
        assert(inst.size == 8)
        print(f'emitting far branch from {inst.addr:x} to {inst.operand:x}', file=stderr)
        skip = inst.addr + 8
        emit(bytes([0x35, far, displacement(skip)])) # bcc <far> <skip>
        emitw(0x11, inst.operand)             # ldwi <target>
        emitb(0xf3, global_labels['pvpc'])    # doke vpc
    else:
        # near jump
        assert(inst.size == 3)
        print(f'emitting near branch from {inst.addr:x} to {inst.operand:x}', file=stderr)
        emit(bytes([0x35, near, displacement(inst.operand)]))

def label(name):
    func.append(Inst('label', name, 0, False, lambda _: None))

def ldwi(con):
    func.append(Inst('ldwi', con, 3, False, lambda inst: emitw(0x11, inst.operand)))

def ld(d):
    func.append(Inst('ld', d, 2, False, lambda inst: emitb(0x1a, inst.operand)))

def ldw(d):
    func.append(Inst('ldw', d, 2, False, lambda inst: emitb(0x21, inst.operand)))

def stw(d):
    func.append(Inst('stw', d, 2, False, lambda inst: emitb(0x2b, inst.operand)))

def jeq(l):
    func.append(Inst('jeq', l, 8, True, lambda inst: emitjcc(inst, 0x3f, 0x72)))

def jne(l):
    func.append(Inst('jne', l, 8, True, lambda inst: emitjcc(inst, 0x72, 0x3f)))

def jge(l):
    func.append(Inst('jge', l, 8, True, lambda inst: emitjcc(inst, 0x53, 0x50)))

def jgt(l):
    func.append(Inst('jgt', l, 8, True, lambda inst: emitjcc(inst, 0x4d, 0x56)))

def jle(l):
    func.append(Inst('jle', l, 8, True, lambda inst: emitjcc(inst, 0x56, 0x4d)))

def jlt(l):
    func.append(Inst('jlt', l, 8, True, lambda inst: emitjcc(inst, 0x50, 0x53)))

def ldi(con):
    func.append(Inst('ldi', con, 2, False, lambda inst: emitb(0x59, inst.operand)))

def st(d):
    func.append(Inst('st', d, 2, False, lambda inst: emitb(0x5e, inst.operand)))

def pop():
    func.append(Inst('pop', None, 1, False, lambda _: emit(bytes([0x63]))))

def push():
    func.append(Inst('push', None, 1, False, lambda _: emit(bytes([0x75]))))

def lup(d):
    func.append(Inst('lup', d, 2, False, lambda inst: emitb(0x7f, inst.operand)))

def andi(con):
    func.append(Inst('andi', con, 2, False, lambda inst: emitb(0x82, inst.operand)))

def ori(con):
    func.append(Inst('ori', con, 2, False, lambda inst: emitb(0x88, inst.operand)))

def xori(con):
    func.append(Inst('xori', con, 2, False, lambda inst: emitb(0x8c, inst.operand)))

def jr():
    # Check for a preceding ldwi. If one exists, snip it out and create a 'j' instead of a 'jr'.
    if len(func) > 0 and func[len(func)-1].opcode == 'ldwi':
        def e(inst):
            if inst.operand & 0xff00 == inst.addr & 0xff00:
                print(f'emitting near jump from {inst.addr:x} to {inst.operand:x}', file=stderr)
                emitb(0x90, displacement(inst.operand))
            else:
                # TODO: does the operand need to be adjusted? probably.
                print(f'emitting far jump from {inst.addr:x} to {inst.operand:x}', file=stderr)
                ldwi(inst.operand).emit()
                emitb(0xf3, global_labels['pvpc'])
        func[len(func)-1] = Inst('j', func[len(func)-1].operand, 5, False, e)
    else:
        func.append(Inst('jr', None, 2, False, lambda _: emitb(0xf3, global_labels['pvpc'])))

def inc(d):
    func.append(Inst('inc', d, 2, False, lambda inst: emitb(0x93, inst.operand)))

def addw(d):
    func.append(Inst('addw', d, 2, False, lambda inst: emitb(0x99, inst.operand)))

def peek():
    func.append(Inst('peek', None, 1, False, lambda _: emit(bytes([0xad]))))

def sys(con):
    func.append(Inst('sys', con, 2, False, lambda inst: emitb(0xb4, inst.operand)))

def subw(d):
    func.append(Inst('subw', d, 2, False, lambda inst: emitb(0xb8, inst.operand)))

def call(d):
    func.append(Inst('call', d, 2, False, lambda inst: emitb(0xcf, inst.operand)))

def addi(con):
    func.append(Inst('addi', con, 2, False, lambda inst: emitb(0xe3, inst.operand)))

def subi(con):
    func.append(Inst('subi', con, 2, False, lambda inst: emitb(0xe6, inst.operand)))

def lslw():
    func.append(Inst('lslw', None, 1, False, lambda inst: emit(bytes([0xe9]))))

def poke(d):
    func.append(Inst('poke', d, 2, False, lambda inst: emitb(0xf0, inst.operand)))

def doke(d):
    func.append(Inst('doke', d, 2, False, lambda inst: emitb(0xf3, inst.operand)))

def deek():
    func.append(Inst('deek', None, 1, False, lambda inst: emit(bytes([0xf6]))))

def andw(d):
    func.append(Inst('andw', d, 2, False, lambda inst: emitb(0xf8, inst.operand)))

def orw(d):
    func.append(Inst('orw', d, 2, False, lambda inst: emitb(0xfa, inst.operand)))

def xorw(d):
    func.append(Inst('xorw', d, 2, False, lambda inst: emnitb(0xfc, inst.operand)))

def ret():
    func.append(Inst('ret', None, 1, False, lambda _: emit(bytes([0xff]))))

def link():
    global segment, vac

    # TODO: take memory map into account
    #
    # lay out the current functionA
    # first set its offset
    # then do its initial layout:
    # - assume all jumps are far unless they are provably close
    #     - basically, all forward jumps must be far jumps if the function may cross a page boundary
    # - place page fallthough code as necessary
    # then attempt to shorten jumps and iterate until reaching a fixed point

    base = 0x500
    segments = []
    labels = copy(global_labels)

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

    def layout(emitting):
        pc, remaining = base, 0x100
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
                    pc = pc + remaining
                    remaining = 0x100

                    if emitting:
                        call(thunk).emit()
                        segment = Segment(pc)
                        segments.append(segment)

                    shorten(inst, pc)

            if inst.addr != pc:
                changed = True
            inst.addr = pc

            if emitting:
                if type(inst.operand) is str:
                    inst.operand = labels[inst.operand]
                if inst.opcode == 'ldwi':
                    vac = inst.operand
                else:
                    vac = None
                inst.emit()

            pc += inst.size
            remaining -= inst.size

        return (pc, changed)

    while True:
        pc, changed = layout(False)
        if not changed:
            break

    segment = Segment(base)
    segments.append(segment)
    layout(True)

    for s in segments:
        s.write(stdout.buffer)
    stdout.buffer.write(bytes([0x00, 0x05, 0x00]))
