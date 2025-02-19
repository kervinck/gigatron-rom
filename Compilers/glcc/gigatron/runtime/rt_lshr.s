
def scope():

    # ----------------------------------------
    # LSHRU : LAC <-- LAC >> vAC
    # ----------------------------------------

    def code0():
        """ Logical right shift LAC by vAC bits.
            Thrashes T5 i.e. sysArgs[67] like its LSRXA counterpart.
            Also thrashes sysFn. """
        nohop()
        if args.cpu >= 7:
            warning('cpu7: use LSRXA instead of _@_lshru')
            label('_@_lshru')
            ANDI(0x1f);LSRXA()
            RET()
        else:
            label('_@_lshru')
            ST(T5)
            label('__@lshru_t5')
            PUSH()
            LD(T5);ANDI(16);_BEQ('.l4')
            LDW(LAC+2);STW(LAC);LDI(0);STW(LAC+2)
            label('.l4')
            LD(T5);ANDI(8);_BEQ('.l5')
            LDW(LAC+1);STW(LAC);LD(LAC+3);STW(LAC+2)
            label('.l5')
            if args.cpu < 5:
                LDWI('__@shrsysfn');STW('sysFn')
            LD(T5);ANDI(7);_BEQ('.ret')
            if args.cpu < 5:
                CALL('sysFn')
            else:
                CALLI('__@shrsysfn')
            LDW(LAC);SYS(52);ST(LAC)
            LDW(LAC+1);SYS(52);ST(LAC+1)
            LDW(LAC+2);SYS(52);STW(LAC+2)
            label('.ret')
            tryhop(2);POP();RET()

    module(name='rt_lshru.s',
           code= [ ('EXPORT', '_@_lshru'),
                   ('EXPORT', '__@lshru_t5'),
                   ('IMPORT', '__@shrsysfn') if args.cpu < 7 else ('NOP',),
                   ('CODE', '_@_lshru', code0) ])

    # ----------------------------------------
    # LSHRS : LAC <-- LAC >> vAC
    # ----------------------------------------

    def code1():
        """ Arithmetical right shift LAC by vAC bits.
            Thrashes T[45] i.e. sysArgs[4567], sysFn."""
        if args.cpu >= 7:
            nohop()
            label('_@_lshrs')
            ANDI(0x1f);ST(T5);LDW(LAC+2);_BLT('.s1')
            LD(T5);LSRXA();RET()
            label('.s1')
            LD(T5);NOTVL(LAC);LSRXA();NOTVL(LAC);RET()
        else:
            label('_@_lshrs')
            PUSH();ST(T5)
            LDW(LAC+2);_BLT('.s1')
            _CALLJ('__@lshru_t5');_BRA('.sret')
            label('.s1')
            _LDI(0xffff);STW(T4);XORW(LAC);STW(LAC)
            LDW(T4);XORW(LAC+2);STW(LAC+2)
            _CALLJ('__@lshru_t5')
            LDW(T4);XORW(LAC);STW(LAC)
            LDW(T4);XORW(LAC+2);STW(LAC+2)
            label('.sret')
            tryhop(2);POP();RET()

    module(name='rt_lshrs.s',
           code= [ ('EXPORT', '_@_lshrs'),
                   ('IMPORT', '__@lshru_t5') if args.cpu < 7 else ('NOP',),
                   ('CODE', '_@_lshrs', code1) ])

scope()


# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
