"""Implementation of multiplication that uses the following identity:

a * b = (a² + b² - (a-b)²)/2

We use this to implement multiplication of nibbles, and from that multiplication of bytes.

We make use of a lookup table of ⌊n²/2⌋ for n in the range [-15,15],
and calculate (⌊a²/2⌋ + ⌊b²/2⌋ - ⌊(a-b)²/2⌋ + (a & b & 1)) - The a & b & 1 term compensates
for the fact that (⌊a²/2⌋ + ⌊b²/2⌋ - ⌊(a-b)²/2⌋) is off by one when both a and b are odd.
We also have a right-shift by four table.

Memory is used as follows:

(0x1,0x2) = Result (would be vAC in a real sys function)
(0x3) = very brief temporary storage (would be vTmp)
(0x4, 0x5) = A, B - Operands, would be sysArgs[0:2] - untouched.
(0x6..) = Variables. Would be sysArgs[2:] - but we try to leave the last value untouched.
"""
import sys
from itertools import chain, count

import asm
from asm import (
    AC,
    C,
    X,
    Xpp,
    Y,
    adda,
    align,
    anda,
    bne,
    bra,
    define,
    end,
    fillers,
    label,
    ld,
    ora,
    pc,
    st,
    suba,
    writeRomFiles,
    xora,
)

DEBUG = False  # Set to True to get debugging output


# I'm keen to avoid using too many variables.
# The comments that this outputs should help me keep things clean
class VariableTracker:
    def __init__(self, size=12, initial_x=6):
        self._x = initial_x
        self._current_bindings = ["unused"] * size

    @staticmethod
    def _comment_next_instruction(message):
        # Hack to add a comment on the next instruction, not the previous.
        address = max(0, asm._romSize)
        line_comments = asm._comments.setdefault(address, [])
        line_comments.append(f";{message}")

    def define(self, name, address, *, suppress_comment=False):
        assert self._current_bindings[address] in ("dead", "unused")
        self._current_bindings[address] = name
        if not suppress_comment:
            self._comment_next_instruction(f"[${address:02x}] <- {name}.")
        if DEBUG:
            print(self)
        return address

    def __repr__(self) -> str:
        return f"{self._current_bindings} x={self._x}"

    def reset_x(self, new_x):
        self._x = new_x
        return new_x

    def define_from_x_postincrement(self, name, *, suppress_comment=False):
        assert self._current_bindings[self._x] in ("dead", "unused"), self
        self._current_bindings[self._x] = name
        if not suppress_comment:
            C(f"[${self._x:02x}] <- {name}; x <- {self._x + 1}")
        self._x += 1
        if DEBUG:
            print(self)

    def read(self, name, *, suppress_comment=False):
        if not suppress_comment:
            self._comment_next_instruction(f"({name})")
        return self._current_bindings.index(name)

    def update(self, name, *, suppress_comment=False):
        if not suppress_comment:
            self._comment_next_instruction(f"Update {name}")
        return self._current_bindings.index(name)

    def kill(self, name, *, suppress_comment=False):
        address = self._current_bindings.index(name)
        self._current_bindings[address] = "dead"
        if not suppress_comment:
            self._comment_next_instruction(f"({name})")
        return address

    def get_name(self, address):
        return self._current_bindings[address]

    def assert_x(self, n):
        assert self._x == n, f"Assert x: have {self._x}, but wanted {n}. {self}"


## Lookup tables are woven through the code, and we need to avoid them.
_emit_half_square = lambda i: st(i, [Y, Xpp])


