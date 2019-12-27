#!/usr/bin/env python3

import argparse, itertools, os, struct, sys
from os import path
from sys import argv

parser = argparse.ArgumentParser(description=
        'Convert a raw 24-bit RGB image into a packed 6-bit RGB image.',
        epilog=
        'Example usage: %(prog)s image.raw -n MyImage -a 0x0300 -l 0x00ff')

parser.add_argument('raw_file', nargs="+", help='A raw 24-bit RGB image')
parser.add_argument('-n', '--name',
                    help='Image name for variable definition/comments')
parser.add_argument('-a', '--start_address', type=lambda x:int(x, 0),
                    help='Start address for data')
parser.add_argument('-l', '--size_limit', type=lambda x:int(x, 0),
                    help='Size limit before splitting data segment')
parser.add_argument('-w', '--output_width', type=int, default=14,
                    help='Number of values to output before a line-break')
parser.add_argument('-d', '--add_defs', action='store_true',
                    help='Add variable definitions')
args = parser.parse_args()

stem = args.name
if stem is None:
    stem = path.splitext(path.basename(args.raw_file))[0]

rawfile = stem + '.raw'
gclfile = stem + '.gcl'

raw = []
with open(rawfile, 'wb') as output:
    for input_file in args.raw_file:
        with open(input_file, 'rb') as input:
            input.seek(0, os.SEEK_END)
            size = input.tell()
            input.seek(0, os.SEEK_SET)

            while input.tell() < size:
                # Read in 4 24-bit pixels and convert them to 6-bit pixels
                pixels6 = []
                for i in range(0, 4):
                    pixel24 = struct.unpack('<3B', input.read(3))
                    r = int(round((pixel24[0] / 255.0) * 3))
                    g = int(round((pixel24[1] / 255.0) * 3))
                    b = int(round((pixel24[2] / 255.0) * 3))
                    pixels6.append((b << 4) | (g << 2) | (r))
                byte3 = ((pixels6[3] << 2) & 0xff) | ((pixels6[2] >> 4) & 0xff)
                byte2 = ((pixels6[2] << 4) & 0xff) | ((pixels6[1] >> 2) & 0xff)
                byte1 = ((pixels6[1] << 6) & 0xff) | ((pixels6[0]) & 0xff)

                bytes = [byte1, byte2, byte3]
                raw = itertools.chain(raw, bytes)
                output.write(bytearray([byte1, byte2, byte3]))

# Write a GCL file
raw = list(raw)
with open(gclfile, 'w') as output:
    defs_prefix = ''
    if args.add_defs:
        defs_prefix = '[def '

    seg_start = args.start_address

    split = len(raw)
    if args.size_limit is not None:
        split = args.size_limit

    output.write('gcl0x\n\n')
    n_seg = 0
    for segment in [raw[i:i + split] for i in range(0, len(raw), split)]:
        n_seg += 1

        if args.start_address is not None:
            output.write('$%04x:' % (seg_start))
            seg_start += 0x100

        if args.size_limit and len(raw) > args.size_limit:
            output.write('\n%s{packed%s%02d}\n' % (defs_prefix, stem, n_seg))
        else:
            output.write('\n%s{packed%s}\n' % (defs_prefix, stem))

        n_bytes = 0;
        for val in segment:
            if n_bytes == args.output_width:
                output.write('\n')
                n_bytes = 0
            output.write(' $%02x#' % (val))
            n_bytes += 1

        if args.add_defs:
            output.write('] packed%s%02d=\n' % (stem, n_seg))
            if split < len(raw):
                output.write('\n\\vLR>++ ret\n')
        else:
            output.write('\n\n')
