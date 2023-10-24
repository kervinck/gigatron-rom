

def scope():


  # ------- OUTPUT ROUTINES -------

  # - putch(int c)
  def code_putch():
    nohop()
    label('putch')
    LDW(R8);DOKE(SP) # using arg buildup zone
    LDW(SP);STW(R8)
    LDI(1);STW(R9)
    if args.cpu >= 6:
      JNE('console_print')
    else:
      PUSH();_CALLJ('console_print')
      tryhop(2);POP();RET()
    
  module(name='putch.s',
         code=[('EXPORT', 'putch'),
               ('IMPORT', 'console_print'),
               ('CODE', 'putch', code_putch) ] )


  # - cputs(const char *s)
  def code_cputs():
    nohop()
    label('cputs')
    LDWI(0x7fff);STW(R9)
    if args.cpu >= 6:
      JNE('console_print')
    else:
      PUSH();_CALLJ('console_print')
      tryhop(2);POP();RET()

  module(name='cputs.s',
         code=[('EXPORT', 'cputs'),
               ('IMPORT', 'console_print'),
               ('CODE', 'cputs', code_cputs) ] )


  # - int wherex()
  def code_wherex():
    nohop()
    label('wherex')
    LD(v('console_state')+3)
    ADDI(1);RET()

  module(name='wherex.s',
         code=[('EXPORT', 'wherex'),
               ('IMPORT', 'console_state'),
               ('CODE', 'wherex', code_wherex) ] )

  # - int wherey()
  def code_wherey():
    nohop()
    label('wherey')
    LD(v('console_state')+2)
    ADDI(1);RET()

  module(name='wherex.s',
         code=[('EXPORT', 'wherey'),
               ('IMPORT', 'console_state'),
               ('CODE', 'wherey', code_wherey) ] )


  # - void gotoxy(int x, int y)
  def code_gotoxy():
    nohop()
    label('gotoxy')
    LDW(R8);SUBI(1);ST(v('console_state')+3)
    LDW(R9);SUBI(1);ST(v('console_state')+2)
    RET()

  module(name='gotoxy.s',
         code=[('EXPORT', 'gotoxy'),
               ('IMPORT', 'console_state'),
               ('CODE', 'gotoxy', code_gotoxy) ] )


  # - void textcolor(int)
  def code_textcolor():
    nohop()
    label('textcolor')
    LD(R8);ST(v('console_state')+1)
    RET()

  module(name='textcolor.s',
         code=[('EXPORT', 'textcolor'),
               ('IMPORT', 'console_state'),
               ('CODE', 'textcolor', code_textcolor) ] )


  # - void textbackground(int)
  def code_textbackground():
    nohop()
    label('textbackground')
    LD(R8);ST(v('console_state'))
    RET()

  module(name='textbackground.s',
         code=[('EXPORT', 'textbackground'),
               ('IMPORT', 'console_state'),
               ('CODE', 'textbackground', code_textbackground) ] )


  # - void clrscr(void)
  def code_clrscr():
    nohop()
    label('clrscr')
    label('console_clear_screen')
    LDI(0);STW(v('console_state')+2)
    LD(v('console_state')+0);STW(R8)
    if args.cpu >= 6:
      JGE('_console_reset')
    else:
      PUSH();_CALLJ('_console_reset')
      tryhop(2);POP();RET()

  module(name='clrscr.s',
         code=[('EXPORT', 'clrscr'),
               ('EXPORT', 'console_clear_screen'),
               ('IMPORT', '_console_reset'),
               ('IMPORT', 'console_state'),
               ('CODE', 'clrscr', code_clrscr) ] )


  # - void clreol(void)
  def code_clreol():
    nohop()
    label('clreol')
    label('console_clear_to_eol')
    PUSH()
    _CALLJ('_console_addr');STW(R8);_BEQ('.ret')
    LD(v('console_state')+0);STW(R9)
    LDI(8);STW(R10)
    _CALLJ('_console_clear')
    label('.ret')
    tryhop(2);POP();RET()

  module(name='clreol.s',
         code=[('EXPORT', 'clreol'),
               ('EXPORT', 'console_clear_to_eol'),
               ('IMPORT', '_console_addr'),
               ('IMPORT', '_console_clear'),
               ('IMPORT', 'console_state'),
               ('CODE', 'clreol', code_clreol) ] )


  # - void cputsxy(int x, int y, const char *s)
  def code_cputsxy():
    nohop()
    label('cputsxy')
    PUSH();ALLOC(-2)
    LDW(v('console_state')+2);STLW(0)
    _CALLJ('gotoxy')
    LDW(R10);STW(R8)
    _CALLJ('cputs')
    LDLW(0);STW(v('console_state')+2)
    ALLOC(2);tryhop(2);POP();RET()

  module(name='cputsxy.s',
         code=[('EXPORT', 'cputsxy'),
               ('IMPORT', 'console_state'),
               ('IMPORT', 'gotoxy'),
               ('IMPORT', 'cputs'),
               ('CODE', 'cputsxy', code_cputsxy) ] )



  # ------- INPUT ROUTINES -------


  def code_getch_vars():
    label('getch.buf')
    bytes(0)

  module(name='getch_vars.s',
         code=[('EXPORT', 'getch.buf'),
               ('DATA', 'getch_vars', code_getch_vars, 1, 1) ] )


  # -- int kbhit(void)
  def code_kbhit():
    nohop()
    label('kbhit')
    PUSH()
    _CALLJ('kbget');_BGT('.hit')
    LDI(0)
    label('.hit')
    ST(R21)
    LDWI('getch.buf')
    if args.cpu >= 6:
      POKEA(R21);LD(R21)
    else:
      STW(R20);LD(R21);POKE(R20)
    tryhop(2);POP();RET()

  module(name='kbhit.s',
         code=[('EXPORT', 'kbhit'),
               ('IMPORT', 'getch.buf'),
               ('IMPORT', 'kbget'),
               ('CODE', 'kbhit', code_kbhit) ] )


  # -- int getch(void)
  def code_getch():
    nohop()
    label('getch')
    PUSH()
    LDWI('getch.buf');STW(R20);PEEK();ST(R21)
    LDI(0);POKE(R20);LD(R21);_BGT('.ret')
    label('.loop')
    _CALLJ('kbget');_BLT('.loop')
    label('.ret')
    tryhop(2);POP();RET()

  module(name='getch.s',
         code=[('EXPORT', 'getch'),
               ('IMPORT', 'getch.buf'),
               ('IMPORT', 'kbget'),
               ('CODE', 'getch', code_getch) ] )


  # -- void ungetch(int c)
  def code_ungetch():
    nohop()
    label('ungetch')
    LDWI('getch.buf');STW(R20)
    LD(R8+1);_BNE('.ret0')
    LD(R8);_BEQ('.ret0')
    XORI(255);_BEQ('.ret0')
    POKE(R20);RET()
    label('.ret0')
    LDI(0);POKE(R20);SUBI(1);RET()

  module(name='ungetch.s',
         code=[('EXPORT', 'ungetch'),
               ('IMPORT', 'getch.buf'),
               ('CODE', 'ungetch', code_ungetch) ] )


  # -- int getche(void)
  def code_getche():
    nohop()
    label('getche')
    PUSH();ALLOC(-4)
    LDWI('getch.buf');STW(R20);PEEK();ST(R21)
    LDI(0);POKE(R20);LD(R21);_BGT('.ret')
    label('.loop')
    _CALLJ('kbget');_BGE('.ret')
    LDI(2);STW(R9);_CALLJ('.pcursor')
    _BRA('.loop')
    label('.ret')
    STLW(2)
    LDI(3);STW(R9);_CALLJ('.pblank')
    LDLW(2);ALLOC(4)
    tryhop(2);POP();RET()
    label('.pcursor')
    LDWI(0x87f);STLW(0)
    LD('frameCount');ANDI(8);_BEQ('.p2')
    label('.pblank')
    LDWI(0x820);STLW(0)
    label('.p2')
    if args.cpu >= 7:
      LDW(vSP)
    else:
      LD(vSP)
    STW(R8);PUSH()
    _CALLJ('console_print')
    tryhop(2);POP();RET()

  module(name='getche.s',
         code=[('EXPORT', 'getche'),
               ('IMPORT', 'kbget'),
               ('IMPORT', 'getch.buf'),
               ('IMPORT', 'console_print'),
               ('CODE', 'getche', code_getche) ] )


  # -- int console_waitkey(void)
  def code_waitkey():
    nohop()
    label('console_waitkey')
    PUSH();ALLOC(-4)
    LDWI('getch.buf');STW(R20);PEEK();ST(R21)
    LDI(0);POKE(R20);LD(R21);_BGT('.ret')
    label('.loop')
    _CALLJ('kbget');_BGE('.ret')
    LDI(2);STW(R9);_CALLJ('.pcursor')
    _BRA('.loop')
    label('.ret')
    STLW(2)
    LDI(2);STW(R9);_CALLJ('.pblank')
    LDLW(2);ALLOC(4)
    tryhop(2);POP();RET()
    label('.pcursor')
    LDWI(0x87f);STLW(0)
    LD('frameCount');ANDI(8);_BEQ('.p2')
    label('.pblank')
    LDWI(0x820);STLW(0)
    label('.p2')
    if args.cpu >= 7:
      LDW(vSP)
    else:
      LD(vSP)
    STW(R8);PUSH()
    _CALLJ('console_print')
    tryhop(2);POP();RET()

  module(name='cons_waitkey.s',
         code=[('EXPORT', 'console_waitkey'),
               ('IMPORT', 'kbget'),
               ('IMPORT', 'getch.buf'),
               ('IMPORT', 'console_print'),
               ('CODE', 'console_waitkey', code_waitkey) ] )


  # -- char *cgets(char *buffer)
  def code_cgets():
    nohop()
    label('cgets')
    PUSH();ALLOC(-2)
    LDW(R8);PEEK();STW(R9)
    LDW(R8);STLW(0);ADDI(2);STW(R8)
    _CALLJ('console_readline');ST(R10)
    LDLW(0);ADDI(1);STW(R8)
    LD(R10);POKE(R8)
    LDW(R8);ADDI(1);
    ALLOC(2);tryhop(2);POP();RET()

  module(name='cgets.s',
         code=[('EXPORT','cgets'),
               ('IMPORT','console_readline'),
               ('CODE','cgets',code_cgets) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
