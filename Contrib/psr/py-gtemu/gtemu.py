"""Python wrapper for the gtemu emulator
"""

import itertools

import asm

import _gtemu

__all__ = ["Emulator", "RAM", "ROM"]

_BLANK_RAM = bytearray([0 for _ in range(1 << 15)])


def _make_state_field_accessor(name):
    """Return a descriptor that accesses the fields of the state"""

    def _getter(self):
        state = self._state
        return getattr(state, name)

    def _setter(self, value):
        state = self._state
        setattr(state, name, value)

    return property(
        _getter,
        _setter,
        doc=f"Get or set the current state of the {name} register",
    )


def _make_zero_page_accessor(
    name, address, *, width=2, byteorder="little", signed=False
):
    """Return a descriptor that accesses zero-page memory"""

    def _getter(self):
        return int.from_bytes(
            RAM[address : address + width], byteorder=byteorder, signed=signed
        )

    def _setter(self, value):
        return value.to_bytes(width, byteorder=byteorder, signed=signed)

    return property(
        _getter, _setter, doc=f"Get or set the current state of the {name} register"
    )


class _Emulator:
    """Provides programatic control over the a Gigatron Emulator"""

    def __init__(self):
        self.reset()

    def load_rom_file(self, path):
        with open(path, "rb") as fp:
            fp.readinto(_gtemu.ffi.buffer(ROM))

    def load_rom_from_asm_module(self):
        """Populates the ROM from the contents of the asm module

        This requires that an assembly script has already been executed.
        """
        def gen_rom_data():
            for opcode, operand in zip(asm._rom0, asm._rom1):
                yield opcode
                yield operand

        rom_data = bytearray(gen_rom_data())
        _gtemu.ffi.buffer(ROM)[0 : len(rom_data)] = rom_data

    def reset(self):
        self._state = _gtemu.ffi.new("CpuState *")[0]
        self._last_pc = None
        self._print = False
        self._vga_x = 0
        self._vga_y = 0
        self._xout = 0
        self.breakpoints = set()
        _gtemu.ffi.buffer(RAM)[:] = _BLANK_RAM
        # Needed for bit shuffling
        _gtemu.ffi.buffer(RAM)[0b1000_0000] = b"\x01"

    PC = _make_state_field_accessor("PC")
    IR = _make_state_field_accessor("IR")
    D = _make_state_field_accessor("D")
    AC = _make_state_field_accessor("AC")
    X = _make_state_field_accessor("X")
    Y = _make_state_field_accessor("Y")
    OUT = _make_state_field_accessor("OUT")

    @property
    def XOUT(self):
        """State of the eXtended OUTput register

        This is set from AC when the hsync signal is low
        """
        return self._xout

    @property
    def hsync(self):
        """Return the state of the HSYNC signal.

        True is high, and False is low, but remember this is an active low signal"""
        return bool(self.OUT & _HSYNC)

    @property
    def vsync(self):
        """Return the state of the VSYNC value.

        True is high, and False is low, but remember this is an active low signal"""
        return bool(self.OUT & _VSYNC)

    @property
    def next_instruction(self):
        """Get the address of the next instruction to execute"""
        return self._last_pc

    @next_instruction.setter
    def next_instruction(self, address):
        """Set program execution to proceed from `address`

        This sets the PC to address + 1, having loaded the instruction at address,
        as if we had just executed address - 1.
        """
        # To start from an address, we need to fill the pipeline with the instruction at address
        # and set PC to address + 1.
        address = asm.symbol(address) or address
        self.PC = address + 1
        self.IR = _gtemu.lib.ROM[address][0]
        self.D = _gtemu.lib.ROM[address][1]
        self._last_pc = address

    @property
    def state(self):
        """Return a string representation of the current state"""
        registers = [
            ("PC", 2),
            ("IR", 1),
            ("D", 1),
            ("AC", 1),
            ("X", 1),
            ("Y", 1),
            ("OUT", 1),
            ("XOUT", 1),
        ]
        heading = " ".join(
            [r.rjust(w * 2 + 1) for r, w in registers] + ["Loaded instruction"]
        )
        separator = " ".join(
            ["-" * (2 * w + 1) for _, w in registers] + ["------------------"]
        )
        values = " ".join(
            [("${:0%dx}" % (w * 2,)).format(getattr(self, r)) for r, w in registers]
            + [asm.disassemble(self.IR, self.D)]
        )
        return "\n".join([heading, separator, values])

    def _step(self):
        """Run a single step of the interpreter"""
        # Store the current PC, so that we can return it as next_instruction
        # This is needed because of the pipeline
        self._last_pc = self.PC
        old_state = self._state
        self._state = new_state = _gtemu.lib.cpuCycle(self._state)
        # Detect changes in the vertical and horizontal sync signals
        hsync_edge = (new_state.OUT & _HSYNC) - (old_state.OUT & _HSYNC)
        vsync_edge = (new_state.OUT & _VSYNC) - (old_state.OUT & _VSYNC)
        # TODO: It might be nice to detect timing errors - but it's tricky because the ROM
        # doesn't go straight into standard timings.
        # We keep these values in order to support run_to_vsync and run_to_hsync
        self._vga_x += 1
        if vsync_edge < 0:
            self._vga_y = -36
        if hsync_edge > 0:
            self._vga_x = 0
            self._vga_y += 1
            self._xout = self.AC

    def step(self):
        self._step()
        if self._print:
            print(self.state)

    def run_for(self, instructions):
        """Run the emulator for a fixed number of cycles

        Will stop at breakpoints if they are hit,
        but always executes at least one cycle

        Returns the number of cycles executed.
        """
        try:
            for i in range(instructions):
                self._step()
                if self._last_pc in self.breakpoints:
                    return i + 1
            return instructions
        finally:
            if self._print:
                print(self.state)

    def run_to(self, address, max_instructions=1000):
        """Run the emulator until it is about to execute the instruction at `address`

        Due to the pipeline, this means that for the previous instruction PC was `address`,
        and therefore we have loaded the instruction.

        Will stop at breakpoints if they are hit,
        but always executes at least one cycle
        """
        address = asm.symbol(address) or address
        iterator = (
            range(max_instructions)
            if max_instructions is not None
            else itertools.count()
        )
        try:
            for i, _ in enumerate(iterator):
                self._step()
                if self._last_pc == address or self._last_pc in self.breakpoints:
                    return i + 1
            raise ValueError(
                "Did not hit address in %d instructions" % (max_instructions,)
            )
        finally:
            if self._print:
                print(self.state)

    def run_to_hblank(self):
        """Run the emulator until we get to the next horizontal blank period

        This is the "front-porch" period which follows the visible pixels
        and preceeds the hsync signal going low

        If we are already in the hblank period, we advance to the following one
        """
        # Counting from the rising edge of HSYNC, there are
        # (48 / 4) back porch pixels
        # + (640 / 4) visible pixels
        # Before we blank
        hblank_start = 48 / 4 + 640 / 4  # = 172
        # Skip through current blank
        while self._vga_x > hblank_start:
            self._step()
        while self._vga_x < hblank_start:
            self._step()
        if self._print:
            print(self.state)

    def run_to_vblank(self):
        """Run the emulator until we get to the next vertical blank period

        This is the "front-porch" period which follows the visible lines
        and preceeds the vsync signal going low

        If we are already in the vblank period, we advance to the following one
        """
        # Our counter starts from -36, following the code in gtemu.c, and so
        # we can just need to wait until we hit line 480

        # If we are currently in a vertical blank interval, advance through it.
        # the caller wants the next one
        while self._vga_y > 480:
            self._step()
        while self._vga_y < 480:
            self._step()
        if self._print:
            print(self.state)

    # vCPU related methods
    vPC = _make_zero_page_accessor("vPC", 0x16)
    vAC = _make_zero_page_accessor("vAC", 0x18)
    vLR = _make_zero_page_accessor("vLR", 0x1A)
    vSP = _make_zero_page_accessor("vSP", 0x1C, width=1)

    @property
    def vcpu_state(self):
        registers = [
            ("vPC", 2),
            ("vAC", 2),
            ("vLR", 2),
            ("vSP", 1),
        ]
        heading = " ".join(
            [r.rjust(w * 2 + 1) for r, w in registers] + ["Next instruction"]
        )
        separator = " ".join(
            ["-" * (2 * w + 1) for _, w in registers] + ["----------------"]
        )
        values = " ".join(
            [("${:0%dx}" % (w * 2,)).format(getattr(self, r)) for r, w in registers]
            + [_disassemble_vcpu_next()]
        )
        return "\n".join([heading, separator, values])

    def _step_vcpu(self):
        breakpoints = {
            0x02FF,
            0x0301,
        }  # ENTER and NEXT - I think we always go through one of these
        new_breakpoints = breakpoints - self.breakpoints
        self.breakpoints |= new_breakpoints
        print_ = self._print
        try:
            self._print = False
            if self.run_for(1000) == 1000:
                if self.next_instruction not in breakpoints:
                    raise ValueError("timeout")
        finally:
            self._print = print_
            self.breakpoints -= new_breakpoints

    def step_vcpu(self):
        self._step_vcpu()
        if self._print:
            print(self.vcpu_state)

    def run_vcpu_to(self, address):
        # Before we run the instruction at 0x200, vPC should be 0x2fe,
        # Because only the low byte is incremented
        target_vpc = address & 0xFF00 | (address - 2) & 0xFF
        while self.vPC != target_vpc:
            self._step_vcpu()
        if self._print:
            print(self.vcpu_state)

    def send_byte(self, value):
        """Send a byte through the input port"""
        # While communication to the Gigatron is serial,
        # it's captured by a shift register, and is presented as a byte
        # so for our purposes we can just write the byte.
        #
        # The shift register is read once per vertical refresh
        # so wait until vsync goes low, write the value and hold it
        # through the rising edge round to the next falling edge
        while self.OUT & _VSYNC:
            self._step()
        _IN[0] = value
        while not (self.OUT & _VSYNC):
            self._step()
        while self.OUT & _VSYNC:
            self._step()
        _IN[0] = 0xFF  # Restore to the initial value

    def read_serial(self, *, bits):
        """Read a single serial value through the output port

        Returns the value read and the number of bits that were read
        """
        # Conversely, from an architectural point of view, the Gigatron has a single 8-bit
        # output port, which is written as a byte. However this drives multiple output devices.
        # Primarily the display, of course, but the /HSYNC signal also clocks the XOUT register
        # (which copies the value of the accumulator) for the LEDS and sound
        # and also the game controller port.
        # The way in which serial output works is actually modulating the duration of the
        # /VSYNC pulse, which can be counted in terms of /HSYNC pulses.
        # 8 is the default length and so can be interpretted as no data transmission,
        # but it is shortened to 7 for a zero bit, and stretched to 9 for a 1.
        # This code is kinda complicated
        def generate_outputs():
            """Step the emulator yielding OUT"""
            while True:
                yield self.OUT
                self._step()

        # create two generators yielding just the vsync and hsync bits.
        # The place value doesn't matter because we're going to edge-detect
        # and only look at sign.
        _outs1, _outs2 = itertools.tee(generate_outputs())
        vsyncs, hsyncs = (o & _VSYNC for o in _outs1), (o & _HSYNC for o in _outs2)

        def detect_edges(gen):
            """Yield positive numbers for rising edges, and negative for falling"""
            # Pairwise iteration
            a, b = itertools.tee(gen)
            next(b, None)
            for prev, curr in zip(a, b):
                yield curr - prev

        def debug_detect_edges_short(edges):
            """Useful routine to use when debugging the code below"""
            for value in edges:
                if value < 0:
                    print("v", end="")
                elif value > 0:
                    print("^", end="")
                yield value

        def debug_detect_edges(edges):
            """Useful routine to use when debugging the code below"""
            state = "X"
            for i, value in enumerate(edges):
                if i % 6400 == 0:
                    print("\n", self.state)
                    print("\n", self.vcpu_state)
                elif i % 80 == 0:
                    print()
                if value == 0:
                    print(state, end="")
                elif value < 0:
                    state = "_"
                    print("\\", end="")
                else:
                    state = "‾"
                    print("/", end="")
                yield value

        # vsync_edges, hsync_edges = (
        #     debug_detect_edges_short(detect_edges(vsyncs)),
        #     debug_detect_edges(detect_edges(hsyncs)),
        # )
        vsync_edges, hsync_edges = detect_edges(vsyncs), detect_edges(hsyncs)

        def do_line_counting(vsync_edges, hsync_edges):
            """Yield count of lines in each negative vsync pulse"""
            pairs = zip(vsync_edges, hsync_edges)
            while True:
                # Advance to falling edge
                i = itertools.dropwhile(lambda pair: pair[0] >= 0, pairs)
                # .. and continue until we see a rising edge
                i = itertools.takewhile(lambda pair: pair[0] <= 0, i)
                # count falling HSYNC edges within this window
                yield sum(1 for _, h_edge in i if h_edge < 0)

        lines_per_vsync = do_line_counting(vsync_edges, hsync_edges)

        def debug_counting(lines_per_vsync):
            """useful routine when debugging the code below"""
            for frame, hsync_edges in enumerate(lines_per_vsync):
                print(f"Frame {frame} has {hsync_edges} line vsync pulse")
                yield hsync_edges

        # lines_per_vsync = debug_counting(lines_per_vsync)
        # Skip frames with 8 line VSYNC signals
        lines_per_vsync = itertools.dropwhile(lambda count: count == 8, lines_per_vsync)
        # and continue until we return to 8
        lines_per_vsync = itertools.takewhile(lambda count: count != 8, lines_per_vsync)
        # Data is sent in little-endian order.
        # So a 1 in the first frame represents 1
        # and a 1 in the second frame represents 2
        result = 0
        for bit_number, lines in enumerate(lines_per_vsync):
            assert lines in (7, 9), f"Unexpected VSYNC length {lines}"
            if lines == 7:
                value = 0
            if lines == 9:
                value = 1 << bit_number
            result |= value
            if bit_number == bits - 1:
                break
        else:
            # We did not see the expected number of bits in the transmission
            raise AssertionError(f"Only saw {bit_number} bits in the transmission")
        return result


