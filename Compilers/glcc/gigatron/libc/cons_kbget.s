

def scope():

  def _LDSB(v):
    if args.cpu >= 7:
      LDSB(v)
    else:
      LD(v);XORI(128);SUBI(128)

  def _POKEQ(i):
    if args.cpu >= 6:
      POKEQ(i)
    else:
      STW(T2);LDI(i);POKE(T2)


  # -- int kbget(void)
  # Aliased to one of kbget[abc]()

  kbget_default = 'kbgeta'
  if 'KBGET_AUTOREPEAT' in args.opts:
    kbget_default = 'kbgetc'
  elif 'KBGET_AUTOBTN' in args.opts:
    kbget_default = 'kbgetb'

  def code_kbget():
    label('kbget', kbget_default)
    label('console_getkey', kbget_default)

  module(name='kbget.s',
         code=[('EXPORT', 'kbget'),
               ('IMPORT', kbget_default),
               ('CODE', 'kbget', code_kbget) ])

  def code_getkey():
    label('console_getkey', kbget_default)

  module(name='console_getkey.s',
         code=[('EXPORT', 'console_getkey'),
               ('IMPORT', kbget_default),
               ('CODE', 'kbget', code_kbget) ])
  
  # -- int kbgeta(void)
  def code_kbgeta():
    nohop()
    label('kbgeta')
    LDWI('.last');STW(R9)
    LD('serialRaw');ST(R8)
    label('.last', pc()+1) # next opcode arg
    XORI(0xff);_BEQ('.ret')
    LD(R8);POKE(R9);XORI(0xff);_BEQ('.ret')
    LD(R8);RET()
    label('.ret')
    SUBI(1);RET()

  module(name='kbgeta.s',
         code=[('EXPORT', 'kbgeta'),
               ('CODE', 'kbgeta', code_kbgeta) ] )



  # -- helper for kbgetb/kbgetc
  def code_kbgetx():
    nohop()
    label('kbget.sub')
    LD('buttonState');XORI(255);STW(R18)
    LD('serialRaw');STW(R19)
    LD('kbget.last');SUBI(0x7f);_BLT('.kbd')
    label('.btn')                # button?
    LDI(0xef);ANDW(R18);STW(R18)
    label('.btn1')
    LDI(0);SUBW(R18);ANDW(R18);ST(R18);_BEQ('.btn2')
    ANDI(0xef);ORW('buttonState');ST('buttonState')
    LD(R18);XORI(255);RET()
    label('.btn2')              # no!
    SUBI(1);RET()
    label('.kbd')               # keyboard?
    LD(R19);XORI(255);_BEQ('.btn2')
    LD('kbget.last');XORW(R19);_BEQ('.btn2')
    _LDSB(R19);_BLE('.btn1')
    label('.kbd2')              # typeC?
    ADDI(1);ANDW(R19);_BEQ('.btn1')
    LDWI(v('.kbd2')+1);_POKEQ(0)
    LDI(255);ST('buttonState')
    LD(R19);RET()

  def code_kbget_last():
    label('kbget.last')
    bytes(0)

  module(name='kbgetx.s',
         code=[('EXPORT', 'kbget.sub'),
               ('EXPORT', 'kbget.last'),
               ('CODE', 'kbget.sub', code_kbgetx),
               ('DATA', 'kbget.last', code_kbget_last),
               ('PLACE', 'kbget.last', 0x00, 0xff) ] )


  # -- int kbgetb(void)
  def code_kbgetb():
    nohop()
    label('kbgetb')
    PUSH()
    _CALLJ('kbget.sub');_BLT('.nok')
    ST('kbget.last')
    tryhop(2);POP();RET()       # normal return
    label('.nok')
    LD('serialRaw');XORI(255);_BNE('.ret0')
    ST('kbget.last')
    label('.ret0')              # nokey return
    _LDI(-1)
    tryhop(2);POP();RET()

  module(name='kbgetb',
         code=[('EXPORT', 'kbgetb'),
               ('IMPORT', 'kbget.sub'),
               ('IMPORT', 'kbget.last'),
               ('CODE', 'kbgetb', code_kbgetb) ] )


  # -- int kbgetc(void)
  def code_kbgetc():
    nohop()
    label('kbgetc')
    PUSH()
    _CALLJ('kbget.sub');_BLT('.nok')
    ST('kbget.last')
    LD('frameCount');ADDI(48);ST('kbget.fc')
    label('.ret1')
    LD('kbget.last')
    tryhop(2);POP();RET()       # normal return
    label('.nok')
    LD('serialRaw');XORI(255);_BNE('.rpt')
    ST('kbget.last')
    label('.ret0')
    _LDI(-1)
    tryhop(2);POP();RET()
    label('.rpt')
    LD('kbget.last');_BEQ('.ret0')
    LD('kbget.fc');STW(R20)
    LD('frameCount');SUBW(R20);_BLT('.ret0')
    LDI(8);ADDW(R20);ST('kbget.fc');_BRA('.ret1')

  def code_kbget_fc():
    label('kbget.fc')
    bytes(0)

  module(name='kbgetc',
         code=[('EXPORT', 'kbgetc'),
               ('IMPORT', 'kbget.sub'),
               ('IMPORT', 'kbget.last'),
               ('CODE', 'kbgetc', code_kbgetc),
               ('DATA', 'kbget_fc', code_kbget_fc, 1, 1),
               ('PLACE', 'kbget_fc', 0x00, 0xff) ] )

scope()

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
