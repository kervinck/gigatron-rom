from vasm import *
from vcpu import VirtualCpu
import io

# vCPU register locations in memory
vPC = ADDR("vPC")
vAC = ADDR("vAC")
vLR = ADDR("vLR")
vSP = ADDR("vSP")

# SYS functions
sysFn = ADDR("sysFn")
sysArgs0 = ADDR("sysArgs0")
sysArgs1 = ADDR("sysArgs1")
sysArgs2 = ADDR("sysArgs2")
sysArgs3 = ADDR("sysArgs3")
sysArgs4 = ADDR("sysArgs4")
sysArgs5 = ADDR("sysArgs5")
sysArgs6 = ADDR("sysArgs6")
sysArgs7 = ADDR("sysArgs7")

SYS_VDrawBits_134 = ADDR("SYS_VDrawBits_134")
SYS_Random_34 = ADDR("SYS_Random_34")

# Extra variables in memory
memSize = ADDR("memSize")
romType = ADDR("romType")
romTypeValue_ROMv3 = ADDR("romTypeValue_ROMv2")
videoTable = ADDR("videoTable")
serialRaw = ADDR("serialRaw")
buttonState = ADDR("buttonState")
frameCount = ADDR("frameCount")

# Some constants
F_IMMD            = 0x80   # Immediate flag
F_COMP            = 0x40   # Compile only flag
F_HIDD            = 0x20   # Hidden flag
F_MASK            = 0x1F   # Mask for the length of the word
rsp_top           = 0x05FE # Top of the return stack
dsp_top           = 0x06FE # Top of the data stack
buf_base          = 0x0200 # Buffer for READ
auxbuf_end        = 0x0320 # Buffer for other functions
segment_size      = 96
buf_size          = 0xF0
init1_base        = 0x0200
init2_base        = 0x0300
code_base         = 0x0800
data_base         = None   # Automatically resolved

welcome_string = "GtForth v0.1\n"
free_bytes_string = "bytes free\n"

# User variables start from $30 in zero page
zp_base_addr = 0x30
variable_base = zp_base_addr

def zpSkip(nbytes=2):
    global zp_base_addr
    addr = zp_base_addr
    zp_base_addr += nbytes
    return addr

buf            = zpSkip()  # Current buffer position
inp            = zpSkip()  # The current input position
wlen           = zpSkip()  # The length of the current word
wbuf           = zpSkip()  # The current word
w              = zpSkip()  # Work register
adj1           = zpSkip()  # Address adjust register 1
tmp1           = zpSkip()  # Temporary 1
tmp2           = zpSkip()  # Temporary 2
tmp3           = zpSkip()  # Temporary 3
tmp4           = zpSkip()  # Temporary 4
tmp5           = zpSkip()  # Temporary 5
md1            = zpSkip()  # Temporary 1 for multiplication / division
md2            = zpSkip()  # Temporary 2 for multiplication / division
md3            = zpSkip()  # Temporary 3 for multiplication / division
str1           = zpSkip()  # Temporary 1 for find
str2           = zpSkip()  # Temporary 2 for find
rsp            = zpSkip()  # Return stack pointer
dsp            = zpSkip()  # Data stack pointer
pc             = zpSkip()  # Program counter (points to the current word)
here           = zpSkip()  # The value of the memory
latest         = zpSkip()  # The latest defined word
base           = zpSkip()  # The base for number parsing / printing
state          = zpSkip()  # The state of the compiler
scr_pos        = zpSkip()  # The position on the screen
scr_color      = zpSkip()  # The current color

assert(zp_base_addr < 0x80)

# Addresses of auxiliary functions
zp_base_addr = 0x82
auxiliary_base = zp_base_addr
auxiliary_functions = []

def defAux(name):
    globals()["%s_fn" % name] = zpSkip()
    auxiliary_functions.append(name)

defAux("next_word")      # The function to compute the next word
defAux("exec")           # To jump to some other execution point
defAux("adjust_addr")    # To adjust a general address (for packing)
defAux("forward")        # Move forward the address by 2 bytes (and adjust)
defAux("cforward")       # Move forward the address by 1 byte (and adjust)
defAux("align_addr")     # To align a general address
defAux("push_data")      # To push value on the data stack
defAux("pop_data")       # To pop from the data stack
defAux("push_return")    # To push value on the return stack
defAux("pop_return")     # To pop from the return stack
defAux("multiply")       # To multiply two numbers
defAux("udivide")        # To divide two (unsigned) numbers
defAux("divide")         # To divide two (signed) numbers
defAux("divide_aux")     # Auriliary function to do division
defAux("unsigned_cmp")   # Auxiliary function to compare unsigned numbers
defAux("signed_cmp")     # Auxiliary function to compare signed numbers
defAux("setup_draw")     # To setup for the SYS_VDrawBits_134
defAux("emit")           # To print a character onto the screen
defAux("select_font")    # To select the character font
defAux("draw_char")      # To draw the current character
defAux("update_pos")     # To update the current position
defAux("emit_scroll")    # To scroll based on the last emit
defAux("clearline")      # To clear a line
defAux("scroll")         # To scroll the screen
defAux("read")           # To read characters from keyboard
defAux("show_cursor")    # To show the cursor
defAux("process_key")    # To process an input key
defAux("process_bs")     # To process the backspace key
defAux("parse_word")     # To parse a word
defAux("number")         # To parse a number
defAux("unumber")        # To parse a unsigned number
defAux("digit")          # To parse a single digit
defAux("memcpy")         # To copy memory from one place to another
defAux("streq")          # To compare two strings
defAux("find")           # To find a word in the dictionary
defAux("match")          # Auxiliary function to find the word
defAux("flags")          # To obtain the flags of a word in the dictionary
defAux("cfa")            # To obtain the code of a word in the dictionary
defAux("comma")          # To compile a word
defAux("str_comma")      # To compile a string
defAux("cv_number")      # To convert a number to string
defAux("cv_unumber")     # To convert an unsigned number to string
defAux("cv_digit")       # To convert a digit to string

assert(zp_base_addr < 0xE0)

current_link = [ 0 ]    # Current link for latest (at assembly time)

# ============================
# Auxiliary meta functions
# ============================

def get_link():
    """Get the link to the latest defined word.

    Returns
    -------
        The link to the latest word (to be resolved later).
    """
    fn = lambda x : ADDR(x[0])
    return (fn, current_link)

def reference(label):
    """Places a reference to a label in the resulting GT1 file.

    Parameters
    ----------
    label: str
       The label.
    """
    # Use little-endian format here
    BYTE((LO, label))
    BYTE((HI, label))

    return 0 # Bogus return value (for unary minus)

def place_string(s, include_length=True):
    """Places a string aligned to 2 bytes.

    Parameters
    ----------
    s: str
        The string to place.
    include_length: bool
        To also place the length before the string bytes.
    """
    if include_length:
        BYTE(len(s))

    for c in s:
        BYTE(ord(c))
    ALIGN(2)

    return 0 # Bogus return value (for unary minus)

def define_word(name, flags, label=None, xt="xt_DOCOL"):
    """Defines a new word.

    Parameters
    ----------
    name: str
        The name of the word.
    flags: int
        The flags used to create the word.
    label: str
        The label used for the assembler.
    xt: str
        The location of the code to execute this word.
    """
    if label is None:
        label = name
    header_label = "word_" + label
    L(header_label)
    reference(current_link[0])
    current_link[0] = header_label

    BYTE(flags + len(name))
    place_string(name, include_length=False)
    L(label)
    reference(xt)

def define_constant(name, value, label=None):
    """Defines a new constant word.

    Parameters
    ----------
    name: str
        The name of the word.
    value: int
        The constant value.
    label: str
        The label used for the assembler.
    """
    define_word(name, 0, label=label, xt="xt_DOCON")
    reference(value)

def resolve_font_address(c):
    """Resolves the font address.

    Parameters
    ----------
    c: byte
       The character to resolve.

    Returns
    -------
       Tha address of the font for the character.
    """
    if ord(c) < 82:
        return ADDR("font32up") + 5 * (ord(c) - 32)
    else:
        return ADDR("font82up") + 5 * (ord(c) - 82)

def extend_callback(gt1, ins):
    """To extend the ORG segments page by page.

    Parameters
    ----------
    gt1: list
        The list of segments of the GT1 file.
    ins: tuple
        The tuple of bytes to append.
    """
    segment = gt1[-1]
    base = segment[0]
    size = segment[1]
    contents = segment[3]
    if len(contents) + len(ins) > size or len(contents) >= size:
        new_segment = (base + 256, size, {}, [])
        gt1.append(new_segment)

