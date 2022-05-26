

# MAC32X16:  LAC <-- T0T1 * T2 , T0T1 <-- T0T1 << 16  (clobbers T3)
def code1():
   label('__@mac32x16')
   PUSH()
   LDI(1);STW(T3);
   label('.mac2')
   ANDW(T2);_BEQ('.mac3')
   if args.cpu >= 6:
      LDI(T0);ADDLP()
   else:
      _CALLJ('__@ladd_t0t1')
   label('.mac3')
   if args.cpu >= 6:
      LDI(0);NROL(4,T0)
   else:
      _CALLJ('__@lshl1_t0t1')
   LDW(T3);LSLW();STW(T3);_BNE('.mac2')
   tryhop(2);POP();RET()
   
# LMUL:   LAC <-- LAC * [vAC]  (clobbers B0,B1, T0,T1,T2,T3)
def code2():
   label('_@_lmul')
   PUSH()
   STW(B0);DEEK();STW(T2);
   LDW(LAC);STW(T0);LDW(LAC+2);STW(T0+2);
   LDI(0);STW(LAC);STW(LAC+2);
   _CALLJ('__@mac32x16')
   LDW(B0);ADDI(2);DEEK();STW(T2);BEQ('.skip')
   _CALLJ('__@mac32x16')
   label('.skip')
   tryhop(2);POP();RET()


code= [ ('EXPORT', '_@_lmul'),
        ('IMPORT', '__@ladd_t0t1') if args.cpu < 6 else ('NOP',),
        ('IMPORT', '__@lshl1_t0t1') if args.cpu < 6 else ('NOP',),
        ('CODE', '__@mac32x16', code1),
        ('CODE', '_@_lmul', code2) ]

module(code=code, name='rt_lmul.s');

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
