"""Tests for reading and writing using the serial port"""
import pathlib

from gtemu import Emulator

_ROM_FILE = pathlib.Path(__file__).parent / ".." / "roms" / "echo.rom"
# _ROM_FILE = pathlib.Path(__file__).parent /'..'/ '..'/ '..'/'..'/'ROMv4.rom'


def setup_module():
    Emulator.load_rom_file(_ROM_FILE)


def setup_function():
    Emulator.reset()


def test_read_initial_state():
    # The following are not necessary, but it was very helpful in debugging
    Emulator.run_to(0x2FF, max_instructions=10_000_000)  # Entry of vCPU
    Emulator.run_vcpu_to(0x200)  # Pass through loading sequence
    buffer = bytearray()
    for _ in range(5):
        value = Emulator.read_serial(bits=8)
        buffer.append(value)
    assert buffer == b"READY"


def _read_bytes(n):
    buffer = bytearray()
    for _ in range(n):
        buffer.append(Emulator.read_serial(bits=8))
    return bytes(buffer)


def test_write():
    assert _read_bytes(5) == b"READY"
    for b in b"Hello":
        Emulator.send_byte(b)
    assert _read_bytes(5) == b"Hello"