class LookupTableManager:

    half_square_table = [
        (i, _emit_half_square, i**2 // 2, f"[x++] <- ⌊${i:x}²/2⌋")
        for i in range(0x10)
    ]

    half_square_table_inverted = [
        (i & 0xFF, _emit_half_square, (i**2 // 2), f"[x++] <- ⌊(${i:x})²/2⌋")
        for i in range(-1, -0x10, -1)
    ]
    right_shift_table = [
        (i * 16, ld, i, f"Right-shift by four lookup for ${i:x}x")
        for i in range(1, 0x10)
    ]

    def __init__(self):
        all_entries = chain(
            self.half_square_table,
            self.half_square_table_inverted,
            self.right_shift_table,
        )
        self.table_entries = iter(sorted(all_entries))
        self.next_entry = next(self.table_entries)

    def emit_next_entry(self):
        address, emit, value, comment = self.next_entry
        assert address >= (
            pc() & 0xFF
        ), f"Next address ({address:x}) is before the current PC ({pc() & 0xff:x}) - we missed it!"
        fillers(until=address)
        while address == pc() & 0xFF:
            emit(value)
            C(comment)
            address, emit, value, comment = next(self.table_entries)
        self.next_entry = address, emit, value, comment

    def finish_tables(self):
        address, emit, value, comment = self.next_entry
        assert address >= (
            pc() & 0xFF
        ), f"Next address ({address:x}) is before the current PC ({pc() & 0xff:x}) - we missed it!"

        fillers(until=address)
        emit(value)
        C(comment)
        for address, emit, value, comment in self.table_entries:
            fillers(until=address)
            emit(value)
            C(comment)


vars = VariableTracker()
labels = (f".multiply.{i}" for i in count(1))
tables = LookupTableManager()


def right_shift(*, emit_next_table_entry=False):
    next_label = next(labels)
    bne(AC)
    C("ac >>= 4")
    bra(next_label)
    ld(0)  # A nop, in this case
    if emit_next_table_entry:
        tables.emit_next_entry()
    label(next_label)


def lookup_and_store_half_square(
    src, operand_name=None, *, kill=False, emit_next_table_entry=False
):
    assert (src is not AC) or (operand_name is not None)
    operand_name = operand_name or src
    target_name = (
        f"⌊{operand_name}²/2⌋" if " " not in operand_name else f"⌊({operand_name})²/2⌋"
    )
    if src is AC:
        bra(AC)
    elif kill:
        bra([vars.kill(src, suppress_comment=True)])
    else:
        bra([vars.read(src, suppress_comment=True)])
    vars.define_from_x_postincrement(target_name)
    next_label = next(labels)
    bra(next_label)
    if emit_next_table_entry:
        tables.emit_next_entry()
    label(next_label)


### Code starts here
define("A", vars.define("A", 0x4, suppress_comment=True))
define("B", vars.define("B", 0x5, suppress_comment=True))


align(0x100)
tables.emit_next_entry()

label("start")
ld([vars.read("A")])
anda(0x0F)
st([vars.define("A-low", 0x1)])
xora([vars.read("A")])
right_shift()
st([vars.define("A-high", 0x6)])
ld([vars.read("B")])
anda(0xF0)
right_shift(emit_next_table_entry=True)
st([vars.define("B-high", 0x3)])

# Start product of high-nibbles
ld(0, Y)
ld(vars.reset_x(0x07), X)
suba([vars.read("A-high")])
lookup_and_store_half_square(AC, "A-high - B-high")
ld([vars.read("B-high")])
anda([vars.read("A-high")])
anda(0x01)
suba([vars.kill("⌊(A-high - B-high)²/2⌋")])
st([vars.define("high-byte total", 0x02)])

# A-low * B-high
ld([vars.read("A-low")])
suba([vars.read("B-high")])
lookup_and_store_half_square(AC, "A-low - B-high", emit_next_table_entry=True)
ld([vars.read("B-high")])
lookup_and_store_half_square("B-high", kill=True)
lookup_and_store_half_square("A-low", emit_next_table_entry=True)
anda([vars.read("A")])
anda(0x01)
adda([vars.read("⌊A-low²/2⌋")])
adda([vars.read("⌊B-high²/2⌋")])
suba([vars.kill("⌊(A-low - B-high)²/2⌋")])
st([vars.define("A-low * B-high", 0x03)])
anda(0x0F)
st([vars.define("low(A-low * B-high)", 0x07)])
xora([vars.kill("A-low * B-high")])
right_shift(emit_next_table_entry=True)
adda([vars.kill("⌊B-high²/2⌋")])
adda([vars.read("high-byte total")])
st([vars.update("high-byte total")])

ld(vars.reset_x(0x8), X)
ld([vars.read("B")])
anda(0x0F)
st([vars.define("B-low", 0x03)])
suba([vars.read("A-high")])
print(vars)
lookup_and_store_half_square(
    AC, operand_name="B-low - A-high", emit_next_table_entry=True
)
lookup_and_store_half_square("B-low", kill=True)
ld([vars.read("A-high")])
ld(vars.reset_x(0x6), X)
lookup_and_store_half_square("A-high", kill=True, emit_next_table_entry=True)
anda([vars.read("B")])
anda(0x01)
adda([vars.read("⌊B-low²/2⌋")])
adda([vars.read("⌊A-high²/2⌋")])
suba([vars.kill("⌊(B-low - A-high)²/2⌋")])
st([vars.define("B-low * A-high", 0x03)])
anda(0x0F)
st([vars.define("low(B-low * A-high)", 0x08)])
xora([vars.kill("B-low * A-high")])
right_shift(emit_next_table_entry=True)
adda([vars.kill("⌊A-high²/2⌋")])
adda([vars.read("high-byte total")])
st([vars.update("high-byte total")])

# Product of low-nibbles
ld(vars.reset_x(6), X)
ld([vars.read("B")])
anda(0x0F)  # Because we lost B-low
suba([vars.read("A-low")])
lookup_and_store_half_square(
    AC, operand_name="B-low - A-low", emit_next_table_entry=True
)
ld([vars.kill("A-low")])
anda([vars.read("B")])
anda(0x01)
adda([vars.kill("⌊B-low²/2⌋")])
adda([vars.kill("⌊A-low²/2⌋")])
suba([vars.kill("⌊(B-low - A-low)²/2⌋")])
st([vars.define("A-low * B-low", 0x3)])

print(vars)
# Final sum - 23 cycles
anda(0xF)
st([vars.define("result low nibble", 0x01)])
xora([vars.kill("A-low * B-low")])
right_shift(emit_next_table_entry=True)
adda([vars.kill("low(A-low * B-high)")])
adda([vars.kill("low(B-low * A-high)")])
st([vars.define("temp", 0x3)])
adda(AC)
adda(AC)
adda(AC)
adda(AC)
ora([vars.kill("result low nibble")])
st([vars.define("low(result)", 0x01)])
ld([vars.kill("temp")])
anda(0xF0)
right_shift(emit_next_table_entry=True)
adda([vars.kill("high-byte total")])
st([vars.define("high(result)", 0x2)])
label("end")
ld(AC, AC)
tables.finish_tables()
end()
define("result", 0x01)
if __name__ == "__main__":
    writeRomFiles(sys.argv[0])
