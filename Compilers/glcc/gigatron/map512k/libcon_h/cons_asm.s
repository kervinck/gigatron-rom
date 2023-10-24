

def scope():

    # ------------------------------------------------------------
    # LOW LEVEL SCREEN ACCESS
    # ------------------------------------------------------------
    # These functions do not know about the console
    # state and simply access the screen at the provided address.
    # ------------------------------------------------------------

    ctrlBits_v5 = 0x1f8
    videoModeB = 0xa   # contain bankinfo on patched rom
    videoModeC = 0xb   # >= 0xfc on patched rom


    def code_bank():
        # Clobbers R21, R22
        nohop()
        ## save current bank
        label('_cons_save_current_bank')
        LD(videoModeB);ANDI(0xfc);XORI(0xfc);_BNE('.cscb1')
        LDWI('.savx');STW(R22);LD(videoModeC);POKE(R22)
        label('.cscb1')
        RET()
        ## restore_saved_bank
        label('_cons_restore_saved_bank')
        LDW('sysFn');STW(R21)
        LDWI('SYS_ExpanderControl_v4_40');STW('sysFn');
        label('.savx', pc()+2)
        LDWI(0x00F0);SYS(40)
        LDW(R21);STW('sysFn')
        RET()
        ## set extended banking code for address in vAC
        label('_cons_set_bank_even')
        _BGE('.wbb1')
        LDWI(0xF8F0);BRA('.wbb3')
        label('.wbb1')
        LDWI(0xE8F0);BRA('.wbb3')
        label('_cons_set_bank_odd')
        _BGE('.wbb2')
        LDWI(0xD8F0);BRA('.wbb3')
        label('.wbb2')
        LDWI(0xC8F0);BRA('.wbb3')
        label('.wbb3')
        STW(R22)
        LDW('sysFn');STW(R21)
        LDWI('SYS_ExpanderControl_v4_40');STW('sysFn')
        LDW(R22);SYS(40)
        LDW(R21);STW('sysFn')
        RET()
        
    module(name='cons_bank.s',
           code=[ ('EXPORT', '_cons_save_current_bank'),
                  ('EXPORT', '_cons_restore_saved_bank'),
                  ('EXPORT', '_cons_set_bank_even'),
                  ('EXPORT', '_cons_set_bank_odd'),
                  ('CODE', '_cons_set_bank', code_bank),
                  ('PLACE', '_cons_set_bank', 0x0200, 0x7fff) ] )

    
    # -- int _console_printchars(int fgbg, char *addr, const char *s, int len)
    #
    # Draws up to `len` characters from string `s` at the screen
    # position given by address `addr`.  This assumes that the
    # horizontal offsets in the string table are all zero. All
    # characters are printed on a single line (no newline).  The
    # function returns when any of the following conditions is met:
    # (1) `len` characters have been printed, (2) the next character
    # would not fit horizontally on the screen, or (3), an unprintable
    # character, i.e. not in [0x20-0x83], has been met.

    def code_printchars():
        label('_console_printchars')
        PUSH()
        CALLI('_cons_save_current_bank')
        _LDI('SYS_VDrawBits_134');STW('sysFn')
        LDW(R8);STW('sysArgs0')                  # move fgbg, freeing R8
        LDI(0);STW(R12)                          # R12: character counter
        label('.loop')
        LDW(R10);PEEK();STW(R8)                  # R8: character code
        LDI(1);ADDW(R10);STW(R10)                # next char
        LDWI(0x8000);ORW(R9);STW('sysArgs4')     # destination address
        LDW(R9);ADDI(3);STW(R9);                 # next address
        LD(vACL);SUBI(0xA0);_BGT('.ret')         # beyond screen?
        _LDI('font32up');STW(R13)                # R13: font address
        LDW(R8);SUBI(32);_BLT('.ret'  )          # c<32
        STW(R8);SUBI(50);_BLT('.draw')           # 32 <= c < 82
        STW(R8);SUBI(50);_BGE('.ret')            # >= 132
        _LDI('font82up');STW(R13)
        label('.draw')
        CALLI('_printonechar')
        LDI(1);ADDW(R12);STW(R12);               # increment counter
        XORW(R11);_BNE('.loop')                  # loop
        label('.ret')
        tryhop(4);LDW(R12);POP();RET()

    def code_printonechar():
        nohop()
        label('_printonechar')
        PUSH()
        LDW(R9);CALLI('_cons_set_bank_even')
        LDW(R8);LSLW();LSLW();ADDW(R8);ADDW(R13)
        STW(R13);LUP(0);ST('sysArgs2');SYS(134);INC('sysArgs4')
        LDI(2);ADDW(R13);LUP(0);ST('sysArgs2');SYS(134);INC('sysArgs4')
        LDI(4);ADDW(R13);LUP(0);ST('sysArgs2');SYS(134)
        LDW('sysArgs4');SUBI(2);STW('sysArgs4')
        LDW(R9);CALLI('_cons_set_bank_odd');
        LDI(1);ADDW(R13);LUP(0);ST('sysArgs2');SYS(134);INC('sysArgs4')
        LDI(3);ADDW(R13);LUP(0);ST('sysArgs2');SYS(134);INC('sysArgs4')
        LDI(0);ST('sysArgs2');SYS(134)
        CALLI('_cons_restore_saved_bank')
        tryhop(2);POP();RET()

    module(name='cons_printchar.s',
           code=[ ('EXPORT', '_console_printchars'),
                  ('IMPORT', '_cons_save_current_bank'),
                  ('IMPORT', '_cons_set_bank_even'),
                  ('IMPORT', '_cons_set_bank_odd'),
                  ('IMPORT', '_cons_restore_saved_bank'),
                  ('CODE', '_console_printchars', code_printchars),
                  ('PLACE', '_console_printchars', 0x0200, 0x7fff),
                  ('CODE', '_printonechar', code_printonechar),
                  ('PLACE', '_printonechar', 0x0200, 0x7fff) ] )
    
    # -- void _console_clear(char *addr, char clr, int nl)
    #
    # Clears from addr to the end of line with color clr.
    # Repeats for nl successive lines.

    def code_clear():
        label('_console_clear')
        PUSH()
        CALLI('_cons_save_current_bank')
        LDI(160);SUBW(R8);ST(R11)
        LD(R9);ANDI(0x3f);ST('sysArgs1')
        LDWI('SYS_SetMemory_v2_54');STW('sysFn')
        label('.loop')
        # even pixels
        LDW(R8);CALLI('_cons_set_bank_even')
        LD(R11);ST('sysArgs0')
        LDWI(0x8000);ORW(R8);STW('sysArgs2')
        SYS(54)
        # odd pixels
        LDW(R8);CALLI('_cons_set_bank_odd')
        LD(R11);ST('sysArgs0')
        LDWI(0x8000);ORW(R8);STW('sysArgs2')
        SYS(54)
        # loop
        INC(R8+1)
        LDW(R10);SUBI(1);STW(R10);
        _BNE('.loop')
        CALLI('_cons_restore_saved_bank')
        tryhop(2);POP();RET()

    module(name='cons_clear.s',
           code=[ ('EXPORT', '_console_clear'),
                  ('IMPORT', '_cons_save_current_bank'),
                  ('IMPORT', '_cons_set_bank_even'),
                  ('IMPORT', '_cons_set_bank_odd'),
                  ('IMPORT', '_cons_restore_saved_bank'),
                  ('CODE', '_console_clear', code_clear),
                  ('PLACE', '_console_clear', 0x0200, 0x7fff) ] )

    # ------------------------------------------------------------
    # HELPERS FOR CONSOLE_PRINT
    # ------------------------------------------------------------
    # Updated for high resolution screen
    # ------------------------------------------------------------

    # -- char *_console_scroll(void)

    def code_scroll(hires = True):
        nohop()
        label('_console_scroll')
        PUSH()
        # clear first line
        LDWI(v('console_info')+4)                # offset
        PEEK();INC(vACH);PEEK();ST(R8+1)
        LDI(0);ST(R8)
        LD('console_state');STW(R9)              # bg
        LDI(8);STW(R10)
        _CALLJ('_console_clear')
        # scroll videotable lines
        LDWI(v('console_info')+4);STW(R10)       # offset
        PEEK();INC(vACH);PEEK();STW(R9)
        LDWI(v('console_info')+0);DEEK()         # nlines
        SUBI(1);ST(v('console_state')+2)         # cy
        _BRA('.tst1')
        label('.loop1')
        ADDW(R10);PEEK();INC(vACH);STW(R12)
        PEEK();ST(R13)
        if not hires: LDI(7)
        if hires: LDI(3)
        _BRA('.tst2')
        label('.loop2')
        LD(R9);POKE(R12)
        INC(R9)
        if hires: INC(R9)
        INC(R12);INC(R12)
        LD(R14);SUBI(1)
        label('.tst2')
        ST(R14);_BGE('.loop2')
        LD(R13);ST(R9)
        LD(R8);SUBI(1)
        label('.tst1')
        ST(R8);_BGE('.loop1')
        tryhop(2);POP();RET()

    module(name='cons_scroll.s',
           code=[ ('EXPORT', '_console_scroll'),
                  ('IMPORT', '_console_clear'),
                  ('IMPORT', 'console_state'),
                  ('IMPORT', 'console_info'),
                  ('CODE', '_console_scroll', code_scroll) ] )


    # -- char *_console_addr(void)

    # Function _console_addr() returns the screen address of the cursor.
    # If the cursor is outside the screen it tries wrapping or scrolling.
    # Return zero if still outside the screen.
    # Warning: depends on the layout of console_state and console_info

    def code_addr(hires = True):
        nohop()
        label('_console_addr')
        PUSH()
        LD(v('console_state')+3);STW(R8)                # cx
        LDWI(v('console_info')+2);DEEK();               # ncolumns
        SUBW(R8);_BGT('.nw')
        LD(v('console_state')+5);_BEQ('.nw');           # wrapx
        INC(v('console_state')+2)                       # cy++
        LDI(0);ST(v('console_state')+3)                 # cx=0
        label('.nw')
        LD(v('console_state')+2);STW(R8)                # cy
        LDWI(v('console_info')+0);DEEK();               # nlines
        SUBW(R8);_BGT('.nh')
        LD(v('console_state')+4);_BEQ('.ret0');         # wrapy
        _CALLJ('_console_scroll')                       # spill!
        label('.nh')
        LDWI(v('console_info')+4);STW(R9)               # offset
        LD(v('console_state')+3);STW(R8)                # cx
        LDWI(v('console_info')+2);DEEK()                # ncolumns
        SUBW(R8);_BLE('.ret0')
        LDW(R8);LSLW();ADDW(R8)                         # times 6 for std
        if not hires: LSLW()                            # times 3 for hires
        STW(R10)
        LD(v('console_state')+2);ADDW(R9)               # cy + offset
        PEEK();INC(vACH);PEEK();ST(R10+1)               # page
        LDW(R10);tryhop(2);POP();RET()
        label('.ret0')
        LDI(0);tryhop(2);POP();RET()

    module(name='cons_addr.s',
           code=[ ('EXPORT', '_console_addr'),
                  ('IMPORT', '_console_scroll'),
                  ('IMPORT', 'console_state'),
                  ('IMPORT', 'console_info'),
                  ('CODE', '_console_addr', code_addr) ] )


    
scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