_VSYNC = 0b1000_0000
_HSYNC = 0b0100_0000


# This is effectively a pointer to the IN global variable, which can be
# Updated through list assignment.
_IN = _gtemu.ffi.addressof(_gtemu.lib, "IN")
RAM = _gtemu.lib.RAM
ROM = _gtemu.lib.ROM
Emulator = _Emulator()


# Little vCPU disassembler copied from gt1dump.py
def _disassemble_vcpu_next():
    """Return the string disassembly of the next instruction"""
    # The next vPC is not Emulator.vPC + 2 - it doesn't carry into the high byte
    next_vpc = Emulator.vPC & 0xFF00 | ((Emulator.vPC + 2 & 0xFF))
    opcode = RAM[next_vpc]
    # If we don't know a mnemonic, just use hex - no operands
    mnemonic, number_of_operands = _OPCODES.get(opcode, (f"${opcode:02x}", 0))
    asm_parts = [mnemonic]
    operands = bytearray(RAM[next_vpc + 1 : next_vpc + 1 + number_of_operands])
    while operands:
        # Poor man's switch statement
        if mnemonic in _ZP_MODE_OPCODES:
            operand = operands.pop(0)
            decoding = _ZERO_PAGE_SYMBOLS.get(operand, f"${operand:02x}")
            asm_parts.append(decoding)
            continue
        if mnemonic == "Bcc":
            operand = operands.pop(0)
            if operand in _BCC_CODES:
                asm_parts = [f"B{_BCC_CODES[operand]}"]
            # Fall through
        if mnemonic in {"Bcc", "BRA", "DEF"}:
            # Next operand is a target within a page
            operand = operands.pop(0)
            target = next_vpc & 0xFF00 | (operand + 2) & 0xFF
            asm_parts.append(f"${target:04x}")
            continue
        if mnemonic == "SYS":
            operand = operands.pop(0)
            if operand != 128:
                max_cycles = 28 - 2 * ((operand ^ 128) - 128)
                asm_parts.append(str(max_cycles))
            else:
                asm_parts = ["HALT"]
            continue
        # Else...
        # Treat any remaining bytes as a single operand
        operand = int.from_bytes(operands, "little", signed=False)
        asm_parts.append(f"${operand:02x}" if len(operands) == 1 else f"${operand:04x}")
        break
    return " ".join(asm_parts)


