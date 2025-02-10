

# Multiplication heler
# LAC += T0T1 * T2 , T0T1 <<= 16
# Trashes T4 i.e. sysArgs[45]

def code1():
   label('__@mac32x16')
   PUSH()
   LDI(1);STW(T4);
   label('.mac2')
   ANDW(T2);_BEQ('.mac3')
   if args.cpu >= 6:
      LDI(T0);ADDL()
   else:
      _CALLJ('__@ladd_t0t1')
   label('.mac3')
   if args.cpu >= 6:
      LSLVL(T0)
   else:
      _CALLJ('__@lshl1_t0t1')
   LDW(T4);LSLW();STW(T4)
   _BNE('.mac2')
   tryhop(2);POP();RET()

# ----------------------------------------
# LMUL:   LAC <-- LAC * [vAC]
# ----------------------------------------

def code2():
   """ Long multiplication LAC * [vAC] -> LAC
       Thrashes T[0-5] sysArgs[0-7] sysFn"""
   label('_@_lmul')
   if args.cpu >= 7:
      STW(T3);ADDI(2);DEEK();MULW(LAC)
      MOVL(LAC,T0);STW(LAC);LDI(0);ST(LAX)
      LDI(1);ADDW(T3);PEEK();MACX();LDI(8);LSLXA()
      PEEKV(T3);MACX();LDI(8);LSLXA()
      RET()
   elif 'has_at67_SYS_Multiply_s16' in rominfo:
      info = rominfo['has_at67_SYS_Multiply_s16']
      addr = int(str(info['addr']),0)
      cycs = int(str(info['cycs']),0)
      PUSH()
      STW(T3);DEEK();STW(T2);
      LDW(LAC);STW(T0);LDW(LAC+2);STW(T0+2);
      LDI(0);STW(LAC);STW(LAC+2);
      _CALLJ('__@mac32x16')
      LDI(2);ADDW(T3);DEEK();STW(T0);_BEQ('.skip')
      _LDI(addr);STW('sysFn');LDW(LAC+2);STW(T4);LDI(1);STW(T5)
      SYS(cycs);STW(LAC+2)
      label('.skip')
      tryhop(2);POP();RET()
   else:
      PUSH()
      STW(T3);DEEK();STW(T2);
      LDW(LAC);STW(T0);LDW(LAC+2);STW(T0+2);
      LDI(0);STW(LAC);STW(LAC+2);
      _CALLJ('__@mac32x16')
      LDI(2);ADDW(T3);DEEK();STW(T2);_BEQ('.skip')
      _CALLJ('__@mac32x16')
      label('.skip')
      tryhop(2);POP();RET()

code= [ ('EXPORT', '_@_lmul'),
        ('IMPORT', '__@ladd_t0t1') if args.cpu < 6 else ('NOP',),
        ('IMPORT', '__@lshl1_t0t1') if args.cpu < 6 else ('NOP',),
        ('CODE', '__@mac32x16', code1) if args.cpu < 7 else ('NOP',),
        ('CODE', '_@_lmul', code2) ]

module(code=code, name='rt_lmul.s');

# Local Variables:
# mode: python
# indent-tabs-mode: ()
# End:
