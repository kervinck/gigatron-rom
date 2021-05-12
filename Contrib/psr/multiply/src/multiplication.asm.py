import math
import sys

from asm import (
    AC,
    C,
    Y,
    align,
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
    suba,
    writeRomFiles,
)

if __name__ == "__main__":
    enableListing()

# The following code implements a lookup table of floored quarter squares,
# for values up to 255.
# This is supposed to enable a fast multiplication for 7-bit numbers.


align(0x100, size=0x100)
# First the high-bytes.
# The table is shifted down by 32 places, as the first 32 high-bytes are all zero
# This allows us to have code later in the page which we can branch back to.
align(0x100, size=0x100)
for i in range(32, 256):
    val = math.floor(i ** 2 / 4)
    ld(hi(val))
    C(f"${val:04x} = {val} = floor({i} ** 2 / 4); ${val:04x} >> 8 = ${val >> 8:02x}")

# We jump back here after looking up the low-byte of the result.
label("low-byte return point")
nop()

label("high-byte table entry")
# Calculate an index into the high-byte table.
# This is basically a matter of subtracting 32, and jumping in if the result >= 0.
# But values greater than 160 have the sign-bit set after subtraction,
# despite being >32.
# We test for the sign bit and jump after subtraction even if 'negative' in these cases.
blt(".value has high bit set")  # 1
suba(32)  # 2
bge(AC)  # 3
bra("high-byte return point")  # 4
ld(0)  # 5
label(".value has high bit set")
bra(AC)  # 3
bra("high-byte return point")  # 4

# We jump back here after looking up the high-byte of the result.
label("high-byte return point")

fillers(until=255)
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
bne(AC)
align(0x100, size=0x100)
jmp(Y, "low-byte return point")
ld(0)
C("0 = floor(0 ** 2 / 4) and floor(1 ** 2 / 4)")
for i in range(2, 256):
    val = math.floor(i ** 2 / 4)
    ld(val)
    C(f"${val:04x} = {val} = floor({i} ** 2 / 4)")
align(0x100, size=0x100)


end()

if __name__ == "__main__":
    disableListing()
    writeRomFiles(sys.argv[0])
