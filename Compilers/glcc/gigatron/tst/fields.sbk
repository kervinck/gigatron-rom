#VCPUv5

# ======== ('DATA', 'x', code0, 8, 2)
def code0():
	align(2);
	label('x');
	words(1); # 1
	bytes(2); # 2
	space(1);
	bytes(3); # 3
	bytes(64); # 64
	bytes(80); # 80
	bytes(6); # 6
# ======== ('DATA', 'i', code1, 2, 2)
def code1():
	align(2);
	label('i');
	words(16); # 16
# ======== ('DATA', 'y', code2, 4, 2)
def code2():
	align(2);
	label('y');
	bytes(35); # 35
	space(1);
	bytes(9); # 9
	bytes(0); # 0
# ======== ('CODE', 'main', code3)
def code3():
	label('main');
	_PROLOGUE(16,16,0x0); # save=None
	LDWI('.4');STW(R8);
	_MOVW(R8,[SP,2]);
	LDWI('x');DEEK();STW(R9);
	_MOVW(R9,[SP,4]);
	LDWI(v('x')+2);PEEK();STW(R10);
	_MOVW(R10,[SP,6]);
	LDWI(v('x')+4);DEEK();_SHLI(4);_SHRIS(4);STW(R11);
	_MOVW(R11,[SP,8]);
	LDWI(v('x')+4);DEEK();_SHRIS(12);STW(R12);
	_MOVW(R12,[SP,10]);
	LDWI(v('x')+6);DEEK();_SHLI(9);_SHRIS(13);STW(R13);
	_MOVW(R13,[SP,12]);
	LDWI(v('x')+7);PEEK();STW(R14);
	_MOVW(R14,[SP,14]);
	CALLI('printf');
	LDWI('.10');STW(R8);
	_MOVW(R8,[SP,2]);
	LDWI('y');DEEK();STW(R22);
	ANDI(3);STW(R9);
	_MOVW(R9,[SP,4]);
	LDW(R22);_SHRIU(2);ANDI(15);STW(R10);
	_MOVW(R10,[SP,6]);
	LDWI(v('y')+2);DEEK();STW(R11);
	_MOVW(R11,[SP,8]);
	CALLI('printf');
	LDWI(v('x')+4);STW(R22);
	LDWI(4095);STW(R21);
	LDWI(0xf000);STW(R20);
	LDWI('i');DEEK();_SHLI(12);_SHRIS(12);_SHLI(12);ANDW(R20);STW(R20);
	LDW(R22);DEEK();ANDW(R21);ORW(R20);DOKE(R22);
	LDWI(v('x')+6);STW(R22);
	LDWI(0xff8f);STW(R21);
	LDW(R22);DEEK();ANDW(R21);DOKE(R22);
	LDWI('.4');STW(R8);
	_MOVW(R8,[SP,2]);
	LDWI('x');DEEK();STW(R9);
	_MOVW(R9,[SP,4]);
	LDWI(v('x')+2);PEEK();STW(R10);
	_MOVW(R10,[SP,6]);
	LDWI(v('x')+4);DEEK();_SHLI(4);_SHRIS(4);STW(R11);
	_MOVW(R11,[SP,8]);
	LDWI(v('x')+4);DEEK();_SHRIS(12);STW(R12);
	_MOVW(R12,[SP,10]);
	LDWI(v('x')+6);DEEK();_SHLI(9);_SHRIS(13);STW(R13);
	_MOVW(R13,[SP,12]);
	LDWI(v('x')+7);PEEK();STW(R14);
	_MOVW(R14,[SP,14]);
	CALLI('printf');
	LDWI('y');STW(R22);
	LDWI(0xfffc);STW(R21);
	LDW(R22);DEEK();ANDW(R21);ORI(2);DOKE(R22);
	LDWI(v('y')+2);STW(R22);
	LDWI('i');DEEK();DOKE(R22);
	LDWI('.10');STW(R8);
	_MOVW(R8,[SP,2]);
	LDWI('y');DEEK();STW(R22);
	ANDI(3);STW(R9);
	_MOVW(R9,[SP,4]);
	LDW(R22);_SHRIU(2);ANDI(15);STW(R10);
	_MOVW(R10,[SP,6]);
	LDWI(v('y')+2);DEEK();STW(R11);
	_MOVW(R11,[SP,8]);
	CALLI('printf');
	LDWI('x');STW(R8);
	_MOVW(R8,[SP,2]);
	CALLI('f2');
	LDI(0);
	label('.3');
	_EPILOGUE(16,16,0x0,saveAC=True);
