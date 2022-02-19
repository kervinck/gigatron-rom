import math
import os.path
import pathlib
from importlib import reload
from types import SimpleNamespace

from hypothesis import given
from hypothesis import strategies as st

import asm
from gtemu import RAM, Emulator

SRC_DIR = (pathlib.Path(__file__).parent / ".." / "src").resolve()
SCRIPT = SRC_DIR / "multiplication.asm.py"


def setup_module():
    global vars
    """Load the Emulator from the multiplication script"""
    reload(asm)
    name, _ = os.path.splitext(os.path.basename(SCRIPT))
    script_globals = {"__file__": str(SCRIPT.absolute()), "__name__": name}
    with SCRIPT.open("rb") as file:
        exec(compile(file.read(), SCRIPT, "exec"), script_globals)
    Emulator.load_rom_from_asm_module()
    vars = SimpleNamespace(**script_globals)


_bytes = st.integers(min_value=0, max_value=255)


@given(value=_bytes)
def test_low_byte_lookup(value):
    """Lookup of the low-byte of a quarter square should work"""
    Emulator.Y = asm.symbol("Quarter-squares lookup table") >> 8
    Emulator.AC = value
    Emulator.next_instruction = "low-byte table entry"

    cycles = Emulator.run_to("low-byte return point")

    assert int(math.floor((value**2) / 4)) & 0xFF == Emulator.AC
    assert vars.cost_of_low_byte_table_entry == cycles


@given(value=_bytes)
def test_high_byte_lookup(value):
    """Lookup of the high-byte of a quarter square should work"""
    RAM[vars.high_byte_action] = asm.symbol("high-byte action.store")
    Emulator.AC = value
    Emulator.Y = asm.symbol("Quarter-squares lookup table") >> 8
    Emulator.next_instruction = "table entry"

    cycles = Emulator.run_to("high-byte action.store")

    assert int(math.floor((value**2) / 4)) >> 8 == Emulator.AC
    assert vars.cost_of_high_byte_table_entry == cycles


_seven_bit_integers = st.integers(min_value=0, max_value=127)


@given(a=_seven_bit_integers, b=_seven_bit_integers)
def test_both_byte_lookup(a, b):
    """Lookup of both bytes of a quarter square should work

    The multiplication routine actually adds 1 to the result,
    so storing it, so check for that.
    """
    RAM[vars.a] = a
    RAM[vars.b] = b
    Emulator.next_instruction = "multiply 7x7"
    expected = int(math.floor(((a + b) ** 2) / 4)) + 1

    cycles = Emulator.run_to(asm.symbol(".after-first-lookup") + 2)

    assert expected == int.from_bytes(
        RAM[vars.result : vars.result + 2], "little", signed=False
    )
    assert cycles == vars.cost_after_first_lookup + 2
    assert asm.symbol("Quarter-squares lookup table") >> 8 == Emulator.Y


@given(
    a=_seven_bit_integers,
    b=_seven_bit_integers,
    previous_value=st.integers(min_value=0, max_value=65025),
)
def test_subtract_quarter_square(a, b, previous_value):
    """to done should subtract the"""
    RAM[vars.a] = a
    RAM[vars.b] = b
    RAM[vars.result : vars.result + 2] = (previous_value + 1).to_bytes(
        2, "little", signed=False
    )
    Emulator.Y = asm.symbol("Quarter-squares lookup table") >> 8
    Emulator.next_instruction = asm.symbol(".after-first-lookup") + 2
    expected = (previous_value - math.floor((a - b) ** 2 / 4)) & 0xFFFF

    cycles = Emulator.run_to(asm.symbol("done"))

    assert expected == int.from_bytes(
        RAM[vars.result : vars.result + 2], "little", signed=False
    )
    assert asm.symbol("Quarter-squares lookup table") >> 8 == Emulator.Y
    assert vars.cost_of_7bit_multiply - vars.cost_after_first_lookup - 2 == cycles


@given(a=_seven_bit_integers, b=_seven_bit_integers)
def test_multiplication_7(a, b):
    """Multiplication of two seven-bit integers should work"""
    RAM[vars.a] = a
    RAM[vars.b] = b
    Emulator.next_instruction = "multiply 7x7"

    cycles = Emulator.run_to("done")

    result = int.from_bytes(RAM[vars.result : vars.result + 2], "little", signed=False)
    assert a * b == result
    assert vars.cost_of_7bit_multiply == cycles


@given(a=_bytes, b=_bytes)
def test_multiplication_8(a, b):
    """Multiplication of two eight-bit integers should work"""
    Emulator.reset()
    RAM[vars.a] = a
    RAM[vars.b] = b
    Emulator.next_instruction = "multiply 8x8"
    expected_saving = (
        vars.no_msb_cost_saving
        if a < 128 and b < 128
        else vars.one_msb_cost_saving
        if a < 128 or b < 128
        else 0
    )

    cycles = Emulator.run_to("done")

    result = int.from_bytes(RAM[vars.result : vars.result + 2], "little", signed=False)
    assert a * b == result
    assert vars.cost_of_8bit_multiply - expected_saving == cycles
