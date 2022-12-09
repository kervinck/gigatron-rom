import argparse, os, sys


def hi(addr):
    return (addr >> 8) & 0xff

def lo(addr):
    return addr & 0xff

def adddata(gt1data,bindata,addr):
    execaddr = gt1data[-3:]
    ngt1 = gt1data[:-3]
    i = 0
    n = len(bindata)
    ngt1 += bytes((hi(addr-2), lo(addr-2), 2, lo(n), hi(n)))
    while i < n:
        l = 256
        if l > n - i:
            l = n - i
        if l > 0x100 - lo(addr):
            l = 0x100 - lo(addr)
        ngt1 += bytes((hi(addr), lo(addr), lo(l)))
        ngt1 += bindata[i:i+l]
        i += l
        addr += l
    ngt1 += execaddr
    return ngt1

def addbook(argv):
    try:
        parser = argparse.ArgumentParser(
            usage='addbook --addr=<xxx> <gt1file> <binfile>',
            description='Concatenate binary data to a GT1 file')
        parser.add_argument('gt1', type=str, help='gt1 file')
        parser.add_argument('bin', type=str, help='bin file')
        parser.add_argument('--addr', type=str, help='starting address',
                            action='store', default='0xc000' )
        args = parser.parse_args(argv)
        with open(args.gt1,"rb") as fd:
            gt1data = fd.read()
        with open(args.bin,"rb") as fd:
            bindata = fd.read()
        gt1data = adddata(gt1data, bindata, int(args.addr, 0))
        with open(args.gt1,"wb") as fd:
            fd.write(gt1data)
        return 0
    except FileNotFoundError as err:
        print(str(err), file=sys.stderr)
    #except Exception as err:
    #    print(repr(err), file=sys.stderr)

if __name__ == '__main__':
    sys.exit(addbook(sys.argv[1:]))

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
	