# ======== ('CODE', 'f1', code4)
def code4():
	label('f1');
	_PROLOGUE(12,8,0x80); # save=R7
	LDW(R8);STW(R7);
	LDWI(0xffc3);STW(R22);
	LDW(R7);DEEK();ANDW(R22);DOKE(R7);
	LDWI(0xfffc);STW(R22);
	LDI(0);ANDI(3);ANDI(3);STW(R21);
	LDW(R7);DEEK();ANDW(R22);ORW(R21);DOKE(R7);
	LDW(R7);DEEK();ANDI(60);_BEQ('.22');
	LDWI('.24');STW(R8);
	_MOVW(R8,[SP,2]);
	CALLI('printf');
	label('.22');
	LDW(R7);DEEK();ORI(3);DOKE(R7);
	LDW(R7);DEEK();ORI(60);DOKE(R7);
	LDWI('.25');STW(R8);
	_MOVW(R8,[SP,2]);
	LDW(R7);DEEK();STW(R22);
	ANDI(3);STW(R9);
	_MOVW(R9,[SP,4]);
	LDW(R22);_SHRIU(2);ANDI(15);STW(R10);
	_MOVW(R10,[SP,6]);
	CALLI('printf');
	LDI(0);
	label('.21');
	_EPILOGUE(12,8,0x80,saveAC=True);
# ======== ('CODE', 'f2', code5)
def code5():
	label('f2');
	_PROLOGUE(8,4,0xc0); # save=R6-7
	LDW(R8);STW(R7);
	LDWI('i');DEEK();_BNE('.28');
	LDI(1);STW(R6);
	_BRA('.29');
	label('.28');
	LDI(0);STW(R6);
	label('.29');
	LDWI(0xfffc);STW(R22);
	LDW(R6);ANDI(3);ANDI(3);STW(R21);
	LDW(R7);DEEK();ANDW(R22);ORW(R21);DOKE(R7);
	LDW(R7);STW(R8);
	CALLI('f1');
	LDWI(0xffc3);STW(R22);
	LDI(0);ANDI(15);_SHLI(2);ANDI(60);STW(R21);
	LDW(R7);DEEK();ANDW(R22);ORW(R21);DOKE(R7);
	LDI(0);
	label('.26');
	_EPILOGUE(8,4,0xc0,saveAC=True);
# ======== ('DATA', '.25', code6, 0, 1)
def code6():
	label('.25');
	bytes(112,45,62,97,32,61,32,48);
	bytes(120,37,120,44,32,112,45,62);
	bytes(98,32,61,32,48,120,37,120);
	bytes(10,0);
# ======== ('DATA', '.24', code7, 0, 1)
def code7():
	label('.24');
	bytes(112,45,62,98,32,33,61,32);
	bytes(48,33,10,0);
# ======== ('DATA', '.10', code8, 0, 1)
def code8():
	label('.10');
	bytes(121,32,61,32,37,100,32,37);
	bytes(100,32,37,100,10,0);
# ======== ('DATA', '.4', code9, 0, 1)
def code9():
	label('.4');
	bytes(120,32,61,32,37,100,32,37);
	bytes(100,32,37,100,32,37,100,32);
	bytes(37,100,32,37,100,10,0);
# ======== (epilog)
code=[
	('EXPORT', 'x'),
	('DATA', 'x', code0, 8, 2),
	('EXPORT', 'i'),
	('DATA', 'i', code1, 2, 2),
	('EXPORT', 'y'),
	('DATA', 'y', code2, 4, 2),
	('EXPORT', 'main'),
	('CODE', 'main', code3),
	('EXPORT', 'f1'),
	('CODE', 'f1', code4),
	('EXPORT', 'f2'),
	('CODE', 'f2', code5),
	('IMPORT', 'printf'),
	('DATA', '.25', code6, 0, 1),
	('DATA', '.24', code7, 0, 1),
	('DATA', '.10', code8, 0, 1),
	('DATA', '.4', code9, 0, 1) ]
module(code=code, name='tst/fields.c', cpu=5);

# Local Variables:
# mode: python
# indent-tabs-mode: t
# End:
