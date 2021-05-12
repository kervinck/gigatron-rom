# SYNOPSIS: from vcpu import *
#
# This is the a small vCPU emulator in Python.
# It can be very useful for debugging and testing GT1 programs
# in Python, specially the ones written using vasm.
#

# VirtualCpu to emulate the vCPU inside the Gigatron ROM.

class VirtualCpu(object):
    vPC_ADDR = 0x16
    vAC_ADDR = 0x18
    vLR_ADDR = 0x1A
    vSP_ADDR = 0x1C

    def __init__(self):
        """Constructor of the VirtualCpu."""
        self._memory = None
        self._mem_size = None
        self._read_watchpoints = None
        self._write_watchpoints = None
        self._lup_callback = None
        self._sys_callback = None
        self._enable_experimental = None
        self._halt = None

        # Instruction table
        insn_table = [
            ( 0x5E, "ST",    2, self.execute_st    ),
            ( 0x2B, "STW",   2, self.execute_stw   ),
            ( 0xEC, "STLW",  2, self.execute_stlw  ),
            ( 0x1A, "LD",    2, self.execute_ld    ),
            ( 0x59, "LDI",   2, self.execute_ldi   ),
            ( 0x11, "LDWI",  3, self.execute_ldwi  ),
            ( 0x21, "LDW",   2, self.execute_ldw   ),
            ( 0xEE, "LDLW",  2, self.execute_ldlw  ),
            ( 0x99, "ADDW",  2, self.execute_addw  ),
            ( 0xB8, "SUBW",  2, self.execute_subw  ),
            ( 0xE3, "ADDI",  2, self.execute_addi  ),
            ( 0xE6, "SUBI",  2, self.execute_subi  ),
            ( 0xE9, "LSLW",  1, self.execute_lslw  ),
            ( 0x93, "INC",   2, self.execute_inc   ),
            ( 0x82, "ANDI",  2, self.execute_andi  ),
            ( 0xF8, "ANDW",  2, self.execute_andw  ),
            ( 0x88, "ORI",   2, self.execute_ori   ),
            ( 0xFA, "ORW",   2, self.execute_orw   ),
            ( 0x8C, "XORI",  2, self.execute_xori  ),
            ( 0xFC, "XORW",  2, self.execute_xorw  ),
            ( 0xAD, "PEEK",  1, self.execute_peek  ),
            ( 0xF6, "DEEK",  1, self.execute_deek  ),
            ( 0xF0, "POKE",  2, self.execute_poke  ),
            ( 0xF3, "DOKE",  2, self.execute_doke  ),
            ( 0x7F, "LUP",   2, self.execute_lup   ),
            ( 0x90, "BRA",   2, self.execute_bra   ),
            ( 0x35, "BCC",   3, self.execute_bcc   ),
            ( 0xCF, "CALL",  2, self.execute_call  ),
            ( 0xFF, "RET",   1, self.execute_ret   ),
            ( 0x75, "PUSH",  1, self.execute_push  ),
            ( 0x63, "POP",   1, self.execute_pop   ),
            ( 0xDF, "ALLOC", 2, self.execute_alloc ),
            ( 0xB4, "SYS",   2, self.execute_sys   ),
            ( 0xCD, "DEF",   2, self.execute_def   ),

            # Experimental instructions
            ( 0x85, "CALLI", 3, self.execute_calli ),
            ( 0x1F, "CMPHS", 2, self.execute_cmphs ),
            ( 0x97, "CMPHU", 2, self.execute_cmphu ),
        ]

        # Construct a dispatch table for the instructions
        self._insn_table = {
            entry[0] : entry for entry in insn_table
        }

        # Branch table
        branch_table = [
            ( 0x3F, "BEQ" ),
            ( 0x72, "BNE" ),
            ( 0x50, "BLT" ),
            ( 0x4D, "BGT" ),
            ( 0x56, "BLE" ),
            ( 0x53, "BGE" ),
        ]
        self._branch_table = {
            entry[0] : entry for entry in branch_table
        }

    def read_byte(self, addr):
        """Reads one byte on a given address.

        Parameters
        ----------
        addr: int
            The address to read from.

        Returns
        -------
            The byte at the given address.
        """
        cb = self._read_watchpoints.get(addr, None)
        if cb is not None:
            cb(self, addr)

        addr = addr % self._mem_size
        return self._memory[addr]

    def write_byte(self, addr, b):
        """Writes one byte to a given address.

        Parameters
        ----------
        addr: int
            The address to write to.
        b: int
            The byte to write.
        """
        cb = self._write_watchpoints.get(addr, None)
        if cb is not None:
            cb(self, addr)

        addr = addr % self._mem_size
        self._memory[addr] = (b & 0xFF)

    def read_word(self, addr):
        """Reads one word (2 bytes) on a given address.

        Parameters
        ----------
        addr: int
            The address to read from.

        Returns
        -------
            The word at the given address.
        """
        lo = self.read_byte(addr)
        hi = self.read_byte(self.relative(addr, 1))
        return (hi << 8) | lo

    def write_word(self, addr, w):
        """Writes one word to a given address.

        Parameters
        ----------
        addr: int
            The address to write to.
        w: int
            The word to write.
        """
        self.write_byte(addr, w & 0xFF)
        self.write_byte(self.relative(addr, 1), (w >> 8))

    def absolute(self, base, page_offset):
        """Computes the address at same page as in the base address.

        Parameters
        ----------
        base: int
            The base address.
        page_offset: int
            The offset relative to the page of the base address.

        Returns
        -------
            The address.
        """
        return (base & ~0xFF) | (page_offset & 0xFF)

    def relative(self, base, offset):
        """Computes the address at given offset of a base address.

        Because addresses stay on the same page (they wrap around, and
        when adding, the carry to the high byte is not included), this
        function is necessary.

        Parameters
        ----------
        base: int
            The base address.
        offset: int
            The offset relative to the base address.

        Returns
        -------
            The address.
        """
        return self.absolute(base, base + offset)

    def get_vPC(self):
        """Gets the value of the vPC register.

        Returns
        -------
            The vPC register.
        """
        return self.read_word(VirtualCpu.vPC_ADDR)

    def set_vPC(self, w):
        """Sets the value of the vPC register.

        Parameters
        ----------
        w: int
            The new value of the vPC register.
        """
        self.write_word(VirtualCpu.vPC_ADDR, w)

    def get_vAC(self):
        """Gets the value of the vAC register.

        Returns
        -------
            The vAC register.
        """
        return self.read_word(VirtualCpu.vAC_ADDR)

    def set_vAC(self, w):
        """Sets the value of the vAC register.

        Parameters
        ----------
        w: int
            The new value of the vAC register.
        """
        self.write_word(VirtualCpu.vAC_ADDR, w)

    def get_vLR(self):
        """Gets the value of the vLR register.

        Returns
        -------
            The vLR register.
        """
        return self.read_word(VirtualCpu.vLR_ADDR)

    def set_vLR(self, w):
        """Sets the value of the vLR register.

        Parameters
        ----------
        w: int
            The new value of the vLR register.
        """
        self.write_word(VirtualCpu.vLR_ADDR, w)

    def get_vSP(self):
        """Gets the value of the vSP register.

        Returns
        -------
            The SP register.
        """
        return self.read_byte(VirtualCpu.vSP_ADDR)

    def set_vSP(self, b):
        """Sets the value of the vSP register.

        Parameters
        ----------
        b: int
            The new value of the vSP register.
        """
        self.write_byte(VirtualCpu.vSP_ADDR, b)

    def halt(self, v=True):
        """Halts the emulation of the vCPU.

        Parameters
        ----------
        v: bool
            Set to True to halt.
        """
        self._halt = v

    def load_gt1(self, gt1_bytes, mem_size=32768,
                 lup_callback=None, sys_callback=None,
                 enable_experimental=False):
        """Loads the GT1 file to memory and resets the vCPU.

        Parameters
        ----------
        gt1_bytes: bytes
            A byte array / buffer that holds the contents of the GT1
            file to load.
        mem_size: int
            The memory size of this VirtualCpu instance.
        lup_callback: callable
            The callback used to implement LUP instructions.
        sys_callback: callable
            The callback used to implement SYS instructions.
        enable_experimental: bool
            To enable experimental instructions (such as CALLI).
        """
        self._mem_size = mem_size
        self._memory = bytearray(mem_size)
        self._read_watchpoints = dict()
        self._write_watchpoints = dict()
        self._lup_callback = lup_callback
        self._sys_callback = sys_callback
        self._enable_experimental = enable_experimental

        idx = 0

        hi_addr = gt1_bytes[idx]
        idx += 1

        while hi_addr != 0:
            lo_addr = gt1_bytes[idx]
            idx += 1

            num_bytes = gt1_bytes[idx]
            idx += 1

            addr = (hi_addr << 8) | lo_addr
            for j in range(num_bytes):
                self.write_byte(self.relative(addr, j),
                                gt1_bytes[idx + j])
            idx += num_bytes

            hi_addr = gt1_bytes[idx]
            idx += 1

        hi_addr = gt1_bytes[idx]
        idx += 1

        lo_addr = gt1_bytes[idx]
        idx += 1

        vPC = (hi_addr << 8) | lo_addr
        self.set_vPC(vPC)
        self.set_vSP(0xFE)

    def disassemble(self):
        vPC = self.get_vPC()
        insn = self.read_byte(vPC)

        if insn in self._insn_table:
            name, nb = self._insn_table[insn][1:3]
            t = "%-5s" % name
            if name == "BCC":
                b1 = self.read_byte(self.relative(vPC, 1))
                b2 = self.read_byte(self.relative(vPC, 2))
                if b1 in self._branch_table:
                    t = self._branch_table[b1][-1]
                    t = "%-5s $%02X" % (t, b2)
                else:
                    t = "<err>"
            else:
                if nb == 2:
                    b = self.read_byte(self.relative(vPC, 1))
                    t = "%s $%02X" % (t, b)
                elif nb == 3:
                    w = self.read_word(self.relative(vPC, 1))
                    t = "%s $%04X" % (t, w)
        else:
            t = "<err>"

        t = "%-15s" % t
        vAC = self.get_vAC()
        vLR = self.get_vLR()
        vSP = self.get_vSP()
        print("PC: $%04X, AC: $%04X, LR: $%04X, SP: $%02X  %s" %\
              (vPC, vAC, vLR, vSP, t))

    def run(self,
            max_instructions=None,
            breakpoints=None,
            read_watchpoints=None,
            write_watchpoints=None):
        """Runs the vCPU for a specified amount.

        Parameters
        ----------
        max_instructions: int
            The maximum number of instructions to run.
            If not specified, it runs indefinitely.
        breakpoints: dict
            The dictionary of breakpoints. The keys are the addresses
            of the breakpoints and the associated values are callbacks
            to be called when the vCPU hits the corresponding breakpoint.
            The callbacks should accept two parameters: a reference to
            `VirtualCpu` itself, and the address of the breakpoint.
        read_watchpoints: dict
            The dictionary of memory addresses to watch for reads.
            It works similarly as the `breakpoints` parameter.
        write_watchpoints: dict
            The dictionary of memory addresses to watch for writes.
            It works similarly as the `breakpoints` parameter.
        """
        if breakpoints is None:
            breakpoints = dict()
        else:
            breakpoints = dict(breakpoints)

        if read_watchpoints is None:
            read_watchpoints = dict()
        else:
            read_watchpoints = dict(read_watchpoints)

        if write_watchpoints is None:
            write_watchpoints = dict()
        else:
            write_watchpoints = ditc(write_watchpoints)

        self._read_watchpoints = read_watchpoints
        self._write_watchpoints = write_watchpoints
        self._halt = False

        num_instructions = 0
        while not self._halt:
            vPC = self.get_vPC()

            cb = breakpoints.get(vPC, None)
            if cb is not None:
                cb(self, vPC)

            if self._halt:
                break

            self.execute_instruction()
            num_instructions += 1

            if max_instructions is not None and\
               num_instructions >= max_instructions:
                break

    def execute_instruction(self):
        """Executes one vCPU instruction."""
        vPC = self.get_vPC()
        insn = self.read_byte(vPC)
        if insn in self._insn_table:
            self._insn_table[insn][-1]()
        else:
            raise RuntimeError("invalid instruction 0x%02X" % insn)

    def execute_st(self):
        """Executes the ST instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vAC = self.get_vAC()
        self.write_byte(dd, vAC & 0xFF)
        self.set_vPC(self.relative(vPC, 2))

    def execute_stw(self):
        """Executes the STW instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vAC = self.get_vAC()
        self.write_word(dd, vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_stlw(self):
        """Executes the STLW instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        dd += self.get_vSP()
        vAC = self.get_vAC()
        self.write_word(dd & 0xFF, vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_ld(self):
        """Executes the LD instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vAC = self.read_byte(dd)
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_ldi(self):
        """Executes the LDI instruction."""
        vPC = self.get_vPC()
        vAC = self.read_byte(self.relative(vPC, 1))
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_ldwi(self):
        """Executes the LDWI instruction."""
        vPC = self.get_vPC()
        vAC = self.read_word(self.relative(vPC, 1))
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 3))

    def execute_ldw(self):
        """Executes the LDW instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vAC = self.read_word(dd)
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_ldlw(self):
        """Executes the LDLW instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        dd += self.get_vSP()
        vAC = self.read_word(dd & 0xFF)
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_addw(self):
        """Executes the ADDW instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vAC = self.get_vAC()
        vAC += self.read_word(dd)
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_subw(self):
        """Executes the SUBW instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vAC = self.get_vAC()
        vAC -= self.read_word(dd)
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_addi(self):
        """Executes the ADDI instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vAC = self.get_vAC()
        vAC += dd
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_subi(self):
        """Executes the SUBI instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vAC = self.get_vAC()
        vAC -= dd
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_lslw(self):
        """Executes the LSLW instruction."""
        vPC = self.get_vPC()
        vAC = self.get_vAC()
        vAC = vAC << 1
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 1))

    def execute_inc(self):
        """Executes the INC instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        v = self.read_byte(dd)
        self.write_byte(dd, v + 1)
        self.set_vPC(self.relative(vPC, 2))

    def execute_andi(self):
        """Executes the ANDI instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vAC = self.get_vAC()
        vAC = vAC & dd
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_andw(self):
        """Executes the ANDW instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vAC = self.get_vAC()
        vAC = vAC & self.read_word(dd)
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_ori(self):
        """Executes the ORI instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vAC = self.get_vAC()
        vAC = vAC | dd
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_orw(self):
        """Executes the ORW instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vAC = self.get_vAC()
        vAC = vAC | self.read_word(dd)
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_xori(self):
        """Executes the XORI instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vAC = self.get_vAC()
        vAC = vAC ^ dd
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_xorw(self):
        """Executes the XORW instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vAC = self.get_vAC()
        vAC = vAC ^ self.read_word(dd)
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_peek(self):
        """Executes the PEEK instruction."""
        vPC = self.get_vPC()
        vAC = self.get_vAC()
        vAC = self.read_byte(vAC)
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 1))

    def execute_deek(self):
        """Executes the DEEK instruction."""
        vPC = self.get_vPC()
        vAC = self.get_vAC()
        vAC = self.read_word(vAC)
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 1))

    def execute_poke(self):
        """Executes the POKE instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        addr = self.read_word(dd)
        vAC = self.get_vAC()
        self.write_byte(addr, vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_doke(self):
        """Executes the DOKE instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        addr = self.read_word(dd)
        vAC = self.get_vAC()
        self.write_word(addr, vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_lup(self):
        """Executes the LUP instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vAC = self.get_vAC()
        if self._lup_callback is not None:
            vAC = self._lup_callback(self, self.relative(vAC, dd))
        self.set_vAC(vAC)
        self.set_vPC(self.relative(vPC, 2))

    def execute_bra(self):
        """Executes the BRA instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vPC = self.absolute(vPC, dd + 2)
        self.set_vPC(vPC)

    def execute_bcc(self):
        """Executes the several BCC instructions (conditional branches)."""
        vPC = self.get_vPC()
        cc = self.read_byte(self.relative(vPC, 1))
        dd = self.read_byte(self.relative(vPC, 2))
        vAC = self.get_vAC()
        jump = ((cc == 0x3F) and (vAC == 0)) or\
               ((cc == 0x72) and (vAC != 0)) or\
               ((cc == 0x50) and (vAC >= 0x8000)) or\
               ((cc == 0x4D) and (vAC > 0) and (vAC < 0x8000)) or\
               ((cc == 0x56) and ((vAC >= 0x8000) or (vAC == 0))) or\
               ((cc == 0x53) and (vAC < 0x8000))

        if jump:
            vPC = self.absolute(vPC, dd + 2)
        else:
            vPC = self.relative(vPC, 3)
        self.set_vPC(vPC)

    def execute_call(self):
        """Executes the CALL instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        addr = self.read_word(dd)
        self.set_vLR(self.relative(vPC, 2))
        self.set_vPC(addr)

    def execute_ret(self):
        """Executes the RET instruction."""
        vLR = self.get_vLR()
        self.set_vPC(vLR)

    def execute_push(self):
        """Executes the PUSH instruction."""
        vPC = self.get_vPC()
        vSP = self.get_vSP()
        vSP = (vSP - 2) & 0xFF
        vLR = self.get_vLR()
        self.write_word(vSP, vLR)
        self.set_vSP(vSP)
        self.set_vPC(self.relative(vPC, 1))

    def execute_pop(self):
        """Executes the POP instruction."""
        vPC = self.get_vPC()
        vSP = self.get_vSP()
        vLR = self.read_word(vSP)
        vSP = (vSP + 2) & 0xFF
        self.set_vSP(vSP)
        self.set_vLR(vLR)
        self.set_vPC(self.relative(vPC, 1))

    def execute_alloc(self):
        """Executes the ALLOC instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vSP = self.get_vSP()
        vSP += dd
        self.set_vSP(vSP)
        self.set_vPC(self.relative(vPC, 2))

    def execute_sys(self):
        """Executes the SYS instruction."""
        if self._sys_callback is not None:
            self._sys_callback(self)
        vPC = self.get_vPC()
        self.set_vPC(self.relative(vPC, 2))

    def execute_def(self):
        """Executes the DEF instruction."""
        vPC = self.get_vPC()
        dd = self.read_byte(self.relative(vPC, 1))
        vAC = self.relative(vPC, 2)
        self.set_vAC(vAC)

        vPC = self.absolute(vPC, dd)
        self.set_vPC(vPC)

    def execute_calli(self):
        """Executes the CALLI instruction."""
        if not self._enable_experimental:
            raise RuntimeError("experimental instruction!")

        vPC = self.get_vPC()
        self.set_vLR(self.relative(vPC, 2))
        self.set_vPC(self.read_word(self.relative(vPC, 1)))

    def execute_cmphs(self):
        """Executes the CMPHS instruction."""
        if not self._enable_experimental:
            raise RuntimeError("experimental instruction!")

        vPC = self.get_vPC()
        vAC = self.get_vAC()
        vACH = vAC >> 8
        addr = self.read_byte(self.relative(vPC, 1))
        b = self.read_byte(addr)

        if (vACH ^ b) & 0x80 != 0:
            if b & 0x80 != 0:
                vAC = ((b + 1) & 0xFF) << 8 | (vAC & 0xFF)
            else:
                vAC = ((b - 1) & 0xFF) << 8 | (vAC & 0xFF)

        self.set_vPC(self.relative(vPC, 2))

    def execute_cmphu(self):
        """Executes the CMPHU instruction."""
        if not self._enable_experimental:
            raise RuntimeError("experimental instruction!")

        vPC = self.get_vPC()
        vAC = self.get_vAC()
        vACH = vAC >> 8
        addr = self.read_byte(self.relative(vPC, 1))
        b = self.read_byte(addr)

        if (vACH ^ b) & 0x80 != 0:
            if b & 0x80 != 0:
                vAC = ((b - 1) & 0xFF) << 8 | (vAC & 0xFF)
            else:
                vAC = ((b + 1) & 0xFF) << 8 | (vAC & 0xFF)

        self.set_vPC(self.relative(vPC, 2))
