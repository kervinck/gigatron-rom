
def scope():
    
    # memset(d,v,l)

    def code0():
        '''version that uses Sys_SetMemory_v2_54'''
        nohop()
        label('memset');                            # R8=d, R9=v, R10=l
        LDW(R8);STW(R21);STW('sysArgs2')
        LDWI('SYS_SetMemory_v2_54');STW('sysFn')    # prep sys
        LD(R9);ST('sysArgs1')
        label('.loop')
        LD(R8);ST(R20);LDI(255);ST(R20+1)           # R20 is minus count to end of block
        LDW(R10);_BGT('.memset2')
        _BEQ('.done')                               # a) len is zero
        ADDW(R20);_BRA('.memset4')                  # b) len is larger than 0x8000
        label('.memset2')
        ADDW(R20);_BLE('.memset5')                  # c) len is smaller than -R20
        label('.memset4')
        STW(R10)                                    # d) len is larger than -R20
        LDI(0);SUBW(R20);STW(R20);ST('sysArgs0');SYS(54)
        LDW(R8);ADDW(R20);STW(R8);STW('sysArgs2')
        _BRA('.loop')
        label('.memset5')
        LDW(R10);ST('sysArgs0');SYS(54)
        label('.done')
        LDW(R21)
        RET()


    return [('EXPORT', 'memset'),
            ('CODE', 'memset', code0) ]

module(code=scope(), name='memset.s');

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
