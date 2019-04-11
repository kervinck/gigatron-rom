#!/usr/bin/env python3

import asm, sys

def main(files):
    for f in files:
        exec(compile(open(f, "rb").read(), f, 'exec'))

    asm.link('@start')

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
