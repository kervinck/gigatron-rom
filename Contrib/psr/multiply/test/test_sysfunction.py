"""Tests for the implementation of SYS_MultiplyBytes_126"""
import os.path
import pathlib
from importlib import reload
from types import SimpleNamespace

from hypothesis import given
from hypothesis import strategies as st

import asm
from gtemu import RAM, Emulator

MAX_CYCLES = 120


SYS_DIR = (pathlib.Path(__file__).parent / ".." / "sys").resolve()
SCRIPT = SYS_DIR / "ROM.asm.py"


def setup_module():
    global vars
    """Load the Emulator from the ROM script"""
    reload(asm)
    name, _ = os.path.splitext(os.path.basename(SCRIPT))
    script_globals = {"__file__": str(SCRIPT.absolute()), "__name__": name}
    with SCRIPT.open("rb") as file:
        exec(compile(file.read(), SCRIPT, "exec"), script_globals)
    Emulator.load_rom_from_asm_module()
    vars = SimpleNamespace(**script_globals)


def setup_function():
    RAM[vars.sysFn : vars.sysFn + 2] = asm.symbol("SYS_MultiplyBytes_120").to_bytes(
        2, "little"
    )
    RAM[vars.vTicks] = 75
    Emulator.next_instruction = "SYS"
    Emulator.AC = 270 - max(14, MAX_CYCLES // 2)


def test_timing_both_lt_128():
    """Follow the routine through, checking the timing comments

    This follows the case where both values are less than 128
    I'm just trying to check that the comments are correct!
    """
    RAM[vars.sysArgs : vars.sysArgs + 2] = 3, 5
    # fmt: off
    cycles = 9  # On entry to SYS, 9 cycles have already elapsed
    cycles += Emulator.run_to("SYS_MultiplyBytes_120");                              assert 14 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("sys_MultiplyBytes.tableEntry");                       assert 29 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("sys_MultiplyBytes.high-byte-action.store-inverted");  assert 35 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("sys_MultiplyBytes.tableExit");                        assert 40 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("sys_MultiplyBytes#44");                               assert 43 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("sys_MultiplyBytes.tableEntry");                       assert 51 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("sys_MultiplyBytes.high-byte-action.restore-and-add"); assert 57 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("sys_MultiplyBytes.tableExit");                        assert 64 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("sys_MultiplyBytes#68");                               assert 67 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("NEXTY");                                              assert 90 == cycles        # noqa: E702, E241, E272
    # fmt: on


def test_timing_neither_lt_128():
    """Follow the routine through, checking the timing comments

    This follows the case where neither value is less than 128
    """
    RAM[vars.sysArgs : vars.sysArgs + 2] = 172, 160
    # fmt: off
    cycles = 9  # On entry to SYS, 9 cycles have already elapsed
    cycles += Emulator.run_to("sys_MultiplyBytes#68");  assert 67  == cycles        # noqa: E702, E241, E272, E221
    cycles += Emulator.run_to("sys_MultiplyBytes#92");  assert 91  == cycles        # noqa: E702, E241, E272, E221
    cycles += Emulator.run_to("sys_MultiplyBytes#114"); assert 113 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("NEXTY");                 assert 118 == cycles        # noqa: E702, E241, E272
    # fmt: on


def test_timing_one_lt_128():
    """Follow the routine through, checking the timing comments

    This follows the case where one value is less than 128
    """
    RAM[vars.sysArgs : vars.sysArgs + 2] = 3, 160
    # fmt: off
    cycles = 9  # On entry to SYS, 9 cycles have already elapsed
    cycles += Emulator.run_to("sys_MultiplyBytes#68");            assert 67 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("sys_MultiplyBytes.oneMsbSetCase"); assert 85 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("sys_MultiplyBytes#92");            assert 91 == cycles        # noqa: E702, E241, E272
    # fmt: on


def _sign_extend(byte_):
    if byte_ & 0x80:
        return ~0xFF | byte_
    return byte_


_bytes = st.integers(min_value=0, max_value=255)


@given(a=_bytes, b=_bytes)
def test_multiply_bytes(a, b):
    setup_function()
    RAM[vars.sysArgs : vars.sysArgs + 2] = a, b

    cycles = 10  # Because Next is marked as zero
    cycles += Emulator.run_to("NEXT")

    assert cycles <= MAX_CYCLES
    assert cycles == _sign_extend(Emulator.AC) * -2
    assert a * b == Emulator.vAC
