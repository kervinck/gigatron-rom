import copy
import pathlib
from importlib import reload
from unittest.mock import patch

import gcl0x as gcl

import asm
from gtemu import RAM, Emulator

ROOT = (pathlib.Path(__file__).parent).resolve()
REPO_ROOT = (ROOT / ".." / ".." / "..").resolve()

SCRIPT = ROOT / "sys" / "ROM.asm.py"
RESET = REPO_ROOT / "Core" / "Reset_v5.gcl"
BOOT = REPO_ROOT / "Apps" / "CardTest" / "CardBoot.gcl"
ORIGINAL_MANDELBROT = REPO_ROOT / "Apps" / "Mandelbrot" / "Mandelbrot_v1.gcl"
MY_MANDELBROT = ROOT / "mandelbrot" / "Mandelbrot.gcl"


CLOCK_FREQUENCY = 6.250e06  # Hz
CLOCK_PERIOD = 1 / CLOCK_FREQUENCY


for file in [SCRIPT, RESET, BOOT, ORIGINAL_MANDELBROT, MY_MANDELBROT]:
    assert file.exists(), f"File does not exist: {file}"


script_globals = {"__file__": str(SCRIPT.absolute()), "__name__": "__main__"}
with SCRIPT.open("rb") as file:
    compiled_script = compile(file.read(), SCRIPT, "exec")
script_argv = [str(SCRIPT.name), f"Reset={RESET}", f"Boot={BOOT}"]


def get_symbol_table(gcl_file):
    """Compile and discard gcl, and return the symbol table

    Requires the rom script to already have been run in so that the
    """
    # Execute the hacked ROM script, to define all of the required variables
    reload(asm)
    asm.define("Main", 0x0000)  # Must be defined to something
    with patch("sys.argv", script_argv), patch("asm.writeRomFiles"), patch(
        "asm.enableListing"
    ), patch("asm.disableListing"), patch("asm.print"), patch("gcl0x.print"):
        exec(compiled_script, copy.copy(script_globals))
    asm.align(1)
    asm.zpReset(asm.symbol("userVars"))
    program = gcl.Program("Mandelbrot", forRom=False)
    program.org(asm.symbol("userCode"))
    with gcl_file.open("r", encoding="utf-8") as fp:
        for line in fp.readlines():
            with patch("gcl0x.print"):
                program.line(line)
    with patch("gcl0x.print"):
        program.end()
    asm.end()
    return program.vars


def compile_and_load_rom(main_gcl):
    reload(asm)
    with patch("sys.argv", script_argv + [f"Main={main_gcl}"]), patch(
        "asm.writeRomFiles"
    ), patch("asm.enableListing"), patch("asm.disableListing"), patch(
        "asm.print"
    ), patch(
        "gcl0x.print"
    ):
        exec(compiled_script, copy.copy(script_globals))
    Emulator.reset()  # To reset PC etc. should really be part of load_rom_from_asm_module
    Emulator.load_rom_from_asm_module()


def _read_word(address, *, signed):
    return int.from_bytes(RAM[address : address + 2], "little", signed=signed)


for name, gcl_file in [("Original", ORIGINAL_MANDELBROT), ("Modified", MY_MANDELBROT)]:
    symbols = get_symbol_table(gcl_file)
    compile_and_load_rom(gcl_file)
    message = f"Running {name}"
    print(message)
    print("=" * len(message))
    cycles = Emulator.run_to("SYS_Reset_88", max_instructions=10_000_000)
    print(f"Boot reached SYS_RESET_88 in {cycles} cycles ({cycles * CLOCK_PERIOD}s)")
    cycles += Emulator.run_to("SYS_Exec_88")
    assert _read_word(asm.symbol("sysArgs0"), signed=False) == asm.symbol("Reset")
    print(
        f"Boot reached SYS_Exec_88 (Reset) in {cycles} cycles ({cycles * CLOCK_PERIOD}s)"
    )
    cycles += Emulator.run_to("SYS_Exec_88", max_instructions=10_000_000)
    assert _read_word(asm.symbol("sysArgs0"), signed=False) == asm.symbol("Main")
    print(
        f"Boot reached SYS_Exec_88 (Main) in {cycles} cycles ({cycles * CLOCK_PERIOD}s)"
    )
    cycles += Emulator.run_vcpu_to(0x200)
    print(
        f"Program was ready to begin exectution after {cycles} cycles ({cycles * CLOCK_PERIOD}s)"
    )
    # Can't run to CALL, as it's included in DEF
    cycles += Emulator.run_to(asm.symbol("CALL") + 1, max_instructions=10_000)
    assert Emulator.AC == symbols["CalcSet"], (Emulator.AC, symbols["CalcSet"])
    cycles += Emulator.step_vcpu()
    print(
        f"Program initialisation was complete after {cycles} cycles ({cycles * CLOCK_PERIOD}s)"
    )
    cycles += Emulator.run_vcpu_to(Emulator.vLR)
    print(f"CalcSet was complete after {cycles} cycles ({cycles * CLOCK_PERIOD}s)")
    print()
