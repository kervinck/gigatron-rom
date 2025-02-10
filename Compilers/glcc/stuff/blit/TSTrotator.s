
# Compile with:
# $ glink r_dev7.s -o r_dev7.gt1 --rom=dev7 --map=64k --gt1-exec-address=main --entry=main --frags

def scope():


    x = 64
    y = 66
    xx = 68
    yy = 70
    ps = 72
    sh_tbl = 74
    sourceX = T3
    sourceY = T3+1
    destinX = T2
    destinY = T2+1
    temp = 84

    def code_main():
        nohop()
        label('main')
        # set video table
        MOVIW(0x100,y)
        label('main1')
        PEEKV(y);ADDI(120);POKE(y)
        INC(y);INC(y);LD(y);XORI(240);_BNE('main1')
        # set video x
        LDWI(0x0101);POKEQ(40)
        # set mode 2
        MOVIW('SYS_SetMode_v2_80', 'sysFn');LDI(3);SYS(80)
        # clear screen
        MOVIW(0x8020,T2);MOVQB(0, T3);LDWI(128*256+220);FILL()
        # cache table address
        MOVIW('Table',sh_tbl)
        # begin loop
        label('rotate1')
        MOVQW(10,y)
        LD(ps);ANDI(62);ADDW(sh_tbl);DEEKA(yy)
        BRA('rotate22')
        label('rotate2')
        ADDSV(32,yy)
        label('rotate22')
        MOVQW(5,x)
        LDW(yy);ADDW(y);ST(temp)
        LD(yy+1);ST(xx);BRA('rotate33')
        label('rotate3')
        ADDSV(32,xx)
        label('rotate33')
        LD(xx);ST(destinX)
        ADDW(y);SUBW(x);ST(sourceX)
        LD(temp);ADDW(x);ST(sourceY)
        LDI(16);ADDW(yy);ST(destinY)
        # blit
        LDWI(0x2020);BLIT()
        # continue
        INC(x);LD(x);XORI(12);_BNE('rotate3')
        LD(y);SUBI(1);ST(y);XORI(6);_BNE('rotate2')
        ADDSV(2,ps)
        # seed
        LD(6);ST(T3);MOVIW(0xbf80,T2);LDWI(0x404);FILL()
        # loop
        BRA('rotate1')

    def code_table():
        label('Table')
        bytes(81,  1)
        bytes(97,  17)
        bytes(89,  9)
        bytes(105, 25)
        bytes(85,  5)
        bytes(101, 21)
        bytes(93,  13)
        bytes(109, 29)
        bytes(83,  3)
        bytes(99,  19)
        bytes(91,  11)
        bytes(107, 27)
        bytes(87,  7)
        bytes(103, 23)
        bytes(95,  15)
        bytes(111, 31)
        bytes(82,  2)
        bytes(98,  18)
        bytes(90,  10)
        bytes(106, 26)
        bytes(86,  6)
        bytes(102, 22)
        bytes(94,  14)
        bytes(110, 30)
        bytes(84,  4)
        bytes(100, 20)
        bytes(92,  12)
        bytes(108, 28)
        bytes(88,  8)
        bytes(104, 24)
        bytes(96,  16)
        bytes(112, 32)
        
    module(name = 'r_dev7.s',
           code = [('EXPORT', 'main'),
                   ('CODE', 'main', code_main),
                   ('DATA', 'Table', code_table) ] )

        
scope()


# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
