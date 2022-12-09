

def scope():

    # -- int _console_bell(int duration)
    # Sounds the bell for duration frames, honoring 'channelMask_v4'

    def code_bell():
        nohop()
        label('_console_bell')
        PUSH()
        LD('channelMask_v4');ANDI(3);LSLW();LSLW();STW(R17)
        LDWI('.cp1');SUBW(R17);STW(R17)
        LDWI(0x1fc);STW(R16);LDWI(0x2177);CALL(R17)
        LDWI(0x1fa);STW(R16);LDWI(0x100);CALL(R17)
        LD(R8);ST('soundTimer')
        POP();RET()
        DOKE(R16);INC(R16+1)
        DOKE(R16);INC(R16+1)
        DOKE(R16);INC(R16+1)
        label('.cp1')
        DOKE(R16);RET()

    module(name='cons_bell.s',
           code=[ ('EXPORT', '_console_bell'),
                  ('CODE', '_console_bell', code_bell) ] )
    
scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
