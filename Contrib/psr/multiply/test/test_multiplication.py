import math
import os.path
import pathlib

from hypothesis import given
from hypothesis import strategies as st

import asm
from gtemu import Emulator

SRC_DIR = (pathlib.Path(__file__).parent / ".." / "src").resolve()
SCRIPT = SRC_DIR / "multiplication.asm.py"


def setup_module():
    """Load the Emulator from the multiplication script"""
    name, _ = os.path.splitext(os.path.basename(SCRIPT))
    script_globals = {"__file__": str(SCRIPT.absolute()), "__name__": name}
    with SCRIPT.open("rb") as file:
        exec(compile(file.read(), SCRIPT, "exec"), script_globals)
    Emulator.load_rom_from_asm_module()


@given(value=st.integers(min_value=0, max_value=255))
def test_low_byte_lookup(value):
    """Lookup of the low-byte of a quarter square should work"""
    Emulator.Y = asm.symbol("low-byte return point") >> 8
    Emulator.AC = value
    Emulator.next_instruction = "low-byte table entry"

    Emulator.run_to("low-byte return point")

    assert int(math.floor((value ** 2) / 4)) & 0xFF == Emulator.AC


@given(value=st.integers(min_value=0, max_value=255))
def test_high_byte_lookup(value):
    """Lookup of the high-byte of a quarter square should work"""
    Emulator.AC = value
    Emulator.next_instruction = "high-byte table entry"

    Emulator.run_to("high-byte return point")

    assert int(math.floor((value ** 2) / 4)) >> 8 == Emulator.AC
