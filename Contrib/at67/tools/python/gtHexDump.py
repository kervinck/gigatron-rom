#!/usr/bin/env python
"""Make a hexdump"""
import re
import sys
from binascii import hexlify
from functools import partial

def hexdump(filename):
    format = partial(re.compile(b'(..)').sub, br'0x\1, ')
    write = getattr(sys.stdout, 'buffer', sys.stdout).write
    with open(filename, 'rb') as file:
        for chunk in iter(partial(file.read), b''):
            write(format(hexlify(chunk)))

hexdump(sys.argv[1])