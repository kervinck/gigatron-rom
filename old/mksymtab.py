#!/usr/bin/env python

import re

symtab = dict()

with open("theloop.asm") as f:
    for line in f:
        match = re.match(r"^(\.?[a-z0-9_]+): *([0-9a-f]{4})", line, re.I)
        if match:
            symtab[int(match.group(2), 16)] = match.group(1)

print symtab

