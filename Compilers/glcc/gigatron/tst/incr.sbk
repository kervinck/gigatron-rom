#VCPUv5

# ======== ('CODE', 'main', code0)
def code0():
	label('main');
	PUSH();
	LDI(0);
	label('.1');
	tryhop(2);POP();RET()
# ======== ('CODE', 'memchar', code1)
def code1():
	label('memchar');
	_PROLOGUE(8,2,0x0); # save=None
	_SP(-4+8);DEEK();STW(R22);
	_SP(-4+8);STW(R21);
	LDI(1);ADDW(R22);DOKE(R21);
	_SP(-1+8);STW(R21);
	LDW(R22);PEEK();POKE(R21);
	_SP(-4+8);DEEK();ADDI(1);STW(R22);
	_SP(-4+8);STW(R21);
	LDW(R22);DOKE(R21);
	_SP(-1+8);STW(R21);
	LDW(R22);PEEK();POKE(R21);
	_SP(-4+8);DEEK();STW(R22);
	_SP(-4+8);STW(R21);
	LDW(R22);SUBI(-(-1));DOKE(R21);
	_SP(-1+8);STW(R21);
	LDW(R22);PEEK();POKE(R21);
	_SP(-4+8);DEEK();SUBI(-(-1));STW(R22);
	_SP(-4+8);STW(R21);
	LDW(R22);DOKE(R21);
	_SP(-1+8);STW(R21);
	LDW(R22);PEEK();POKE(R21);
	LDI(0);
	label('.2');
	_EPILOGUE(8,2,0x0,saveAC=True);
# ======== ('CODE', 'memint', code2)
def code2():
	label('memint');
	_PROLOGUE(8,2,0x0); # save=None
	_SP(-4+8);DEEK();STW(R22);
	_SP(-4+8);STW(R21);
	LDI(2);ADDW(R22);DOKE(R21);
	_SP(-2+8);STW(R21);
	LDW(R22);DEEK();DOKE(R21);
	_SP(-4+8);DEEK();ADDI(2);STW(R22);
	_SP(-4+8);STW(R21);
	LDW(R22);DOKE(R21);
	_SP(-2+8);STW(R21);
	LDW(R22);DEEK();DOKE(R21);
	_SP(-4+8);DEEK();STW(R22);
	_SP(-4+8);STW(R21);
	LDW(R22);SUBI(-(-2));DOKE(R21);
	_SP(-2+8);STW(R21);
	LDW(R22);DEEK();DOKE(R21);
	_SP(-4+8);DEEK();SUBI(-(-2));STW(R22);
	_SP(-4+8);STW(R21);
	LDW(R22);DOKE(R21);
	_SP(-2+8);STW(R21);
	LDW(R22);DEEK();DOKE(R21);
	LDI(0);
	label('.3');
	_EPILOGUE(8,2,0x0,saveAC=True);
# ======== ('CODE', 'regchar', code3)
def code3():
	label('regchar');
	PUSH();
	LDW(R18);STW(R22);
	LDI(1);ADDW(R22);STW(R18);
	LDW(R22);PEEK();STW(R19);
	LDI(1);ADDW(R18);
	STW(R18);
	PEEK();STW(R19);
	LDW(R18);STW(R22);
	SUBI(-(-1));STW(R18);
	LDW(R22);PEEK();STW(R19);
	LDW(R18);SUBI(-(-1));
	STW(R18);
	PEEK();STW(R19);
	LDI(0);
	label('.4');
	tryhop(2);POP();RET()
# ======== ('CODE', 'regint', code4)
def code4():
	label('regint');
	PUSH();
	LDW(R18);STW(R22);
	LDI(2);ADDW(R22);STW(R18);
	LDW(R22);DEEK();STW(R19);
	LDI(2);ADDW(R18);
	STW(R18);
	DEEK();STW(R19);
	LDW(R18);STW(R22);
	SUBI(-(-2));STW(R18);
	LDW(R22);DEEK();STW(R19);
	LDW(R18);SUBI(-(-2));
	STW(R18);
	DEEK();STW(R19);
	LDI(0);
	label('.5');
	tryhop(2);POP();RET()
# ======== (epilog)
code=[
	('EXPORT', 'main'),
	('CODE', 'main', code0),
	('EXPORT', 'memchar'),
	('CODE', 'memchar', code1),
	('EXPORT', 'memint'),
	('CODE', 'memint', code2),
	('EXPORT', 'regchar'),
	('CODE', 'regchar', code3),
	('EXPORT', 'regint'),
	('CODE', 'regint', code4) ]
module(code=code, name='tst/incr.c', cpu=5);

# Local Variables:
# mode: python
# indent-tabs-mode: t
# End:
