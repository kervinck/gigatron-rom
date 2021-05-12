import math
import sys

from asm import (
    AC,
    C,
    Y,
    align,
    bne,
    disableListing,
    enableListing,
    end,
    fillers,
    jmp,
    label,
    ld,
    writeRomFiles,
)

if __name__ == "__main__":
    enableListing()
# The following code implements a lookup table of floored quarter squares,
# for values up to 255.
# This is supposed to enable a fast multiplication for 7-bit numbers.
align(0x100, size=0x100)
# We jump back here after looking up the low-byte of the result.
label("low-byte return point")
fillers(until=255)
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
label("low-byte table entry")
bne(AC)
align(0x100, size=0x100)
jmp(Y, "low-byte return point")
ld(0)
C("0 = floor(0 ** 2 / 4) and floor(1 ** 2 / 4)")
for i in range(2, 256):
    val = math.floor(i ** 2 / 4)
    ld(val)
    C(f"${val:04x} = {val} = floor({i} ** 2 / 4)")
end()

if __name__ == "__main__":
    disableListing()
    writeRomFiles(sys.argv[0])