def pack_segments_factory(base, size):
    """To resolve and pack multiple ORG segments.

    Parameters
    ----------
    base: int
        The base address of the segments.
    size: int
        The size of each segment (bin).

    Returns
    -------
        A callback to pack the segments (to be used by END()).
    """
    def do_packing(gt1, symbols):
        cur_base = base
        orig_gt1 = list(gt1)
        used_pages = set()

        gt1.clear()
        new_segments = [ ]
        for segment in orig_gt1:
            seg_base = segment[0]
            if seg_base is None:
                continue

            gt1.append(segment)
            used_pages.add(seg_base // 256)

        for segment in orig_gt1:
            if segment[0] is not None:
                continue

            seg_size = len(segment[3])
            if seg_size > size:
                raise RuntimeError("segment too large: %d bytes" %\
                                   seg_size)

            selected = None
            for s in new_segments:
                if s[1] - len(s[3]) >= seg_size:
                    selected = s
                    break

            if selected is None:
                cur_page = cur_base // 256
                if cur_page in used_pages:
                    raise RuntimeError("page overlap at 0x%04X" % cur_base)

                new_segments.append([cur_base, size, {}, []])
                selected = new_segments[-1]
                cur_base += 256

            s = selected
            d = len(s[3])

            # Update the labels within this segment
            labels_for_id = []
            for lbl, offset in segment[2].items():
                if not lbl.startswith("."):
                    labels_for_id.append(lbl)
                s[2][lbl] = d + offset

            if seg_size > 55:
                print("Section %r is large (%d bytes, resolved to 0x%04X)" %\
                      (labels_for_id, seg_size, s[0] + d))
            s[3].extend(segment[3])

        total_bytes = 0
        for s in new_segments:
            total_bytes += len(s[3])
            gt1.append(s)

        fraction = float(total_bytes) / (len(new_segments) * size)
        print("Packed %d bytes into %d segments of %d bytes each (%.1f%%)" % \
              (total_bytes, len(new_segments), size, 100 * fraction))

        # Update the symbols
        for segment in gt1:
            for lbl, offset in segment[2].items():
                symbols[lbl] = segment[0] + offset

        segment = gt1[-1]
        return segment[0]

    return do_packing

# ============================
# Initialization
# ============================

# Note: Code in this segment will be destroyed after initialization
# This code occupies the same region of memory for the data and return
# stacks.
ORG(init1_base, size=variable_base)
L("start")
-   LDWI("main")
-   STW(tmp1)
-   CALL(tmp1)

# Data initialization
ORG(init1_base + rsp)
L("init_data")
-   reference(rsp_top)      # rsp
-   reference(dsp_top)      # dsp
-   reference("cold_start") # pc
-   reference("here_loc")   # here
-   reference(get_link())   # latest
-   reference(10)           # base
-   reference(0)            # state
-   reference(0x7662)       # scr_pos
-   reference(0x0B00)       # scr_color

ORG(init1_base + 0x80)
BYTE(1)

ORG(init1_base + auxiliary_base)
for name in auxiliary_functions:
    reference(name)
L("init_data_end")

ORG(init2_base)
L("main")

# Shift the video to not display the code
-   LDWI(videoTable+1)
-   STW(tmp1)
-   LDI(96)
-   POKE(tmp1)

# Copy initialization data
-   LDWI("init_data")
-   STW(tmp1)
-   ANDI(0xFF)
-   STW(tmp3)
-   LDWI("init_data_end")
-   STW(tmp2)

L(".copy_init_loop")
-       LDW(tmp1)
-       PEEK()
-       POKE(tmp3)
-       INC(tmp1)
-       INC(tmp3)
-       LDW(tmp1)
-       SUBW(tmp2)
-       BNE(".copy_init_loop")

# Start the interpreter
-   CALL(next_word_fn)

# ============================
# Auxiliary functions
# ============================

# Processes the next forth word.
ORG(None)
L("next_word")
-   LDW(pc)
-   DEEK()
-   STW(w)

# Increment the pc by 2 and adjust address.
-   LDI(pc)
-   CALL(forward_fn)

-   LDW(w)

# Starts execution at a given address (in vAC)
L("exec")
-   STW(w)
-   DEEK()
-   STW(vLR)
-   RET()

# Adjusts the address to be within the valid region of RAM.
# The address of the memory location containing the address
# to adjust is given in vAC.
# Note: vAC must always be even!
ORG(None)
L("adjust_addr")
-   STW(adj1)
-   ADDI(1)
-   PEEK()
-   SUBI(8)
-   BLT(".adjust_skip")
-   LDW(adj1)
-   DEEK()
-   BLT(".adjust_skip")
-   ANDI(0xFF)
-   SUBI(segment_size & 0xFF)
-   BLT(".adjust_skip")
-       POKE(adj1)
-       INC(adj1) # Can increment here because adj1 is always even
-       LDW(adj1)
-       PEEK()
-       ADDI(1)
-       POKE(adj1)
L(".adjust_skip")
-   RET()

# Moves forward by two bytes and adjust the address.
# The input of this function is given in vAC (same as for
# adjust_addr).
ORG(None)
L("forward")
-   PUSH()
-   STW(adj1)
-   DEEK()
-   ADDI(2)
L(".forward_writeback")
-   DOKE(adj1)
-   LDW(adj1)
-   CALL(adjust_addr_fn)
-   POP()
-   RET()

# Moves forward by one byte and adjust the address.
# The input of this function is given in vAC (same as for
# adjust_addr).
L("cforward")
-   PUSH()
-   STW(adj1)
-   DEEK()
-   ADDI(1)
-   BRA(".forward_writeback")

# Alignes an address to a multiple of two and
# then adjusts the address.
ORG(None)
L("align_addr")
-   PUSH()
-   STW(adj1)
-   DEEK()
-   ANDI(1)
-   BEQ(".align_skip")
-       LDW(adj1)
-       DEEK()
-       ADDI(1)
-       DOKE(adj1)
L(".align_skip")
-   LDW(adj1)
-   CALL(adjust_addr_fn)
-   POP()
-   RET()

# Pushes a value on the data stack.
ORG(None)
L("push_data")
-   DOKE(dsp)
-   LD(dsp)
-   SUBI(2)
-   ST(dsp)
-   RET()

# Pops a value from the data stack.
ORG(None)
L("pop_data")
-   INC(dsp) # Can increment because dsp stays in the same page
-   INC(dsp)
-   LDW(dsp)
-   DEEK()
-   RET()

# Pushes a value on the return stack.
ORG(None)
L("push_return")
-   DOKE(rsp)
-   LD(rsp)
-   SUBI(2)
-   ST(rsp)
-   RET()

# Pops a value from the return stack.
ORG(None)
L("pop_return")
-   INC(rsp)
-   INC(rsp)
-   LDW(rsp)
-   DEEK()
-   RET()

# To multiply two numbers.
# The inputs are given in md1 and md2.
# The result is in md3.
# Uses tmp5 as temporary storage.
ORG(None)
L("multiply")
-   LDI(0)
-   STW(md3)
-   LDI(1)
L(".multiply_loop")
-       STW(tmp5)
-       ANDW(md2)
-       BEQ(".multiply_continue")
-       LDW(md1)
-       ADDW(md3)
-       STW(md3)
L(".multiply_continue")
-       LDW(md1)
-       LSLW()
-       STW(md1)
-       LDW(tmp5)
-       LSLW()
-       BNE(".multiply_loop")
-   RET()

# To divide two (unsigned) numbers.
# The inputs are given in md1 and md2 (dividend and divisor).
# The results are in md3 (quotient) and md1 (remainder).
# Note: if the divisor is zero, the quotient will be md1
# and the remainder will also be md1.
# Uses tmp5 as temporary storage.
ORG(None)
L("udivide")
-   PUSH()
-   LDW(md1)
-   STW(md3)
-   LDW(md2)
-   BEQ(".udivide_end")
-   LDI(0)
-   STW(md1)
L(".udivide_loop")
-   STW(tmp5)
-   LDW(md1)
-   LSLW()
-   STW(md1)
-   LDW(md3)
-   BGE(".udivide_no_increment1")
-   INC(md1)
L(".udivide_no_increment1")
-   LSLW()
-   STW(md3)
-   CALL(unsigned_cmp_fn)
-   BLT(".udivide_no_increment2")
-   LDW(md1)
-   SUBW(md2)
-   STW(md1)
-   INC(md3)
L(".udivide_no_increment2")
-   LDW(tmp5)
-   ADDI(1)
-   ANDI(15)
-   BNE(".udivide_loop")
L(".udivide_end")
-   POP()
-   RET()

# To divide two (signed) numbers.
# The inputs are given in md1 and md2 (dividend and divisor).
# The results are in md3 (quotient) and md1 (remainder).
# Note: if the divisor is zero, the quotient will be md1
# and the remainder will also be md1.
# Uses tmp4 and tmp5 as temporary storage.
ORG(None)
L("divide")
-   PUSH()
-   LDI(0)
-   SUBW(md1)
-   BGT(".divide_neg1")
-   CALL(divide_aux_fn)
-   BRA(".divide_end")
L(".divide_neg1")
-   STW(md1)
-   CALL(divide_aux_fn)
-   LDI(0)
-   SUBW(md3)
-   STW(md3)
-   LDW(md1)
-   BEQ(".divide_end")
-   LDW(md2)
-   SUBW(md1)
-   STW(md1)
-   LDW(tmp4)
-   BLT(".divide_neg2")
-   LDWI(-1)
-   BRA(".divide_continue")
L(".divide_neg2")
-   LDI(1)
L(".divide_continue")
-   ADDW(md3)
-   STW(md3)
L(".divide_end")
-   POP()
-   RET()

# Auxiliary function divide two (signed) numbers.
# The inputs are given in md1 and md2 (dividend and divisor).
# The results are in md3 (quotient) and md1 (remainder).
# Note: This function assumes that md1 is non-negative and
# on the return, md2 is possibly changed in sign so that
# it is positive. In addition tmp4 keeps the original value
# of md2.
# Note: if the divisor is zero, the quotient will be md1
# and the remainder will also be md1.
# Uses tmp5 as temporary storage.
ORG(None)
L("divide_aux")
-   PUSH()
-   LDW(md2)
-   STW(tmp4)
-   LDI(0)
-   SUBW(md2)
-   BLT(".divide_aux_no_adjust")
-   STW(md2)
L(".divide_aux_no_adjust")
-   CALL(udivide_fn)
-   LDW(tmp4)
-   BGE(".divide_aux_end")
-   LDI(0)
-   SUBW(md3)
-   STW(md3)
L(".divide_aux_end")
-   POP()
-   RET()


# Auxiliary function to compare md1 and md2
# as unsigned numbers.
# It returns a negative number if md1 < md2 (unsigned),
# a positive number if md1 > m2 (unsigned) and zero
# if md1 = md2.
ORG(None)
L("unsigned_cmp")
-   LDW(md1)
-   XORW(md2)
-   BLT(".unsigned_cmp_diff")
-   LDW(md1)
-   SUBW(md2)
-   RET()
L(".unsigned_cmp_diff")
-   LDW(md2)
-   ORI(1)
-   RET()

# Auxiliary function to compare md1 and md2
# as signed numbers.
# It returns a negative number if md1 < md2 (signed),
# a positive number if md1 > md2 (signed), and zero
# if md1 = md2.
ORG(None)
L("signed_cmp")
-   LDW(md1)
-   XORW(md2)
-   BLT(".signed_cmp_diff")
-   LDW(md1)
-   SUBW(md2)
-   RET()
L(".signed_cmp_diff")
-   LDW(md1)
-   ORI(1)
-   RET()

# Setup the register for invoking the SYS_VDrawBits_134 function.
ORG(None)
L("setup_draw")
-   LDWI(SYS_VDrawBits_134)
-   STW(sysFn)
-   LDW(scr_color)
-   STW(sysArgs0)
-   RET()

# Prints a character onto the screen.
# The charater is in register vAC.
# It returns True if the character is printable or newline.
# This functions uses tmp3, tmp4, and tmp5 for temporary storage.
ORG(None)
L("emit")
-   PUSH()
-   STW(tmp5)

# Handle newline (line-feed)
-   XORI(ord('\n'))
-   BNE(".emit_no_newline")
# Move position to the new of the line
-   LDI(0xF8)
-   ST(scr_pos)
-   BRA(".emit_update_pos")
L(".emit_no_newline")
-   LDW(tmp5)
-   CALL(select_font_fn)
-   BEQ(".emit_end")
-   LDW(scr_pos)
-   STW(sysArgs4)
-   CALL(draw_char_fn)
L(".emit_update_pos")
-   CALL(update_pos_fn)
-   CALL(emit_scroll_fn)
-   LDI(1)
L(".emit_end")
-   POP()
-   RET()

# Selects the font and returns True if it is a printable character.
# The charater is in vAC.
# The address of the font is stored in tmp5 on return.
# It uses tmp4 and tmp5 for temporary storage.
ORG(None)
L("select_font")
-   SUBI(82)
-   BLT(".select_font_32")
-   STW(tmp5)
-   SUBI(50)
-   BGE(".select_font_end") # Skip unprintable characters
-   LDWI("font82up")
-   BRA(".select_font_continue")
L(".select_font_32")
-   ADDI(50)
-   BLT(".select_font_end")
-   STW(tmp5)
-   LDWI("font32up")
L(".select_font_continue")
-   STW(tmp4)
-   LDW(tmp5)
-   LSLW()
-   LSLW()
-   ADDW(tmp5)
-   ADDW(tmp4)
-   STW(tmp5)
-   LDI(1)
-   RET()
L(".select_font_end")
-   LDI(0)
-   RET()

# Draws a character to screen.
# The address of the character is in tmp5.
# It uses tmp4 and tmp5 for temporary storage.
ORG(None)
L("draw_char")
-   LDI(5)
L(".draw_char_loop")
-       STW(tmp4)
-       LDW(tmp5)
-       LUP(0)
-       ST(sysArgs2)
-       SYS(134) # Call SYS_VDrawBits_134
-       INC(tmp5)
-       INC(sysArgs4)
-       LDW(tmp4)
-       SUBI(1)
-       BGT(".draw_char_loop")
-   RET()

# Updates scr_pos.
ORG(None)
L("update_pos")
-   LDW(scr_pos)
-   ADDI(6)
-   STW(scr_pos)
-   LD(scr_pos)
-   SUBI(0xFE)
-   BLT(".update_pos_end")
-   LDWI(0x0964)
-   ADDW(scr_pos)
-   BGE(".update_pos_continue")
-   LDWI(0x0862)
L(".update_pos_continue")
-   STW(scr_pos)
L(".update_pos_end")
-   RET()

# Checks if there is need to scroll.
# It uses tmp3, tmp4, and tmp5 as temporary storage.
ORG(None)
L("emit_scroll")
-   PUSH()
# Check if started a new line
-   LD(scr_pos)
-   SUBI(0x62)
-   BGT(".emit_scroll_end")
-   LDW(scr_pos)
# Recall that there is a border of 2 pixels in scr_pos
-   SUBI(2)
-   CALL(clearline_fn)
# Check if would wrap around the screen
-   LDWI(videoTable)
-   PEEK()
-   STW(tmp5)
-   LDI(scr_pos + 1)
-   PEEK()
-   XORW(tmp5)
-   BNE(".emit_scroll_end")
# Scroll by 10 lines to avoid the wrap around
-   LDI(10)
-   CALL(scroll_fn)
L(".emit_scroll_end")
-   POP()
-   RET()

# Clears a line (for newline).
# This means that the screen is cleared for the current line.
# It uses tmp3 and tmp5 as temporary storage.
ORG(None)
L("clearline")
-   PUSH()
-   STW(sysArgs4)
-   LDI(32)
L(".clearline_loop")
-       STW(tmp3)
-       LDWI(resolve_font_address(' '))
-       STW(tmp5)
-       CALL(draw_char_fn)
-       LDW(tmp3)
-       SUBI(1)
-       BNE(".clearline_loop")
-   POP()
-   RET()

# Code to implement scrolling.
# It uses tmp3, tmp4, and tmp5 as temporary storage.
ORG(None)
L("scroll")
-   STW(tmp5) # The amount to scroll
-   LDWI(120)
-   STW(tmp3)
-   LDWI(videoTable)
-   STW(tmp4)
L(".scroll_loop")
-       LDW(tmp4)
-       PEEK()
-       ADDW(tmp5)
-       SUBI(8)
-       BLT(".scroll_1")
-       SUBI(120)
-       BGE(".scroll_2")
L(".scroll_1")
-       ADDI(120)
L(".scroll_2")
-       ADDI(8)
-       POKE(tmp4)
-       LDW(tmp4)
-       ADDI(2)
-       STW(tmp4)
-       LDW(tmp3)
-       SUBI(1)
-       STW(tmp3)
-       BNE(".scroll_loop")
-   RET()

# Code to implement reading from the keyboard.
# Uses tmp3, tmp4, and tmp5 as temporary storage.
ORG(None)
L("read")
-   PUSH()

L(".read_loop1")
-       LD(serialRaw)
-       STW(tmp3)
-       LD(frameCount)   # Use the frameCount to implement the cursor
-       STW(tmp4)

L(".read_loop2")
-           LD(serialRaw)
-           XORW(tmp3)
-           BNE(".read_key")
-           LD(frameCount)
-           XORW(tmp4)
-           BEQ(".read_loop2")

# Show the cursor
-           LD(frameCount)
-           ANDI(8)
-           CALL(show_cursor_fn)
-           LD(frameCount)
-           STW(tmp4)
-           BRA(".read_loop2")

L(".read_key")
-       LD(serialRaw)
-       CALL(process_key_fn)
-       BNE(".read_loop1")
-   POP()
-   RET()

# Shows the blinking cursor.
# If vAC is zero, shows a space, otherwise shows a full square.
# Uses tmp4 and tmp5 as temporary storage.
ORG(None)
L("show_cursor")
-   PUSH()
-   BEQ(".show_cursor_space")
-   LDWI(resolve_font_address(chr(127)))
-   BRA(".show_cursor_continue")
L(".show_cursor_space")
-   LDWI(resolve_font_address(' '))
L(".show_cursor_continue")
-   STW(tmp5)
-   LDW(scr_pos)
-   STW(sysArgs4)
-   CALL(draw_char_fn)
-   POP()
-   RET()

# Processes a key input from the keyboard.
# Uses tmp3, tmp4, and tmp5 as temporary storage.
ORG(None)
L("process_key")
-   PUSH()
-   STW(tmp5)

-   LDI(ord('\n'))    # Check for newlines
-   XORW(tmp5)
-   BNE(".process_key_continue")
-   LDI(0)
-   CALL(show_cursor_fn)
-   LDI(0)
-   BRA(".process_key_end")

L(".process_key_continue")
-   LDI(127)   # Now check for backspaces
-   XORW(tmp5)
-   BNE(".process_key_regular")
-   CALL(process_bs_fn)
-   BRA(".process_key_return_true")

L(".process_key_regular")
-   LD(buf)
-   SUBI(buf_size)   # Check if the buffer is full
-   BEQ(".process_key_return_true")

-   LDW(tmp5)
-   POKE(buf)
-   CALL(emit_fn)
-   BEQ(".process_key_return_true")
-   INC(buf)
L(".process_key_return_true")
-   LDI(1)
L(".process_key_end")
-   POP()
-   RET()

# Processes the backspace key.
# Uses tmp4 and tmp5 as temporary storage.
ORG(None)
L("process_bs")
-   PUSH()
-   LD(scr_pos)
-   SUBI(0x62)
-   BLE(".process_bs_end")
-   LD(buf)
-   BEQ(".process_bs_end")
-   LDI(0)
-   CALL(show_cursor_fn)
-   LDW(scr_pos)
-   SUBI(6)
-   STW(scr_pos)
-   LDI(0)
-   CALL(show_cursor_fn)
-   LD(buf)
-   SUBI(1)
-   ST(buf)
L(".process_bs_end")
-   POP()
-   RET()

# Processes a word.
# Input: vAC -> the character delimiter
# Uses tmp5 as temporary storage.
ORG(None)
L("parse_word")
-   PUSH()
-   STW(tmp5)
-   LDI(0)
-   STW(wlen)
L(".parse_word_loop1")
-   LDW(inp)
-   XORW(buf)
-   BEQ(".parse_word_end")
-   LDW(inp)
-   INC(inp)
-   STW(wbuf)
-   PEEK()
-   XORW(tmp5)
-   BEQ(".parse_word_loop1")
-   INC(wlen)
L(".parse_word_loop2")
-   LDW(inp)
-   XORW(buf)
-   BEQ(".parse_word_end")
-   LDW(inp)
-   PEEK()
-   XORW(tmp5)
-   INC(inp)
-   BEQ(".parse_word_end")
-   INC(wlen)
-   BRA(".parse_word_loop2")
L(".parse_word_end")
-   POP()
-   RET()

# To parse a number.
# Input: str1 is the length of the string, and
# str2 is the pointer to the string.
# Output: tmp1 is the parsed number, and vAC
# indicates whether or not the parsing succeeded
# (non-positive number means error).
# This function uses tmp2, tmp3, tmp4 and tmp5 as
# temporary storage.
ORG(None)
L("number")
-   PUSH()
-   LDW(str2)
-   STW(tmp3)
-   LDW(str1)
-   STW(tmp2)
-   BEQ(".number_end")
-   LDW(tmp3)
-   PEEK()
-   XORI(ord('-'))
-   BEQ(".number_negative")
-   CALL(unumber_fn)
L(".number_end")
-   POP()
-   RET()

L(".number_negative")
-   LDW(tmp2)
-   SUBI(1)
-   STW(tmp2)
-   LDI(tmp3)
-   CALL(cforward_fn)
-   CALL(unumber_fn)
-   STW(tmp2)
-   LDI(0)
-   SUBW(tmp1)
-   STW(tmp1)
-   LDW(tmp2)
-   BRA(".number_end")


# To parse an unsigned number.
# Input: tmp2 is the length of the string, and
# tmp3 is the pointer to the string.
# Output: tmp1 is the parsed number, and vAC
# indicates whether or not the parsing succeeded
# (non-positive number means error).
# This function uses tmp4 and tmp5 as temporary storage.
ORG(None)
L("unumber")
-   PUSH()
-   LDI(0)
-   STW(tmp1)
-   LDW(base)
-   STW(md2)
-   LDW(tmp2)
-   BEQ(".unumber_end")
L(".unumber_loop")
-       LDW(tmp3)
-       PEEK()
-       STW(tmp4)
-       LDI(tmp3)
-       CALL(cforward_fn)
-       CALL(digit_fn)
-       BLE(".unumber_end")
-       LDW(tmp1)
-       STW(md1)
-       CALL(multiply_fn)
-       LDW(md3)
-       ADDW(tmp4)
-       STW(tmp1)
-       LDW(tmp2)
-       SUBI(1)
-       STW(tmp2)
-       BNE(".unumber_loop")
-   LDI(1)
L(".unumber_end")
-   POP()
-   RET()

# To parse a single digit.
# The input digit is in tmp4 and the output is returned in tmp4 as well.
# On success this function should return a positive number in vAC.
ORG(None)
L("digit")
-   LDW(tmp4)
-   SUBI(ord('0'))
-   BLT(".digit_error")
-   SUBI(ord('9') - ord('0'))
-   BGT(".digit_letter")
-   ADDI(ord('9') - ord('0'))
-   STW(tmp4)
-   BRA(".digit_check")

L(".digit_letter")
-   LDW(tmp4)
-   SUBI(ord('A'))
-   BLT(".digit_error")
-   LDI(ord('Z'))
-   SUBW(tmp4)
-   BLT(".digit_error")
-   LDW(tmp4)
-   ADDI(10 - ord('A'))
-   STW(tmp4)

L(".digit_check")
-   LDW(base)
-   SUBW(tmp4)
L(".digit_error")
-   RET()

# To copy one memory region to another.
# Input: tmp1 -> number of bytes to copy;
#        tmp2 -> source address;
#        tmp3 -> destination address;
ORG(None)
L("memcpy")
-   PUSH()
L(".memcpy_loop")
-       LDW(tmp1)
-       BEQ(".memcpy_end")
-       SUBI(1)
-       STW(tmp1)
-       LDW(tmp2)
-       PEEK()
-       POKE(tmp3)
-       LDI(tmp2)
-       CALL(cforward_fn)
-       LDI(tmp3)
-       CALL(cforward_fn)
-       BRA(".memcpy_loop")
L(".memcpy_end")
-   POP()
-   RET()

# To compare two strings.
# Input: tmp1 -> length of the first string;
#        tmp2 -> address of the first string;
#        tmp3 -> length of the second string;
#        tmp4 -> address of the second string;
# Return: Zero if the strings are equal, non-zero otherwise.
ORG(None)
L("streq")
-   PUSH()
-   LDW(tmp1)
-   SUBW(tmp3)
-   BNE(".streq_end")
L(".streq_loop")
-       LDW(tmp1)
-       BEQ(".streq_end")
-       SUBI(1)
-       STW(tmp1)
-       LDW(tmp2)
-       PEEK()
-       STW(tmp3)
-       LDW(tmp4)
-       PEEK()
-       SUBW(tmp3)
-       BNE(".streq_end")
-       LDI(tmp2)
-       CALL(cforward_fn)
-       LDI(tmp4)
-       CALL(cforward_fn)
-       BRA(".streq_loop")
L(".streq_end")
-   POP()
-   RET()

# To find one word in the dictionary.
# Input: str1 -> length of the word to search;
#        str2 -> address of the string to search;
# Output: vAC contains the address of the word in the dictionary.
# This function uses tmp1, tmp2, tmp3, tmp4, and tmp5
# as temporary storage.
ORG(None)
L("find")
-   PUSH()
-   LDW(latest)
L(".find_loop")
-       STW(tmp5)
-       BEQ(".find_end")
-       CALL(match_fn)
-       BEQ(".find_end")
-       LDW(tmp5)
-       DEEK()
-       BRA(".find_loop")
L(".find_end")
-   LDW(tmp5)
-   POP()
-   RET()

# Auxiliary function to find one word in the dictionary.
# Input: str1 -> length of the word to search;
#        str2 -> address of the string to search;
#        vAC  -> address of the current word;
# Output: vAC is zero if the word was found.
# This function uses tmp1, tmp2, tmp3 and tmp4 as temporary storage.
ORG(None)
L("match")
-   PUSH()
-   STW(tmp4)
-   LDI(tmp4)
-   CALL(forward_fn)
-   LDW(tmp4)
-   PEEK()
-   ANDI(F_HIDD)
-   BNE(".match_end")
-   LDW(tmp4)
-   INC(tmp4) # There is no need to adjust as tmp4 is even
-   PEEK()
-   ANDI(F_MASK)
-   STW(tmp3)
-   LDW(str1)
-   STW(tmp1)
-   LDW(str2)
-   STW(tmp2)
-   CALL(streq_fn)
L(".match_end")
-   POP()
-   RET()

# Auxiliary function to extract the flags of a word in dictionary.
# Input: tmp1 -> address of the word in dictionary
# Output: vAC -> the flags (including the length)
#         tmp1 -> the address of the flags byte.
ORG(None)
L("flags")
-   PUSH()
-   LDI(tmp1)
-   CALL(forward_fn)
-   LDW(tmp1)
-   PEEK()
-   POP()
-   RET()

# Auxiliary function to extract the flags of a word in dictionary.
# Input: tmp1 -> address of the flags byte of the word in dictionary.
# Output: tmp1 -> the result;
#         tmp2 -> the length of the name;
#         tmp3 -> the pointer to the name string;
# Note: one should call "flags" before calling this function.
ORG(None)
L("cfa")
-   PUSH()
-   LDW(tmp1)
-   INC(tmp1)    # tmp1 is even here
-   PEEK()
-   ANDI(F_MASK)
-   STW(tmp2)
-   LDW(tmp1)
-   STW(tmp3)
-   ADDW(tmp2)
-   STW(tmp1)
-   LDI(tmp1)
-   CALL(align_addr_fn)
-   POP()
-   RET()

# Compiles one word.
# Input: vAC -> word to compile.
ORG(None)
L("comma")
-   PUSH()
-   DOKE(here)
-   LDI(here)
-   CALL(forward_fn)
-   POP()
-   RET()

# Compiles one string.
# Input: tmp1 -> length of string;
#        tmp2 -> address of string;
# Uses tmp3 as temporary variable.
ORG(None)
L("str_comma")
-   PUSH()
-   LDW(tmp1)
-   POKE(here)
-   INC(here)
-   LDW(here)
-   STW(tmp3)
-   CALL(memcpy_fn)
-   LDW(tmp3)
-   STW(here)
-   LDI(here)
-   CALL(align_addr_fn)
-   POP()
-   RET()

# Converts one number to string.
# Input: tmp1 -> number to be converted;
#        tmp2 -> address of the buffer to write the number;
ORG(None)
L("cv_number")
-   PUSH()
-   LDI(0)
-   SUBW(tmp1)
-   BGT(".cv_number_negative")
-   CALL(cv_unumber_fn)
-   POP()
-   RET()
L(".cv_number_negative")
-   STW(tmp1)
-   CALL(cv_unumber_fn)
-   LDI(ord('-'))
-   POKE(tmp2)
-   LDW(tmp2)
-   SUBI(1)
-   STW(tmp2)
-   POP()
-   RET()

# Converts one unsigned number to string.
# Input: tmp1 -> number to be converted;
#        tmp2 -> address of the buffer to write the number;
# Uses tmp5 as temporary storage;
ORG(None)
L("cv_unumber")
-   PUSH()
-   LDW(base)
-   STW(md2)
-   LDW(tmp1)
-   STW(md3)
-   BEQ(".cv_unumber_write")
L(".cv_unumber_loop")
-   STW(md1)
-   CALL(udivide_fn)
-   LDW(md1)
L(".cv_unumber_write")
-   CALL(cv_digit_fn)
-   POKE(tmp2)
-   LDW(tmp2)
-   SUBI(1)
-   STW(tmp2)
-   LDW(md3)
-   BNE(".cv_unumber_loop")
-   POP()
-   RET()

# Converts one digit to string.
# Input: vAC -> digit to be convert;
# Output: vAC -> the ascii code of the digit;
ORG(None)
L("cv_digit")
-   SUBI(10)
-   BGE(".cv_digit_letter")
-   ADDI(ord('0') + 10)
-   RET()
L(".cv_digit_letter")
-   ADDI(ord('A'))
-   RET()

# ============================
# Code of the words
# ============================

# Preamble of colon definitions.
# ( -- )
ORG(None)
L("xt_DOCOL")
-   LDW(pc)
-   CALL(push_return_fn)
-   LDW(w)     # Work register contains the pointer to the current word.
-   STW(pc)
-   LDI(pc)
-   CALL(forward_fn)
-   CALL(next_word_fn)

# Code used to implement constants.
# ( -- n )
ORG(None)
L("xt_DOCON")
-   LDI(w)
-   CALL(forward_fn)
-   LDW(w)
-   DEEK()     # Get the literal value
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Code for adjusting addresses.
# ( a -- a' )
ORG(None)
L("xt_ADJ")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   LDI(tmp1)
-   CALL(adjust_addr_fn)
-   LDW(tmp1)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Code for aligning addresses.
# ( a -- a' )
ORG(None)
L("xt_ALIGN")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   LDI(tmp1)
-   CALL(align_addr_fn)
-   LDW(tmp1)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Code for executing an instruction on the data stack.
# ( w -- )
ORG(None)
L("xt_EXECUTE")
-   CALL(pop_data_fn)
-   CALL(exec_fn)

# Code for the BRANCH instruction.
# The next cell contains a relative address of the branch.
# ( -- )
ORG(None)
L("xt_BRANCH")
-   LDW(pc)
-   DEEK()     # Value in the slot after branch is the offset for the PC
-   STW(pc)
-   CALL(next_word_fn)

# Code for the 0BRANCH instruction (used to implement IF word).
# This will execute the branch if the number on top of the data
# stack is zero.
# ( n -- )
L("xt_0BRANCH")
-   CALL(pop_data_fn)
-   BEQ("xt_BRANCH")
-   LDI(pc)
-   CALL(forward_fn)
-   CALL(next_word_fn)

# Code to return from the current word.
# ( -- )
ORG(None)
L("xt_EXIT")
-   CALL(pop_return_fn)
-   STW(pc)    # Pop pc from return stack
-   CALL(next_word_fn)

# Pushes the literal in the next cell to the top of
# the data stack.
# ( -- n )
ORG(None)
L("xt_LIT")
-   LDW(pc)
-   DEEK()     # Get the literal value
-   CALL(push_data_fn)
-   LDI(pc)
-   CALL(forward_fn)
-   CALL(next_word_fn)

# Pushes the number of bytes and the address of the
# string onto the data stack.
# ( -- l a )
ORG(None)
L("xt_LITSTR")
-   LDW(pc)
-   PEEK()     # Get the length
-   STW(tmp1)
-   CALL(push_data_fn)
-   INC(pc)    # Can increment here because pc is even
-   LDW(pc)
-   CALL(push_data_fn)
-   LDI(tmp1)
-   CALL(adjust_addr_fn)
-   LDW(pc)
-   ADDW(tmp1)
-   STW(pc)
-   LDI(pc)
-   CALL(align_addr_fn)
-   CALL(next_word_fn)

# Drops the current value in the data stack.
# ( n -- )
ORG(None)
L("xt_DROP")
-   CALL(pop_data_fn)
-   CALL(next_word_fn)

# Drops the last two values in the data stack.
# ( n2 n1 -- )
ORG(None)
L("xt_2DROP")
-   CALL(pop_data_fn)
-   CALL(pop_data_fn)
-   CALL(next_word_fn)

# Swaps the last two values in the data stack.
# ( n2 n1 -- n1 n2 )
ORG(None)
L("xt_SWAP")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(pop_data_fn)
-   STW(tmp2)
-   LDW(tmp1)
-   CALL(push_data_fn)
-   LDW(tmp2)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Duplicates the value in the (data) stack.
# ( n1 -- n1 n1 )
ORG(None)
L("xt_DUP")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(push_data_fn)
-   LDW(tmp1)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Duplicates the last two values in the (data) stack.
# ( n2 n1 -- n2 n1 n2 n1 )
ORG(None)
L("xt_2DUP")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(pop_data_fn)
-   STW(tmp2)
-   CALL(push_data_fn)
-   LDW(tmp1)
-   CALL(push_data_fn)
-   LDW(tmp2)
-   CALL(push_data_fn)
-   LDW(tmp1)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Duplicates the value in the (data) stack if the
# value is non-zero
# ( n1 -- n1 | n1 n1 )
ORG(None)
L("xt_?DUP")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(push_data_fn)
-   LDW(tmp1)
-   BEQ(".xt_?DUP_end")
-   CALL(push_data_fn)
L(".xt_?DUP_end")
-   CALL(next_word_fn)

# Pushes the 2nd element from the top to the data stack.
# ( n2 n1 -- n2 n1 n2 )
ORG(None)
L("xt_OVER")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(pop_data_fn)
-   STW(tmp2)
-   CALL(push_data_fn)
-   LDW(tmp1)
-   CALL(push_data_fn)
-   LDW(tmp2)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Rotates the last 3 elements of the stack.
# ( n3 n2 n1 -- n2 n1 n3 )
ORG(None)
L("xt_ROT")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(pop_data_fn)
-   STW(tmp2)
-   CALL(pop_data_fn)
-   STW(tmp3)
-   LDW(tmp2)
-   CALL(push_data_fn)
-   LDW(tmp1)
-   CALL(push_data_fn)
-   LDW(tmp3)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Rotates the last 3 elements of the stack in reverse order.
# ( n3 n2 n1 -- n1 n3 n2 )
ORG(None)
L("xt_RROT")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(pop_data_fn)
-   STW(tmp2)
-   CALL(pop_data_fn)
-   STW(tmp3)
-   LDW(tmp1)
-   CALL(push_data_fn)
-   LDW(tmp3)
-   CALL(push_data_fn)
-   LDW(tmp2)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Transfers a value from the return stack to the data stack.
# ( -- r )
ORG(None)
L("xt_R>")
-   CALL(pop_return_fn)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Transfers a value from the return stack to the data stack.
# ( r -- )
ORG(None)
L("xt_>R")
-   CALL(pop_data_fn)
-   CALL(push_return_fn)
-   CALL(next_word_fn)

# Gets the value of the (data) stack pointer (before the push).
# ( -- dsp )
ORG(None)
L("xt_DSP@")
-   LDW(dsp)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Sets the value of the (data) stack pointer.
# ( dsp -- )
ORG(None)
L("xt_DSP!")
-   CALL(pop_data_fn)
-   STW(dsp)
-   CALL(next_word_fn)

# Gets the value of the (return) stack pointer.
# ( -- rsp )
ORG(None)
L("xt_RSP@")
-   LDW(rsp)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Sets the value of the (return) stack pointer.
# ( rsp -- )
ORG(None)
L("xt_RSP!")
-   CALL(pop_data_fn)
-   STW(rsp)
-   CALL(next_word_fn)

# Stores a value into memory.
# ( n a -- )
ORG(None)
L("xt_!")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(pop_data_fn)
-   DOKE(tmp1)
-   CALL(next_word_fn)

# Fetches a value from memory.
# ( a -- n )
ORG(None)
L("xt_@")
-   CALL(pop_data_fn)
-   DEEK()
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Stores a character (byte).
# ( b a -- )
ORG(None)
L("xt_C!")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(pop_data_fn)
-   POKE(tmp1)
-   CALL(next_word_fn)

# Fetches a character.
# ( a -- b )
ORG(None)
L("xt_C@")
-   CALL(pop_data_fn)
-   PEEK()
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Replaces the last two values on the data
# stack by their sum.
# ( n2 n1 -- n1+n2 )
ORG(None)
L("xt_+")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(pop_data_fn)
-   ADDW(tmp1)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Replaces the last two values on the data
# stack by their difference.
# ( n2 n1 -- n2-n1 )
ORG(None)
L("xt_-")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(pop_data_fn)
-   SUBW(tmp1)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Replaces the last two values on the data
# stack by their logical AND.
# ( n2 n1 -- n1&n2 )
ORG(None)
L("xt_AND")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(pop_data_fn)
-   ANDW(tmp1)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Replaces the last two values on the data
# stack by their logical OR.
# ( n2 n1 -- n1|n2 )
ORG(None)
L("xt_OR")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(pop_data_fn)
-   ORW(tmp1)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Replaces the last two values on the data
# stack by their logical exclusive OR.
# ( n2 n1 -- n1^n2 )
ORG(None)
L("xt_XOR")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(pop_data_fn)
-   XORW(tmp1)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Replaces the top of the stack with its logical NOT value.
# ( n1 -- ~n1 )
ORG(None)
L("xt_NOT")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   LDWI(-1)
-   XORW(tmp1)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Replaces the top of the stack with its negative value.
# ( n1 -- ~n1 )
ORG(None)
L("xt_NEG")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   LDI(0)
-   SUBW(tmp1)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Replaces the last two values on the data
# stack by their product.
# ( n2 n1 -- n2*n1 )
ORG(None)
L("xt_*")
-   CALL(pop_data_fn)
-   STW(md1)
-   CALL(pop_data_fn)
-   STW(md2)
-   CALL(multiply_fn)
-   LDW(md3)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Replaces the last two values on the data
# stack by their quotient and remainder.
# The two numbers n1 and n2 are treated as unsigned
# numbers.
# ( n2 n1 -- n2%n1 n2/n1 )
ORG(None)
L("xt_U/MOD")
-   CALL(pop_data_fn)
-   STW(md2)
-   CALL(pop_data_fn)
-   STW(md1)
-   CALL(udivide_fn)
-   LDW(md1)
-   CALL(push_data_fn)
-   LDW(md3)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Replaces the last two values on the data
# stack by their quotient and remainder.
# The two numbers n1 and n2 are treated as signed
# numbers.
# ( n2 n1 -- n2%n1 n2/n1 )
ORG(None)
L("xt_/MOD")
-   CALL(pop_data_fn)
-   STW(md2)
-   CALL(pop_data_fn)
-   STW(md1)
-   CALL(divide_fn)
-   LDW(md1)
-   CALL(push_data_fn)
-   LDW(md3)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Tests if the top of the stack equals to zero.
# ( n -- n==0 )
ORG(None)
L("xt_=0")
-   CALL(pop_data_fn)
-   BEQ(".xt_=0_satisfied")
-   LDI(0)
-   BRA(".xt_=0_end")
L(".xt_=0_satisfied")
-   LDI(1)
L(".xt_=0_end")
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Tests if the top of the stack is not zero.
# ( n -- n<>0 )
ORG(None)
L("xt_<>0")
-   CALL(pop_data_fn)
-   BNE(".xt_<>0_satisfied")
-   LDI(0)
-   BRA(".xt_<>0_end")
L(".xt_<>0_satisfied")
-   LDI(1)
L(".xt_<>0_end")
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Tests if the top of the stack is positive.
# ( n -- n>0 )
ORG(None)
L("xt_>0")
-   CALL(pop_data_fn)
-   BGT(".xt_>0_satisfied")
-   LDI(0)
-   BRA(".xt_>0_end")
L(".xt_>0_satisfied")
-   LDI(1)
L(".xt_>0_end")
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Tests if the top of the stack is negative.
# ( n -- n<0 )
ORG(None)
L("xt_<0")
-   CALL(pop_data_fn)
-   BLT(".xt_<0_satisfied")
-   LDI(0)
-   BRA(".xt_<0_end")
L(".xt_<0_satisfied")
-   LDI(1)
L(".xt_<0_end")
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Tests if the top of the stack is non-negative.
# ( n -- n>=0 )
ORG(None)
L("xt_>=0")
-   CALL(pop_data_fn)
-   BGE(".xt_>=0_satisfied")
-   LDI(0)
-   BRA(".xt_>=0_end")
L(".xt_>=0_satisfied")
-   LDI(1)
L(".xt_>=0_end")
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Tests if the top of the stack is non-positive.
# ( n -- n<=0 )
ORG(None)
L("xt_<=0")
-   CALL(pop_data_fn)
-   BLE(".xt_<=0_satisfied")
-   LDI(0)
-   BRA(".xt_<=0_end")
L(".xt_<=0_satisfied")
-   LDI(1)
L(".xt_<=0_end")
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Compares two numbers. That is, returns
# a number with the same sign as "n2 - n1" without the wrap
# around due to the 16-bit limit.
# ( n2 n1 -- cmp )
ORG(None)
L("xt_CMP")
-   CALL(pop_data_fn)
-   STW(md2)
-   CALL(pop_data_fn)
-   STW(md1)
-   CALL(signed_cmp_fn)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Compares unsignedtwo numbers. That is, returns
# a number with the same sign as "n2 - n1" without the wrap
# around due to the 16-bit limit.
# ( n2 n1 -- ucmp )
ORG(None)
L("xt_UCMP")
-   CALL(pop_data_fn)
-   STW(md2)
-   CALL(pop_data_fn)
-   STW(md1)
-   CALL(unsigned_cmp_fn)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Pushes a random value on top of the data stack.
# ( -- n )
ORG(None)
L("xt_RAND")
-   LDWI(SYS_Random_34)
-   STW(sysFn)
-   SYS(34)  # Call SYS_Random_34
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Gets a byte from the serial port.
# ( -- b )
ORG(None)
L("xt_SERIAL")
-   LD(serialRaw)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Gets the button state.
# ( -- b )
ORG(None)
L("xt_BUTTON")
-   LD(buttonState)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Gets a byte from the keyboard.
# ( -- b )
ORG(None)
L("xt_KEY")
-   LD(serialRaw)
-   STW(tmp1)
L(".xt_KEY_loop")
-       LD(serialRaw)
-       XORW(tmp1)
-       BEQ(".xt_KEY_loop")
-   LD(serialRaw)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Fills the input buffer.
# ( -- )
ORG(None)
L("xt_READ")
-   CALL(setup_draw_fn)
-   LDWI(buf_base)
-   STW(buf)
-   STW(inp)
-   CALL(read_fn)
-   CALL(next_word_fn)

# Scrolls the screen by a specified number of lines.
# ( n -- )
ORG(None)
L("xt_SCROLL")
-   CALL(pop_data_fn)
-   CALL(scroll_fn)
-   CALL(next_word_fn)

# Puts a character onto the screen.
# ( b -- )
ORG(None)
L("xt_EMIT")
-   CALL(setup_draw_fn)
-   CALL(pop_data_fn)
-   CALL(emit_fn)
-   CALL(next_word_fn)

# Prints a strings onto the screen.
# ( n addr -- )
ORG(None)
L("xt_TYPE")
-   CALL(setup_draw_fn)
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(pop_data_fn)
-   STW(tmp2)

L(".xt_TYPE_loop")
-       LDW(tmp2)
-       BEQ(".xt_TYPE_end")
-       SUBI(1)
-       STW(tmp2)

-       LDW(tmp1)
-       PEEK()
-       STW(tmp3)
-       LDI(tmp1)
-       CALL(cforward_fn)
-       LDW(tmp3)
-       CALL(emit_fn)
-       BRA(".xt_TYPE_loop")
L(".xt_TYPE_end")
-   CALL(next_word_fn)

# Clears the screen with the background color.
# ( -- )
ORG(None)
L("xt_CLL")
-   CALL(setup_draw_fn)
-   LDW(scr_pos)
-   CALL(clearline_fn)
-   CALL(next_word_fn)

# Clears the screen with the background color.
# ( -- )
ORG(None)
L("xt_CLS")
-   CALL(setup_draw_fn)
-   LDWI(0x0800)
-   STW(tmp2)
-   LDWI(0x0860)
-   STW(tmp1)
L(".xt_CLS_loop")
-       CALL(clearline_fn)
-       LDW(tmp1)
-       ADDW(tmp2)
-       STW(tmp1)
-       BLT(".xt_CLS_end")
-       BRA(".xt_CLS_loop")
L(".xt_CLS_end")
-   CALL(next_word_fn)

# Parses the current word.
# ( b -- n addr )
ORG(None)
L("xt_WORD")
-    CALL(pop_data_fn)
-    CALL(parse_word_fn)
-    LD(wlen)
-    CALL(push_data_fn)
-    LDW(wbuf)
-    CALL(push_data_fn)
-    CALL(next_word_fn)

# Parses a number.
# ( l a -- n 1 | 0 )
ORG(None)
L("xt_NUMBER")
-   CALL(pop_data_fn)
-   STW(str2)
-   CALL(pop_data_fn)
-   STW(str1)
-   CALL(number_fn)
-   BLE(".xt_NUMBER_fail")
-   LDW(tmp1)
-   CALL(push_data_fn)
-   LDI(1)
L(".xt_NUMBER_end")
-   CALL(push_data_fn)
-   CALL(next_word_fn)
L(".xt_NUMBER_fail")
-   LDI(0)
-   BRA(".xt_NUMBER_end")

# Copies memory from one location to another
# (from a2 to a1, l bytes).
# ( l a2 a1 -- )
ORG(None)
L("xt_MEMCPY")
-   CALL(pop_data_fn)
-   STW(tmp3)
-   CALL(pop_data_fn)
-   STW(tmp2)
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(memcpy_fn)
-   CALL(next_word_fn)

# Checks if two strings are equal.
# ( l2 a2 l1 a1 -- e )
ORG(None)
L("xt_S=")
-   CALL(pop_data_fn)
-   STW(tmp4)
-   CALL(pop_data_fn)
-   STW(tmp3)
-   CALL(pop_data_fn)
-   STW(tmp2)
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(streq_fn)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Finds a word in the dictionary.
# ( l a -- n )
ORG(None)
L("xt_FIND")
-   CALL(pop_data_fn)
-   STW(str2)
-   CALL(pop_data_fn)
-   STW(str1)
-   CALL(find_fn)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# To process the next token (word or number).
# ( b -- l a 0 | w 1 | n 2 )
ORG(None)
L("xt_TOKEN")
-   CALL(pop_data_fn)
-   CALL(parse_word_fn)
-   LD(wlen)
-   BEQ(".xt_TOKEN_error")
-   STW(str1)
-   LDW(wbuf)
-   STW(str2)
-   CALL(find_fn)
-   BEQ(".xt_TOKEN_number")
-   CALL(push_data_fn)
-   LDI(1)
-   BRA(".xt_TOKEN_end")
L(".xt_TOKEN_number")
-   CALL(number_fn)
-   BLE(".xt_TOKEN_error")
-   LDW(tmp1)
-   CALL(push_data_fn)
-   LDI(2)
-   BRA(".xt_TOKEN_end")
L(".xt_TOKEN_error")
-   LD(wlen)
-   CALL(push_data_fn)
-   LDW(wbuf)
-   CALL(push_data_fn)
-   LDI(0)
L(".xt_TOKEN_end")
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Verifies if the word is not compile only
# and we trying to execute it.
# ( a b -- a b | l a 0 )
# where ( a b ) = ( w 1 ) or ( n 2 )
ORG(None)
L("xt_VERIFY")
-   LDW(state)
-   BNE(".xt_VERIFY_end")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   SUBI(1)
-   BEQ(".xt_VERIFY_word")
-   LDW(tmp1)
L(".xt_VERIFY_push_end")
-   CALL(push_data_fn)
L(".xt_VERIFY_end")
-   CALL(next_word_fn)
L(".xt_VERIFY_word")
-   CALL(pop_data_fn)
-   STW(tmp2)
-   STW(tmp1)
-   CALL(flags_fn)
-   ANDI(F_COMP)
-   BNE(".xt_VERIFY_error")
-   LDW(tmp2)
-   CALL(push_data_fn)
-   LDI(1)
-   BRA(".xt_VERIFY_push_end")
L(".xt_VERIFY_error")
-   CALL(cfa_fn)
-   LDW(tmp2)
-   CALL(push_data_fn)
-   LDW(tmp3)
-   CALL(push_data_fn)
-   LDI(0)
-   BRA(".xt_VERIFY_push_end")

# To execute the token.
# ( w 1 | n 2 -- )
ORG(None)
L("xt_DISPATCH")
-   LDW(state)
-   BNE(".xt_DISPATCH_compile")
-   CALL(pop_data_fn)
-   XORI(1)
-   BNE(".xt_DISPATCH_end")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(flags_fn)
L(".xt_DISPATCH_execute_word")
-   CALL(cfa_fn)
-   LDW(tmp1)
-   CALL(exec_fn)
L(".xt_DISPATCH_compile")
-   CALL(pop_data_fn)
-   XORI(1)
-   BEQ(".xt_DISPATCH_compile_word")
-   LDWI("LIT")
-   CALL(comma_fn)
-   CALL(pop_data_fn)
L(".xt_DISPATCH_compile_end")
-   CALL(comma_fn)
L(".xt_DISPATCH_end")
-   CALL(next_word_fn)
L(".xt_DISPATCH_compile_word")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(flags_fn)
-   ANDI(F_IMMD)
-   BNE(".xt_DISPATCH_execute_word")
-   CALL(cfa_fn)
-   LDW(tmp1)
-   BRA(".xt_DISPATCH_compile_end")

# Returns the flags of a word.
# ( w -- flags )
ORG(None)
L("xt_FLAGS")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(flags_fn)
-   LDW(tmp1)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Returns the name of a word.
# ( w -- l a )
ORG(None)
L("xt_NAME")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(flags_fn)
-   CALL(cfa_fn)
-   LDW(tmp2)
-   CALL(push_data_fn)
-   LDW(tmp3)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Returns the CFA of a word.
# ( w -- cfa )
ORG(None)
L("xt_>CFA")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(flags_fn)
-   CALL(cfa_fn)
-   LDW(tmp1)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Creates a new word.
# ( l a -- )
ORG(None)
L("xt_CREATE")
-   CALL(pop_data_fn)
-   STW(tmp2)
-   CALL(pop_data_fn)
-   STW(tmp1)
-   LDI(here)
-   CALL(align_addr_fn)
-   LDW(latest)
-   DOKE(here)
-   LDW(here)
-   STW(latest)
-   LDI(here)
-   CALL(forward_fn)
-   LDW(here)
-   STW(tmp4)
-   CALL(str_comma_fn)
#-   LDW(tmp4)
#-   PEEK()
#-   ORI(F_HIDD)  # Hide the word
#-   POKE(tmp4)
-   CALL(next_word_fn)

# Compiles a word.
# ( n -- )
ORG(None)
L("xt_,")
-   CALL(pop_data_fn)
-   CALL(comma_fn)
-   CALL(next_word_fn)

# Compiles a string.
# ( l a -- )
ORG(None)
L("xt_S,")
-   CALL(pop_data_fn)
-   STW(tmp2)
-   CALL(pop_data_fn)
-   STW(tmp1)
-   CALL(str_comma_fn)
-   CALL(next_word_fn)

# Converts a number to string.
# ( n -- l a )
ORG(None)
L("xt_CONVERT")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   LDWI(auxbuf_end)
-   STW(tmp2)
-   CALL(cv_number_fn)
-   LDWI(auxbuf_end)
-   SUBW(tmp2)
-   CALL(push_data_fn)
-   INC(tmp2)
-   LDW(tmp2)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Converts an unsigned number to string.
# ( n -- l a )
ORG(None)
L("xt_UCONVERT")
-   CALL(pop_data_fn)
-   STW(tmp1)
-   LDWI(auxbuf_end)
-   STW(tmp2)
-   CALL(cv_unumber_fn)
-   LDWI(auxbuf_end)
-   SUBW(tmp2)
-   CALL(push_data_fn)
-   INC(tmp2)
-   LDW(tmp2)
-   CALL(push_data_fn)
-   CALL(next_word_fn)


# Returns the number of free bytes.
# ( -- n )
ORG(None)
L("xt_FREE")
-   LDI(0)
-   STW(tmp3)
-   LD(memSize)
-   BNE(".xt_FREE_not_64k")
-   LDWI(0x100)
L(".xt_FREE_not_64k")
-   STW(tmp1)
-   SUBI(128)
-   BLE(".xt_FREE_32k")
-   ST(tmp3+1)
-   LDI(128)
-   STW(tmp1)
L(".xt_FREE_32k")
-   LD(here+1)
-   STW(tmp2)
-   LDW(tmp1)
-   SUBW(tmp2)
-   SUBI(9)    # The first 8 pages are not usable
-   STW(md1)
-   LDWI(segment_size)
-   STW(md2)
-   CALL(multiply_fn)
-   LD(here)
-   STW(tmp2)
-   LDW(md2)
-   SUBW(tmp2)
-   ADDW(md3)
-   ADDW(tmp3)
-   CALL(push_data_fn)
-   CALL(next_word_fn)

# Trigger the callback to resolve the segments.
last_segment_addr = RESOLVE_SEGMENTS(
    pack_segments_factory(base=code_base,
                          size=segment_size)
)

if data_base is None:
    data_base = last_segment_addr + 256
    print("Resolved data_base = 0x$%04X" % data_base)

# ============================
# Data segment
# ============================

ORG(data_base, size=segment_size, callback=extend_callback)

# The basic words

define_word("ADJ", 0, xt="xt_ADJ")
define_word("ALIGN", 0, xt="xt_ALIGN")
define_word("EXECUTE", 0, xt="xt_EXECUTE")
define_word("BRANCH", F_COMP, xt="xt_BRANCH")
define_word("0BRANCH", F_COMP, xt="xt_0BRANCH")
define_word("EXIT", F_COMP, xt="xt_EXIT")
define_word("LIT", F_COMP, xt="xt_LIT")
define_word("LITSTR", F_COMP, xt="xt_LITSTR")
define_word("DROP", 0, xt="xt_DROP")
define_word("2DROP", 0, xt="xt_2DROP")
define_word("SWAP", 0, xt="xt_SWAP")
define_word("DUP",  0, xt="xt_DUP")
define_word("2DUP",  0, xt="xt_2DUP")
define_word("?DUP",  0, xt="xt_?DUP")
define_word("OVER",  0, xt="xt_OVER")
define_word("ROT", 0, xt="xt_ROT")
define_word("RROT", 0, xt="xt_RROT")
define_word("R>", 0, xt="xt_R>")
define_word(">R", 0, xt="xt_>R")
define_word("DSP@", 0, xt="xt_DSP@")
define_word("DSP!", 0, xt="xt_DSP!")
define_word("RSP@", 0, xt="xt_RSP@")
define_word("RSP!", 0, xt="xt_RSP!")
define_word("!", 0, xt="xt_!")
define_word("@", 0, xt="xt_@")
define_word("C!", 0, xt="xt_C!")
define_word("C@", 0, xt="xt_C@")
define_word("+", 0, xt="xt_+")
define_word("-", 0, xt="xt_-")
define_word("AND", 0, xt="xt_AND")
define_word("OR", 0, xt="xt_OR")
define_word("XOR", 0, xt="xt_XOR")
define_word("NOT", 0, xt="xt_NOT")
define_word("NEG", 0, xt="xt_NEG")
define_word("*", 0, xt="xt_*")
define_word("U/MOD", 0, xt="xt_U/MOD")
define_word("/MOD", 0, xt="xt_/MOD")
define_word("=0", 0, xt="xt_=0")
define_word("<>0", 0, xt="xt_<>0")
define_word(">0", 0, xt="xt_>0")
define_word("<0", 0, xt="xt_<0")
define_word(">=0", 0, xt="xt_>=0")
define_word("<=0", 0, xt="xt_<=0")
define_word("CMP", 0, xt="xt_CMP")
define_word("UCMP", 0, xt="xt_UCMP")
define_word("RAND", 0, xt="xt_RAND")
define_word("SERIAL", 0, xt="xt_SERIAL")
define_word("BUTTON", 0, xt="xt_BUTTON")
define_word("KEY", 0, xt="xt_KEY")
define_word("READ", 0, xt="xt_READ")
define_word("SCROLL", 0, xt="xt_SCROLL")
define_word("EMIT", 0, xt="xt_EMIT")
define_word("TYPE", 0, xt="xt_TYPE")
define_word("CLL", 0, xt="xt_CLL")
define_word("CLS", 0, xt="xt_CLS")
define_word("WORD", 0, xt="xt_WORD")
define_word("NUMBER", 0, xt="xt_NUMBER")
define_word("MEMCPY", 0, xt="xt_MEMCPY")
define_word("S=", 0, xt="xt_S=")
define_word("FIND", 0, xt="xt_FIND")
define_word("TOKEN", 0, xt="xt_TOKEN")
define_word("VERIFY", 0, xt="xt_VERIFY")
define_word("DISPATCH", 0, xt="xt_DISPATCH")
define_word("FLAGS", 0, xt="xt_FLAGS")
define_word("NAME", 0, xt="xt_NAME")
define_word(">CFA", 0, xt="xt_>CFA")
define_word("CREATE", 0, xt="xt_CREATE")
define_word(",", 0, xt="xt_,")
define_word("S,", 0, xt="xt_S,")
define_word("CONVERT", 0, xt="xt_CONVERT")
define_word("UCONVERT", 0, xt="xt_UCONVERT")
define_word("FREE", 0, xt="xt_FREE")

# Some constants
define_constant("HERE", here)
define_constant("LATEST", latest)
define_constant("BASE", base)
define_constant("STATE", state)
define_constant("SCR_POS", scr_pos)
define_constant("SCR_COLOR", scr_color)
define_constant("F_IMMD", F_IMMD)
define_constant("F_COMP", F_COMP)
define_constant("F_HIDD", F_HIDD)
define_constant("F_MASK", F_MASK)
define_constant("DOCOL", "xt_DOCOL")
define_constant("DOCON", "xt_DOCON")
define_constant("RSP0", rsp_top)
define_constant("DSP0", dsp_top)
define_constant("SEGSIZE", segment_size)
define_constant("FALSE", 0)
define_constant("TRUE", 1)
define_constant("BL", ord(' '))
define_constant("\\n", ord('\n'))

define_word("CR", 0)
-   reference("\\n")
-   reference("EMIT")
-   reference("EXIT")

define_word("SPACE", 0)
-   reference("BL")
-   reference("EMIT")
-   reference("EXIT")

define_word(".", 0)
-   reference("CONVERT")
-   reference("TYPE")
-   reference("SPACE")
-   reference("EXIT")

define_word("U.", 0)
-   reference("UCONVERT")
-   reference("TYPE")
-   reference("SPACE")
-   reference("EXIT")

define_word("RESET", 0)
L("cold_start")
-   reference("CLS")
-   reference("LITSTR")
-   place_string(welcome_string)
-   reference("TYPE")
-   reference("FREE")
-   reference("U.")
-   reference("LITSTR")
-   place_string(free_bytes_string)
-   reference("TYPE")
-   reference("INTERPRET")

define_word("INTERPRET", 0)
-   reference("RSP0")
-   reference("RSP!")
-   reference("DSP0")
-   reference("DSP!")
L(".interpret_loop1")
-       reference("LIT")
-       reference(ord('>'))
-       reference("STATE")
-       reference("@")
-       reference("0BRANCH")
-       reference(".interpret_prompt")
-       reference("DROP")
-       reference("BL")
L(".interpret_prompt")
-       reference("EMIT")
-       reference("READ")
-       reference("CR")
L(".interpret_loop2")
-           reference("BL")
-           reference("TOKEN")
-           reference("?DUP")
-           reference("0BRANCH")
-           reference(".interpret_test_error")
-           reference("VERIFY")
-           reference("?DUP")
-           reference("0BRANCH")
-           reference(".interpret_error_compile_only")
-           reference("DISPATCH")
-           reference("BRANCH")
-           reference(".interpret_loop2")
L(".interpret_test_error")
-       reference("OVER")
-       reference("=0")
-       reference("0BRANCH")
-       reference(".interpret_error_unknown_word")
-       reference("2DROP")
-       reference("BRANCH")
-       reference(".interpret_loop1")
L(".interpret_error_unknown_word")
-       reference("LIT")
-       reference(ord('?'))
L(".interpret_continue")
-       reference("EMIT")
-       reference("SPACE")
-       reference("TYPE")
-       reference("CR")
-       reference("BRANCH")
-       reference(".interpret_loop1")
L(".interpret_error_compile_only")
-       reference("LIT")
-       reference(ord('!'))
-       reference("BRANCH")
-       reference(".interpret_continue")

define_word("'", F_IMMD)
-   reference("BL")
-   reference("WORD")
-   reference("2DUP")
-   reference("FIND")
-   reference("?DUP")
-   reference("0BRANCH")
-   reference(".interpret_error_unknown_word")
-   reference(">CFA")
-   reference("RROT")
-   reference("2DROP")
-   reference("EXIT")

define_word("WORDS", 0)
-   reference("LATEST")
L(".words_loop")
-   reference("@")
-   reference("?DUP")
-   reference("0BRANCH")
-   reference(".words_end")
-   reference("DUP")
-   reference("FLAGS")
-   reference("C@")
-   reference("F_HIDD")
-   reference("AND")
-   reference("=0")
-   reference("0BRANCH")
-   reference(".words_loop")
-   reference("DUP")
-   reference("NAME")
-   reference("TYPE")
-   reference("SPACE")
-   reference("BRANCH")
-   reference(".words_loop")
L(".words_end")
-   reference("EXIT")

L("here_loc")

# ============================
# Emulation
# ============================

gt1_bytes = END("start", filename=None)
vcpu = VirtualCpu()

src_text = """
BL WORD [ CREATE DOCOL , ' FALSE , ' STATE , ' ! ,  ' EXIT ,
LATEST @ FLAGS DUP C@ F_HIDD NOT AND F_IMMD OR SWAP C!

BL WORD ] CREATE DOCOL , ' TRUE , ' STATE , ' ! , ' EXIT ,
LATEST @ FLAGS DUP C@ F_HIDD NOT AND SWAP C!

BL WORD UNHIDE CREATE DOCOL , ]
LATEST @ FLAGS DUP C@ F_HIDD NOT AND SWAP C! EXIT [
LATEST @ FLAGS DUP C@ F_HIDD NOT AND SWAP C!

BL WORD : CREATE DOCOL , ] BL WORD CREATE DOCOL , ] EXIT [ UNHIDE

: IMMEDIATE LATEST @ FLAGS DUP C@ F_IMMD OR SWAP C! EXIT [ UNHIDE

: ; LIT EXIT , UNHIDE [ ' [ , ] EXIT [ UNHIDE IMMEDIATE

: COMPILE-ONLY LATEST @ FLAGS DUP C@ F_COMP OR SWAP C! ;

: CONSTANT BL WORD CREATE DOCON , , ;

: VARIABLE BL WORD CREATE DOCON , HERE @ 0 , HERE @ SWAP ! 0 , ;

: DECIMAL 10 BASE ! ;
: HEX 16 BASE ! ;

: IF LIT 0BRANCH , HERE @ 0 , ; IMMEDIATE COMPILE-ONLY
: THEN HERE @ SWAP ! ; IMMEDIATE COMPILE-ONLY
: ELSE LIT BRANCH , HERE @ SWAP 0 ,
  HERE @ SWAP ! ; IMMEDIATE COMPILE-ONLY

: BEGIN HERE @ ; IMMEDIATE COMPILE-ONLY
: UNTIL LIT 0BRANCH , , ; IMMEDIATE COMPILE-ONLY
: AGAIN LIT BRANCH , , ; IMMEDIATE COMPILE-ONLY
: WHILE LIT 0BRANCH , HERE @ 0 , ; IMMEDIATE COMPILE-ONLY
: REPEAT LIT BRANCH , SWAP , HERE @ SWAP ! ; IMMEDIATE COMPILE-ONLY

: CHAR BL WORD SWAP IF C@ ELSE 0 THEN ;

: ( LIT [ CHAR ) , ] WORD 2DROP ;

( We have comments! Not that I happen to use many comments anyway :-)

: = - =0 ;
: <> - <>0 ;
: > CMP >0 ;
: < CMP <0 ;
: >= CMP >=0 ;
: <= CMP <=0 ;

: / /MOD SWAP DROP ;
: MOD /MOD DROP ;

: S" LIT [ CHAR " , ] WORD STATE @
  IF LIT LITSTR , S, THEN
; IMMEDIATE

: ." LIT [ CHAR " , ] WORD STATE @
  IF LIT LITSTR , S, LIT TYPE , ELSE TYPE THEN
; IMMEDIATE

: FORGET BL WORD FIND DUP @ LATEST ! HERE ! INTERPRET ;

FREE .
"""

src = io.BytesIO(src_text.encode("ascii"))
out = io.BytesIO()

def emit_breakpoint(vcpu, addr):
    c = vcpu.get_vAC()
    out.write(bytes(tuple([ c ])))

def read_breakpoint(vcpu, addr):
    if addr == ADDR(".read_loop1"):
        vcpu.write_byte(serialRaw, 0xFF)
    else:
        b = src.read(1)
        if len(b) == 0:
            vcpu.halt()
        else:
            vcpu.write_byte(serialRaw, b[0])

breakpoints = {}
breakpoints[ADDR("emit")] = emit_breakpoint
breakpoints[ADDR(".read_loop1")] = read_breakpoint
breakpoints[ADDR(".read_loop2")] = read_breakpoint

here_before = ADDR("here_loc")
latest_before = ADDR(current_link[0])


print("\nRunning GtForth in emulated vCPU ...")
vcpu.load_gt1(gt1_bytes=gt1_bytes)
vcpu.write_byte(memSize, 128)
try:
    vcpu.run(breakpoints=breakpoints)
finally:
    out_text = out.getvalue().decode("ascii")
    print(out_text)

here_current = vcpu.read_word(here)
latest_current = vcpu.read_word(latest)
print("HERE (before): 0x%04X" % here_before)
print("HERE (after): 0x%04X" % here_current)
print("LATEST (before): 0x%04X" % latest_before)
print("LATEST (after): 0x%04X\n" % latest_current)

assert(here_current < 0x8000)

# Inject the new code
print("Injecting new code into GT1 file ...")
addr = here_before
while addr < here_current:
    b = vcpu.read_byte(addr)
    BYTE(b)
    addr += 1
    if (addr & 0xFF) >= segment_size:
        addr = (addr & ~0xFF) + 0x100

def inject_new_values_factory(new_values):
    def do_inject(gt1, symbols):
        resolved = set()
        for segment in gt1:
            base = segment[0]
            contents = segment[3]
            size = len(contents)
            for addr, val in new_values.items():
                if addr >= base and addr < base + size:
                    resolved.add(addr)
                    offset = addr - base
                    contents[offset] = val & 0xFF
                    contents[offset + 1] = (val >> 8) & 0xFF

    return do_inject

RESOLVE_SEGMENTS(inject_new_values_factory({
    (init1_base + here): here_current,
    (init1_base + latest): latest_current
}))

# ============================
# GT1 file generation
# ============================

# Execution start address
END("start")

used_bytes = segment_size * (here_current >> 8) \
    + (here_current & 0xFF)
free_bytes = segment_size * (0x80 - 0x08) - used_bytes
print("used bytes = %d" % used_bytes)
print("free bytes for 32K RAM = %d" % free_bytes)
