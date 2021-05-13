import math
import sys

from asm import (
    AC,
    C,
    X,
    Y,
    adda,
    align,
    anda,
    bge,
    blt,
    bne,
    bra,
    disableListing,
    enableListing,
    end,
    fillers,
    hi,
    jmp,
    label,
    ld,
    nop,
    ora,
    pc,
    st,
    suba,
    writeRomFiles,
    xora,
    zpByte,
)

if __name__ == "__main__":
    enableListing()

# Inputs
a, b = zpByte(), zpByte()
# Output
result = zpByte(2)
# Used for storage
tmp = zpByte()
# Used for the continuation address after lookup.
continuation = zpByte()
# Used for the action to take with the high byte.
high_byte_action = zpByte()


# The following code implements a lookup table of floored quarter squares,
# for values up to 255.
# This is supposed to enable a fast multiplication for 7-bit numbers.

# First the high-bytes.
# The table is shifted down by 32 places, as the first 32 high-bytes are all zero
# This allows us to have code later in the page which we can branch back to.
align(0x100, size=0x100)
label("Quarter-squares lookup table")
for i in range(32, 256):
    val = math.floor(i ** 2 / 4)
    ld(hi(val))
    C(f"${val:04x} = {val} = floor({i} ** 2 / 4); ${val:04x} >> 8 = ${val >> 8:02x}")

# We jump back here after looking up the low-byte of the result.
label("low-byte return point")
ld(hi("multiply 7x7"), Y)
jmp(Y, [continuation])
ld(hi(pc()), Y)  # Make it easy to get back here!
cost_of_low_byte_return = 3
label("table entry.possibly-negative")
# AC is negative, if b > a. Find absolute value
blt(pc() + 3)  # 1
bra(pc() + 3)  # 2
suba(1)  # 3; if >= 0
xora(0xFF)  # 3;  if < 0
adda(1)  # 4
cost_of_absolute = 4
label("table entry")
# Calculate an index into the high-byte table.
# This is basically a matter of subtracting 32, and jumping in if the result >= 0.
# But values greater than 160 have the sign-bit set after subtraction,
# despite being >32.
# We test for the sign bit and jump after subtraction even if 'negative' in these cases.
st([tmp])  # 1
blt(pc() + 5)  # 2
suba(32)  # 3
bge(AC)  # 4
bra([high_byte_action])  # 5
ld(0)  # 6
bra(AC)  # 4
bra([high_byte_action])  # 5
cost_of_high_byte_table_entry = 6
# Some space here for other code?
fillers(until=251)
# We jump back here after looking up the high-byte of the result.
# Counting is in reverse
label("high-byte action.invert-and-add")
xora(0xFF)  # 4
label("high-byte action.add")
adda([result + 1])  # 3
label("high-byte action.store")
st([result + 1])  # 2
ld([tmp])  # 1
assert pc() & 0xFF == 0xFF, pc()
cost_of_high_byte_invert_and_add = 4
cost_of_high_byte_add = 3
cost_of_high_byte_store = 2
label("low-byte table entry")
# With the index in the accumulator, and the current page in the Y register,
# We jump to the right entry in the low-byte table, replace AC with the result,
# and jump back out to 'low-byte return point' defined above,
# using the double-jump trick.
# This exploits the fact the following is the last instruction on the page,
# and the high byte of the PC has already incremented,
# so the bne will take us to an address in the next page.
# We use the Y register to return to the current page.
# The table has two zeros at the start, meaning that we can replace the first
# with the jump back.
bne(AC)  # 1
align(0x100, size=0x100)
jmp(Y, "low-byte return point")  # 2
ld(0)  # 3
cost_of_low_byte_table_entry = 3
C("0 = floor(0 ** 2 / 4) and floor(1 ** 2 / 4)")
for i in range(2, 256):
    val = math.floor(i ** 2 / 4)
    ld(val)
    C(f"${val:04x} = {val} = floor({i} ** 2 / 4)")


# Code copied from the main ROM. This provides a lookup table for right-shifts
align(0x100, size=0x100)
label("shiftTable")
shiftTable = pc()

for ix in range(255):
    for n in range(1, 9):  # Find first zero
        if ~ix & (1 << (n - 1)):
            break
    pattern = ["x" if i < n else "1" if ix & (1 << i) else "0" for i in range(8)]
    ld(ix >> n)
    C("0b%s >> %d" % ("".join(reversed(pattern)), n))

assert pc() & 255 == 255
bra([tmp])  # Jumps back into next page
align(0x100, size=0x100)
nop()  #

label("multiply 7x7")
# The formula is floor(((a + b) ** 2) / 4) - floor(((a - b) ** 2) / 4)

ld(".after-first-lookup")  # 1
st([continuation])
ld(hi("Quarter-squares lookup table"), Y)
ld("high-byte action.store")
st([high_byte_action])  # 5
ld([a])
jmp(Y, "table entry")  # 7
adda([b])  # 8
cost_to_first_lookup = 8

cost_after_first_lookup = (
    cost_to_first_lookup
    + cost_of_high_byte_table_entry
    + cost_of_high_byte_store
    + cost_of_low_byte_table_entry
    + cost_of_low_byte_return
)

label(".after-first-lookup")
# On return we have the low-byte in the accumulator
# We can safely add one to it without causing an overflow,
# because 255 does not appear in the low-byte table.
# This is part of the following subtraction.
adda(1)  # 1
st([result])
ld(".after-second-lookup")
st([continuation])
ld("high-byte action.invert-and-add")  # 5
st([high_byte_action])
ld([a])
jmp(Y, "table entry.possibly-negative")  # 8
suba([b])  # 9
cost_between_lookups = 9

cost_after_second_lookup = (
    cost_after_first_lookup
    + cost_between_lookups
    + cost_of_absolute
    + cost_of_high_byte_table_entry
    + cost_of_high_byte_invert_and_add
    + cost_of_low_byte_table_entry
    + cost_of_low_byte_return
)

label(".after-second-lookup")
xora(0xFF)  # 1
# We need to add this to the result
# But we may have a carry
adda([result])
st([tmp])
blt(pc() + 4)  # 5
suba([result])  # 6
bra(pc() + 4)  # 7
ora([result])  # 8
bra(pc() + 2)  # 7
anda([result])  # 8
anda(0b1000_0000, X)  # 9
ld([tmp])
st([result])  # 10
ld([X])
adda([result + 1])
st([result + 1])  # 13
cost_of_final_add = 13

cost_of_7bit_multiply = cost_after_second_lookup + cost_of_final_add
C(f"Total cost: {cost_of_7bit_multiply} cycles")
label("done")
end()

if __name__ == "__main__":
    disableListing()
    writeRomFiles(sys.argv[0])
