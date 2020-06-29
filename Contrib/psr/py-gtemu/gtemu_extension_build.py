"""CFFI Build script - Tells CFFI how to build the _gtemu extension

Defines the functions and structures that need to be useable from Python
using C syntax, and says where to find the C file that also be compiled and
linked.
"""
import pathlib
from cffi import FFI

# Paths to files elsewhere in the repository
REPO_ROOT = (pathlib.Path(__file__).parent / '..' / '..' / '..').resolve()
GTEMU_C = REPO_ROOT / 'Doc' / 'gtemu.c'

ffibuilder = FFI()

# These definitions become most of the the API of the extension.
# They're parsed by CFFI, but are in C syntax. 
# They're copied from gtemu.c, but in a more typical usage
# they'd be taken from a manpage or other documentation.
ffibuilder.cdef(
    """
typedef struct { // TTL state that the CPU controls
  uint16_t PC;
  uint8_t IR, D, AC, X, Y, OUT, undef;
} CpuState;
"""
)
ffibuilder.cdef("""extern uint8_t ROM[1<<16][2], RAM[1<<15], IN;""")
ffibuilder.cdef("CpuState cpuCycle(const CpuState S);")

ffibuilder.set_source(
    "_gtemu",  # name of the output C extension
    # The same code as above, but this time for including in the generated C.
    # More typically this would be a single line that includes a header.
    """\
typedef struct { // TTL state that the CPU controls
  uint16_t PC;
  uint8_t IR, D, AC, X, Y, OUT, undef;
} CpuState;

CpuState cpuCycle(const CpuState S);
extern uint8_t ROM[1<<16][2], RAM[1<<15], IN;
""",
    sources=[GTEMU_C],
    libraries=[],
)
if __name__ == "__main__":
    ffibuilder.compile(verbose=True)