_OPCODES = {
    0x11: ("LDWI", 2),
    0x1A: ("LD", 1),
    0x1F: ("CMPHS", 1),
    0x21: ("LDW", 1),
    0x2B: ("STW", 1),
    0x35: ("Bcc", 2),
    0x59: ("LDI", 1),
    0x5E: ("ST", 1),
    0x63: ("POP", 0),
    0x75: ("PUSH", 0),
    0x7F: ("LUP", 1),
    0x82: ("ANDI", 1),
    0x85: ("CALLI", 2),
    0x88: ("ORI", 1),
    0x8C: ("XORI", 1),
    0x90: ("BRA", 1),
    0x93: ("INC", 1),
    0x97: ("CMPHU", 1),
    0x99: ("ADDW", 1),
    0xAD: ("PEEK", 0),
    0xB4: ("SYS", 1),
    0xB8: ("SUBW", 1),
    0xCD: ("DEF", 1),
    0xCF: ("CALL", 1),
    0xDF: ("ALLOC", 1),
    0xE3: ("ADDI", 1),
    0xE6: ("SUBI", 1),
    0xE9: ("LSLW", 0),
    0xEC: ("STLW", 1),
    0xEE: ("LDLW", 1),
    0xF0: ("POKE", 1),
    0xF3: ("DOKE", 1),
    0xF6: ("DEEK", 0),
    0xF8: ("ANDW", 1),
    0xFA: ("ORW", 1),
    0xFC: ("XORW", 1),
    0xFF: ("RET", 0),
}

