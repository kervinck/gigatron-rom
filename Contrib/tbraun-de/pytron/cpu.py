from random import randint
import sys


class CpuState:
    def __init__(self):
        self.pc = randint(0, 255)
        self.ir = randint(0, 255)
        self.d = randint(0, 255)
        self.ac = randint(0, 255)
        self.x = randint(0, 255)
        self.y = randint(0, 255)
        self.out = randint(0, 255)
        self.undef = randint(0, 255)

    def copyTo(self, destination):
        destination.pc = self.pc
        destination.ir = self.ir
        destination.d = self.d
        destination.ac = self.ac
        destination.x = self.x
        destination.y = self.y
        destination.out = self.out
        destination.undef = self.undef


class Cpu():
    def __init__(self):
        self.rom = [[randint(0, 255) for x in range(2)] for y in range(1 << 16)]
        self.ram = [randint(0, 255) for x in range(1 << 15)]
        self.IN = 0xff
        self.tempState = CpuState()

    def cpuCycle(self, state: CpuState):
        state.copyTo(self.tempState)

        self.tempState.ir = self.rom[state.pc][0]
        self.tempState.d = self.rom[state.pc][1]

        instruction = state.ir >> 5
        mod = (state.ir >> 2) & 7
        bus = state.ir & 3
        w = (instruction == 6)
        j = (instruction == 7)
        lo = state.d
        hi = 0
        to = None

        inc_x = False

        if not j:
            if mod == 0:
                to = "AC" if w is False else None
            elif mod == 1:
                to = "AC" if w is False else None
                lo = state.x
            elif mod == 2:
                to = "AC" if w is False else None
                hi = state.y
            elif mod == 3:
                to = "AC" if w is False else None
                lo = state.x
                hi = state.y
            elif mod == 4:
                to = "X"
            elif mod == 5:
                to = "Y"
            elif mod == 6:
                to = "OUT" if w is False else None
            elif mod == 7:
                to = "OUT" if w is False else None
                lo = state.x
                hi = state.y
                inc_x = True
        addr = (hi << 8) | lo

        b = state.undef
        if bus == 0:
            b = state.d
        elif bus == 1:
            if not w:
                b = self.ram[addr & 0x7fff]
        elif bus == 2:
            b = state.ac
        elif bus == 3:
            b = self.IN

        if w:
            self.ram[addr & 0x7fff] = b

        if instruction == 0:    # LD
            ALU = b
        elif instruction == 1:  # ANDA
            ALU = state.ac & b
        elif instruction == 2:  # ORA
            ALU = state.ac | b
        elif instruction == 3:  # XORA
            ALU = state.ac ^ b
        elif instruction == 4:  # ADDA
            ALU = (state.ac + b) & 0xFF
        elif instruction == 5:  # SUBA
            ALU = (state.ac - b) & 0xFF
        elif instruction == 6:  # ST
            ALU = state.ac
        elif instruction == 7:  # Bcc/JMP
            ALU = (-state.ac) & 0xFF

        # Load value into register
        if to == "AC":
            self.tempState.ac = ALU
        elif to == "X":
            self.tempState.x = ALU
        elif to == "Y":
            self.tempState.y = ALU
        elif to == "OUT":
            self.tempState.out = ALU

        if inc_x:
            self.tempState.x = (state.x + 1) & 0xFF

        self.tempState.pc = (state.pc + 1) & 0xFFFF
        if j:
            if mod != 0:
                cond = (state.ac >> 7) + 2 * (state.ac == 0)
                if (mod & (1 << cond)) != 0:    # 74153
                    self.tempState.pc = (state.pc & 0xff00) | b
            else:
                self.tempState.pc = (state.y << 8) | b   # Unconditional far jump
        return self.tempState


state = CpuState()
new_state = CpuState()
cpu = Cpu()
with open("../../../ROMv3.rom", "rb") as f:
    byte = f.read(1)
    rom_address = 0
    while byte != b"":
        cpu.rom[rom_address][0] = ord(byte)
        byte = f.read(1)
        cpu.rom[rom_address][1] = ord(byte)
        byte = f.read(1)
        rom_address = rom_address + 1

vgaX = 0
vgaY = 0

t = -2
while True:
    if t < 0:
        state.pc = 0  # MCP100 Power-On Reset

    #sys.stdout.write("PC %04x: %02x%02x IR:%02x D:%02x AC:%02x X:%02x Y:%02x OUT:%02x UNDEF:%02x\n" %
    #                 (state.pc, cpu.rom[state.pc][0], cpu.rom[state.pc][1], state.ir, state.d, state.ac, state.x,
    #                  state.y, state.out, state.undef))

    cpu.cpuCycle(state).copyTo(new_state)  # Update CPU

    hSync = (new_state.out & 0x40) - (state.out & 0x40)  # "VGA monitor" (use simple stdout)
    vSync = (new_state.out & 0x80) - (state.out & 0x80)
    if vSync < 0:
        vgaY = -36
    vgaX = vgaX + 1
    if vgaX <= 200:
        if hSync != 0:
            sys.stdout.write('|')  # Visual indicator of hSync
        elif vgaX == 200:
            sys.stdout.write('>')  # too many pixels
        elif (~state.out) & 0x80 > 0:
            sys.stdout.write('^')  # visualize vBlank pulse
        else:
            sys.stdout.write(chr(32 + (state.out & 63)))  # plot pixel
    if hSync > 0:
        if vgaX != 200:
            sys.stdout.write('~')  # mark horizontal cycle errors
        sys.stdout.write("line %-3d xout %02x t %0.3f\n" % (vgaY, new_state.ac, t / 6250000))
        vgaX = 0
        vgaY = vgaY + 1
        new_state.undef = randint(0, 255)

    new_state.copyTo(state)

    t = t + 1

