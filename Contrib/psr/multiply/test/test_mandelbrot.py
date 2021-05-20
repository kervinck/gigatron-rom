import os.path
import pathlib
from importlib import reload
from types import SimpleNamespace

import gcl0x as gcl
from hypothesis import given
from hypothesis import strategies as st

import asm
from gtemu import RAM, Emulator

ROOT = (pathlib.Path(__file__).parent / "..").resolve()
SCRIPT = ROOT / "sys" / "ROM.asm.py"
GCL = ROOT / "mandelbrot" / "Mandelbrot.gcl"


def setup_module():
    global vars, symbol_table, execution_address
    """Load the Emulator from the ROM script and Mandelbrot
    """
    reload(asm)
    name, _ = os.path.splitext(os.path.basename(SCRIPT))
    script_globals = {"__file__": str(SCRIPT.absolute()), "__name__": name}
    with SCRIPT.open("rb") as file:
        exec(compile(file.read(), SCRIPT, "exec"), script_globals)
    Emulator.load_rom_from_asm_module()
    vars = SimpleNamespace(**script_globals)

    # This sequence of calls is roughly taken from compilegcl.py
    old_rom_size = asm._romSize
    program = gcl.Program("Mandelbrot", forRom=False)
    user_code = asm.symbol("userCode")
    user_vars = asm.symbol("userVars")
    program.org(user_code)
    asm.align(1)
    asm.zpReset(user_vars)
    with GCL.open("r", encoding="utf-8") as fp:
        for line in fp.readlines():
            program.line(line)
    program.end()
    asm.end()

    # Copy the resulting data straight into RAM, in the appropriate blocks
    data = asm.getRom1()[old_rom_size:]
    index, more_data = 0, bool(data)
    while more_data:
        start_address = int.from_bytes(data[index : index + 2], "big", signed=False)
        index += 2
        size = data[index] or 256
        index += 1
        chunk = data[index : index + size]
        index += size
        RAM[start_address : start_address + len(chunk)] = chunk
        more_data = data[index]
    execution_address = program.execute
    symbol_table = program.vars


def _prepare_for_vcpu_execution_at(execution_address):
    Emulator.AC = 127
    Emulator.next_instruction = "ENTER"
    RAM[vars.vReturn] = vars.videoZ & 0xFF
    RAM[vars.vCpuSelect] = Emulator.next_instruction >> 8

    Emulator.vLR = execution_address
    Emulator.vPC = execution_address & 0xFF00 | (execution_address - 2) & 0xFF


def _call_vcpu_function(variable):
    return_point = Emulator.vPC & 0xFF00 | (Emulator.vPC + 2) & 0xFF
    Emulator.AC = symbol_table[variable]
    Emulator.next_instruction = "CALL"
    Emulator.step_vcpu()  # Execute CALL
    Emulator.run_vcpu_to(return_point)


def _write_word(address, value, *, signed):
    RAM[address : address + 2] = value.to_bytes(2, "little", signed=signed)


def _read_word(address, *, signed):
    return int.from_bytes(RAM[address : address + 2], "little", signed=signed)


_signed_ints_with_12bit_range = st.integers(
    min_value=-(1 << 11), max_value=(1 << 11) - 1
)


@given(a=_signed_ints_with_12bit_range, b=_signed_ints_with_12bit_range)
def test_MulShift8(a, b):
    _prepare_for_vcpu_execution_at(execution_address)
    Emulator.run_vcpu_to(0x300)  # Run the first page, so the variable is defined
    _write_word(vars.sysFn, asm.symbol("SYS_MultiplyBytes_120"), signed=False)
    _write_word(symbol_table["A"], a, signed=True)
    _write_word(symbol_table["B"], b, signed=True)

    _call_vcpu_function("MulShift8")

    assert int(a * b / 256) == _read_word(vars.vAC, signed=True)
