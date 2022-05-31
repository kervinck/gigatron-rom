#!/usr/bin/env python3

import argparse, asm, os, sys

def main(argv):
    parser = argparse.ArgumentParser(description='assemble and link Gigatron sources into a .gt1 file')
    parser.add_argument('-o', type=str, help='the output file path')
    parser.add_argument('-d', action='store_true', help='enable rich debug output on stderr')
    parser.add_argument('files', nargs='+')

    args = parser.parse_args(argv)

    for f in args.files:
        exec(compile(open(f, "rb").read(), f, 'exec'))

    outf = sys.stdout.buffer
    if 'o' in args:
        outf = open(args.o, 'wb')

    logf = open(os.devnull, 'w')
    if args.d:
        logf = sys.stderr

    asm.link('@start', outf, logf)

if __name__ == '__main__':
    sys.exit(main(sys.argv[1:]))