_BCC_CODES = {
    0x3F: "EQ",
    0x4D: "GT",
    0x50: "LT",
    0x53: "GE",
    0x56: "LE",
    0x72: "NE",
}

_ZERO_PAGE_SYMBOLS = {
    0x00: "zeroConst",
    0x01: "memSize",
    0x06: "entropy",
    0x09: "videoY",
    0x0E: "frameCount",
    0x0F: "serialRaw",
    0x11: "buttonState",
    0x14: "xoutMask",
    0x16: "vPC",
    0x17: "vPC+1",
    0x18: "vAC",
    0x19: "vAC+1",
    0x1A: "vLR",
    0x1B: "vLR+1",
    0x1C: "vSP",
    0x21: "romType",
    0x22: "sysFn",
    0x23: "sysFn+1",
    0x24: "sysArgs+0",
    0x25: "sysArgs+1",
    0x26: "sysArgs+2",
    0x27: "sysArgs+3",
    0x28: "sysArgs+4",
    0x29: "sysArgs+5",
    0x2A: "sysArgs+6",
    0x2B: "sysArgs+7",
    0x2C: "soundTimer",
    0x2E: "ledState_v2",
    0x2F: "ledTempo",
    0x80: "oneConst",
}

# Opcodes that address page zero
_ZP_MODE_OPCODES = {
    "LD",
    "LDW",
    "STW",
    "ST",
    "INC",
    "ADDW",
    "SUBW",
    "CALL",
    "POKE",
    "DOKE",
    "ANDW",
    "ORW",
    "XORW",
    "CMPHS",
    "CMPHU",
}
