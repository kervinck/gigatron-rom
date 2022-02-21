import sys
from itertools import chain, count

import asm
from asm import (
    AC,
    C,
    Xpp,
    Y,
    align,
    bne,
    bra,
    define,
    end,
    fillers,
    label,
    ld,
    pc,
    st,
    writeRomFiles,
)

DEBUG = True


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

label("end")
ld(AC, AC)
tables.finish_tables()
end()
define("result", 0x01)
if __name__ == "__main__":
    writeRomFiles(sys.argv[0])
