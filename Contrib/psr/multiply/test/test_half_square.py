import os.path
import pathlib
from importlib import reload
from types import SimpleNamespace

from hypothesis import given
from hypothesis import strategies as st

import asm
from gtemu import RAM, Emulator

SRC_DIR = (pathlib.Path(__file__).parent / ".." / "src").resolve()
SCRIPT = SRC_DIR / "half-square.asm.py"


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


@given(a=_bytes, b=_bytes)
def test_multiplication_8(a, b):
    """Multiplication of two eight-bit integers should work"""
    Emulator.reset()
    RAM[asm.symbol("A")] = a
    RAM[asm.symbol("B")] = b
    Emulator.next_instruction = "start"

    cycles = Emulator.run_to("end")

    result = int.from_bytes(
        RAM[asm.symbol("result") : asm.symbol("result") + 2],
        "little",
        signed=False,
    )
    assert a * b == result
    assert 119 == cycles
