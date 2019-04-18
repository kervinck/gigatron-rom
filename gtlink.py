#!/usr/bin/env python3

import argparse, asm, sys

def main(argv):
    parser = argparse.ArgumentParser(description='assemble and link Gigatron sources into a .gt1 file')
    parser.add_argument('-o', type=str, help='the output file path')
    parser.add_argument('files', nargs='+')

    args = parser.parse_args(argv)

    for f in args.files:
        exec(compile(open(f, "rb").read(), f, 'exec'))

    outf = sys.stdout.buffer
    if 'o' in args:
        outf = open(args.o, 'wb')

    asm.link('@start', outf)

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
