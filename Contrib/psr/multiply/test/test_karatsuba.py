"""Tests for the implementation of SYS_KaratsubaPrepare_54"""
import os.path
import pathlib
from importlib import reload
from types import SimpleNamespace

from hypothesis import given
from hypothesis import strategies as st

import asm
from gtemu import RAM, Emulator

MAX_CYCLES = 54


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
    RAM[vars.sysFn : vars.sysFn + 2] = asm.symbol("SYS_KaratsubaPrepare_54").to_bytes(
        2, "little"
    )
    RAM[vars.vTicks] = 75
    Emulator.next_instruction = "SYS"
    Emulator.AC = 270 - max(14, MAX_CYCLES // 2)


def _sign_extend(byte_):
    if byte_ & 0x80:
        return ~0xFF | byte_
    return byte_


_unsigned_words = st.integers(min_value=0, max_value=1 << 16 - 1)


@given(a=_unsigned_words, b=_unsigned_words)
def test_karatsuba_prepare(a, b):
    setup_function()  # Because Hypothesis calls us repeatedly, and the function under test changes sysFn etc.
    RAM[vars.sysArgs : vars.sysArgs + 2] = a.to_bytes(2, "little", signed=False)
    RAM[vars.vAC : vars.vAC + 2] = b.to_bytes(2, "little", signed=False)

    cycles = 10  # 9 Cycles from previous next, plus one, because, I dunno, but you do.
    cycles += Emulator.run_to("NEXT")

    assert abs((a & 0xFF) - (a >> 8)) == RAM[vars.sysArgs + 0]
    assert abs((b >> 8) - (b & 0xFF)) == RAM[vars.sysArgs + 1]
    assert ((a >> 8) > (a & 0xFF)) ^ ((b & 0xFF) > (b >> 8)) == RAM[vars.vAC]
    assert int.from_bytes(
        RAM[vars.sysFn : vars.sysFn + 2], "little", signed=False
    ) == asm.symbol("SYS_MultiplyBytes_120")
    assert RAM[vars.vAC] == RAM[vars.vAC + 1]
    assert cycles == MAX_CYCLES
    assert cycles == _sign_extend(Emulator.AC) * -2


def test_timing_both_overflow():
    """Follow the routine through, checking the timing comments

    This follows the case where both subtractions result in a negative number:
    the high byte of vAC is bigger than the low, and
    the low byte of sysArgs[0:2] is bigger than the high.
    """
    RAM[vars.sysArgs : vars.sysArgs + 2] = 0x00FF.to_bytes(2, "little", signed=False)
    RAM[vars.vAC : vars.vAC + 2] = 0xFF00.to_bytes(2, "little", signed=False)
    # fmt: off
    cycles = 9  # On entry to SYS, 9 cycles have already elapsed
    cycles += Emulator.run_to("SYS_KaratsubaPrepare_54");           assert 14 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("sysKaratsubaPrepare#18");            assert 17 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("sysKaratsubaPrepare#21");            assert 20 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("sysKaratsubaPrepare#35");            assert 34 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("REENTER");                           assert 51 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("NEXT");                              assert 53 == cycles        # noqa: E702, E241, E272
    # fmt: on


def test_timing_neither_overflow():
    """Follow the routine through, checking the timing comments

    This follows the case where both subtractions result in a positive number:
    the low byte of vAC is bigger than the high, and
    the high byte of sysArgs[0:2] is bigger than the low.
    """
    RAM[vars.sysArgs : vars.sysArgs + 2] = 0xFF00.to_bytes(2, "little", signed=False)
    RAM[vars.vAC : vars.vAC + 2] = 0x00FF.to_bytes(2, "little", signed=False)
    # fmt: off
    cycles = 9  # On entry to SYS, 9 cycles have already elapsed
    cycles += Emulator.run_to("SYS_KaratsubaPrepare_54");           assert 14 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("sysKaratsubaPrepare#21");            assert 20 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("sysKaratsubaPrepare#35");            assert 34 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("REENTER");                           assert 51 == cycles        # noqa: E702, E241, E272
    cycles += Emulator.run_to("NEXT");                              assert 53 == cycles        # noqa: E702, E241, E272
    # fmt: on
