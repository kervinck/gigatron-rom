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
    lo,
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
bra([continuation])  # Jumps back into next page
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
nop()


label("multiply 8x8")
# Extend the 7bit x 7bit multiplication to 8bit x 8bit
#
# The logic goes as follows.
# Let A and B be the low seven bits of a and b, e.g.
# A = a₆2⁶ + a₅2⁵ + a₄2⁴ + a₃2³ + a₂2² + a₁2¹ + a₀2⁰
# B = b₆2⁶ + b₅2⁵ + b₄2⁴ + b₃2³ + b₂2² + b₁2¹ + b₀2⁰
# Then we could think of 8bit x 8bit multiply as
# (a₇2⁷ + A)(b₇2⁷ + B)
# Multiplying out the brackets gives
# a₇2⁷b₇2⁷ + a₇2⁷B + b₇2⁷A + AB
# (and AB is the result we already know how to calculate)
# Simplifying a bit, we get
# a₇b₇2¹⁴ + 2⁷(a₇B + b₇A) + AB
# Since a₇ and b₇ are one or zero, multiplying by them is like an if.
# We can consider four cases,
# if a₇ is 1 and b₇ is 1:
#   2¹⁴ + 2⁷(B + A) + AB
# if a₇ is 1 and b₇ is 0:
#   2⁷B + AB
# if a₇ is 0 and b₇ is 1:
#   2⁷A + AB
# if a₇ is 0 and b₇ is 0:
#   AB
# Multiplication by 2⁷(=128) is a left-shift by seven, which
# can be broken down as moving the LSB of the low-byte to the
# MSB of the low-byte, and setting all of the other bits to zero,
# The other bits of the low byte can be right-shifted by one,
# and moved to the high byte.

# Test which of the branches we are on.
ld([a])  # 1
xora([b])
blt(".one MSB set")  # 3
ld([a])  # 4

# Both MSBs equal
bge("multiply 7x7")  # 5
anda(0b0111_1111)  # 6
cost_of_8bit_multiply__both_msbs_low = cost_of_7bit_multiply + 6

# Both MSBs set
st([a])  # 7; a = A
ld(".after right-shift")
st([continuation])
ld(2 ** 14 >> 8)  # 10; Write the high-byte for later addition.
st([result + 1])
ld([b])
anda(0b0111_1111)
st([b])  # b = B
adda([a])  # 15
cost_of_both_msbs_set = 15

label(". << 7")
st([tmp])  # 1
anda(0b0000_0001)  # Clear all but the bottom bit
adda(0b0111_1111)  # Carries bottom bit to top bit
anda(0b1000_0000)  # Clears all but the top bit
st([result])  # 5
ld([tmp])
anda(0b1111_1110)  # Calculate index to right-shift-table
ld(hi("shiftTable"), Y)
jmp(Y, AC)  # 9
bra(0xFF)  # 10
# 11 ld (a + b) >> 1
# 12 bra [continuation]
# 12 nop
cost_of_right_shift = 13
cost_after_right_shift = cost_of_both_msbs_set + cost_of_right_shift

label(".one MSB set")
bge(".b has msb set")  # 5
ld(0b0111_1111)  # 6
# a has msb set
anda([a])  # 7
st([a])
ld(lo(".after right-shift") + 1)
st([continuation])  # 10
bra(". << 7")  # 11
ld([b])  # 12
label(".b has msb set")
anda([b])  # 7
st([b])
ld(lo(".after right-shift") + 1)
st([continuation])  # 10
bra(". << 7")  # 11
ld([a])  # 12
cost_of_one_msb_set = 12
one_msb_cost_saving = cost_of_both_msbs_set - cost_of_one_msb_set + 1

label(".after right-shift")
adda([result + 1])  # 1
st([result + 1])
ld(".after-first-lookup-8bit")
st([continuation])
ld(hi("Quarter-squares lookup table"), Y)  # 5
ld("high-byte action.add")
st([high_byte_action])
ld([a])
jmp(Y, "table entry")  # 9
adda([b])  # 10
cost_of_after_right_shift = 10
cost_after_first_lookup__8bit = (
    cost_after_right_shift
    + cost_of_after_right_shift
    + cost_of_high_byte_table_entry
    + cost_of_high_byte_add
    + cost_of_low_byte_table_entry
    + cost_of_low_byte_return
)

label(".after-first-lookup-8bit")
# On return we have the low-byte in the accumulator
# We already either have 0 or 128 in the low byte of the result.
# Adding will cause an overflow into the high byte iff both
# have the MSB set, in which case the result will definitely
# not have the MSB set.
adda([result])  # 1
blt(pc() + 3)  # 2
bra(pc() + 3)  # 3
ld([result], X)  # 4; May be a carry
ld(0, X)  # 4; Definitely no carry
# We can safely add one to the result without causing a further overflow,
# because 255 does not appear in the low-byte table.
# This is part of the subtraction.
adda(1)  # 5
st([result])
# Do the carry
ld([X])
adda([result + 1])
st([result + 1])
ld(".after-second-lookup")  # 10
st([continuation])
ld("high-byte action.invert-and-add")
st([high_byte_action])
ld([a])
jmp(Y, "table entry.possibly-negative")  # 15
suba([b])  # 16
cost_between_lookups__8bit = 16

cost_after_second_lookup__8bit = (
    cost_after_first_lookup__8bit
    + cost_between_lookups__8bit
    + cost_of_absolute
    + cost_of_high_byte_table_entry
    + cost_of_high_byte_invert_and_add
    + cost_of_low_byte_table_entry
    + cost_of_low_byte_return
)

cost_of_8bit_multiply = cost_after_second_lookup__8bit + cost_of_final_add
no_msb_cost_saving = cost_of_8bit_multiply - cost_of_8bit_multiply__both_msbs_low
C(f"Worst case cost: {cost_of_8bit_multiply}")
C(f"Saving when both values < 128: {no_msb_cost_saving}")
C(f"Saving when only one value < 128: {one_msb_cost_saving}")
end()

if __name__ == "__main__":
    disableListing()
    writeRomFiles(sys.argv[0])
