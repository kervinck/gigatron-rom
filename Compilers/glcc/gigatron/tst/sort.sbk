#VCPUv5

# ======== ('DATA', 'in', code0, 0, 2)
def code0():
	align(2);
	label('in');
	words(10); # 10
	words(32); # 32
	words(65535); # -1
	words(567); # 567
	words(3); # 3
	words(18); # 18
	words(1); # 1
	words(65485); # -51
	words(789); # 789
	words(0); # 0
# ======== ('CODE', 'main', code1)
def code1():
	label('main');
	_PROLOGUE(8,6,0x80); # save=R7
	LDWI('in');STW(R8);
	_MOVW(R8,[SP,2]);
	LDI(10);STW(R9);
	_MOVW(R9,[SP,4]);
	CALLI('sort');
	LDI(0);STW(R7);
	_BRA('.5');
	label('.2');
	LDWI('in');STW(R22);
	LDW(R7);LSLW();ADDW(R22);DEEK();STW(R8);
	_MOVW(R8,[SP,2]);
	CALLI('putd');
	LDI(10);STW(R8);
	_MOVW(R8,[SP,2]);
	CALLI('putchar');
	label('.3');
	LDI(1);ADDW(R7);STW(R7);
	label('.5');
	LDW(R7);_CMPIU(10);_BLT('.2');
	LDI(0);
	label('.1');
	_EPILOGUE(8,6,0x80,saveAC=True);
# ======== ('CODE', 'putd', code2)
def code2():
	label('putd');
	_PROLOGUE(8,4,0x80); # save=R7
	LDW(R8);STW(R7);
	LDW(R7);_BGE('.7');
	LDI(45);STW(R8);
	_MOVW(R8,[SP,2]);
	CALLI('putchar');
	LDI(0);SUBW(R7);STW(R7);
	label('.7');
	LDW(R7);_DIVIS(10);_BEQ('.9');
	LDW(R7);_DIVIS(10);STW(R8);
	CALLI('putd');
	label('.9');
	LDW(R7);_MODIS(10);ADDI(48);STW(R8);
	_MOVW(R8,[SP,2]);
	CALLI('putchar');
	LDI(0);
	label('.6');
	_EPILOGUE(8,4,0x80,saveAC=True);
# ======== ('CODE', 'sort', code3)
def code3():
	label('sort');
	_PROLOGUE(12,8,0xc0); # save=R6-7
	LDW(R8);STW(R7);
	LDW(R9);STW(R6);
	LDWI('xx');STW(R22);
	LDW(R7);DOKE(R22);
	LDW(R7);STW(R8);
	_MOVW(R8,[SP,2]);
	LDI(0);STW(R9);
	_MOVW(R9,[SP,4]);
	LDW(R6);SUBI(1);
	STW(R6);
	STW(R10);
	_MOVW(R10,[SP,6]);
	CALLI('quick');
	LDI(0);
	label('.11');
	_EPILOGUE(12,8,0xc0,saveAC=True);
# ======== ('CODE', 'quick', code4)
def code4():
	label('quick');
	_PROLOGUE(16,8,0xf0); # save=R4-7
	LDW(R8);STW(R7);
	LDW(R9);STW(R6);
	LDW(R10);STW(R5);
	LDW(R6);_CMPWS(R5);_BLT('.13');
	LDI(0);
	_BRA('.12');
	label('.13');
	LDW(R7);STW(R8);
	_MOVW(R8,[SP,2]);
	LDW(R6);STW(R9);
	_MOVW(R9,[SP,4]);
	LDW(R5);STW(R10);
	_MOVW(R10,[SP,6]);
	CALLI('partition');
	STW(R4);
	LDW(R7);STW(R8);
	LDW(R6);STW(R9);
	LDW(R4);SUBI(1);STW(R10);
	CALLI('quick');
	LDW(R7);STW(R8);
	LDI(1);ADDW(R4);STW(R9);
	LDW(R5);STW(R10);
	CALLI('quick');
	LDI(0);
	label('.12');
	_EPILOGUE(16,8,0xf0,saveAC=True);
# ======== ('CODE', 'partition', code5)
def code5():
	label('partition');
	_PROLOGUE(16,6,0xf8); # save=R3-7
	LDW(R8);STW(R7);
	LDW(R9);STW(R6);
	LDW(R10);STW(R5);
	LDI(1);ADDW(R5);STW(R5);
	LDW(R6);STW(R3);
	LSLW();ADDW(R7);DEEK();STW(R4);
	_BRA('.17');
	label('.16');
	LDI(1);ADDW(R6);STW(R6);
	_BRA('.20');
	label('.19');
	LDI(1);ADDW(R6);STW(R6);
	label('.20');
	LDW(R6);LSLW();ADDW(R7);DEEK();_CMPWS(R4);_BLT('.19');
	LDW(R5);SUBI(1);STW(R5);
	_BRA('.23');
	label('.22');
	LDW(R5);SUBI(1);STW(R5);
	label('.23');
	LDW(R5);LSLW();ADDW(R7);DEEK();_CMPWS(R4);_BGT('.22');
	LDW(R6);_CMPWS(R5);_BGE('.25');
	LDW(R6);LSLW();ADDW(R7);STW(R8);
	_MOVW(R8,[SP,2]);
	LDW(R5);LSLW();ADDW(R7);STW(R9);
	_MOVW(R9,[SP,4]);
	CALLI('exchange');
	label('.25');
	label('.17');
	LDW(R6);_CMPWS(R5);_BLT('.16');
	LDW(R3);LSLW();ADDW(R7);STW(R8);
	_MOVW(R8,[SP,2]);
	LDW(R5);LSLW();ADDW(R7);STW(R9);
	_MOVW(R9,[SP,4]);
	CALLI('exchange');
	LDW(R5);
	label('.15');
	_EPILOGUE(16,6,0xf8,saveAC=True);
# ======== ('CODE', 'exchange', code6)
def code6():
	label('exchange');
	_PROLOGUE(16,8,0xc0); # save=R6-7
	LDW(R8);STW(R7);
	LDW(R9);STW(R6);
	LDWI('.28');STW(R8);
	_MOVW(R8,[SP,2]);
	LDWI('xx');DEEK();STW(R22);
	LDI(2);STW(R21);
	LDW(R7);SUBW(R22);_DIVS(R21);STW(R9);
	_MOVW(R9,[SP,4]);
	LDW(R6);SUBW(R22);_DIVS(R21);STW(R10);
	_MOVW(R10,[SP,6]);
	CALLI('printf');
	_SP(-2+16);STW(R22);
	LDW(R7);DEEK();DOKE(R22);
	LDW(R6);DEEK();DOKE(R7);
	_SP(-2+16);DEEK();DOKE(R6);
	LDI(0);
	label('.27');
	_EPILOGUE(16,8,0xc0,saveAC=True);
# ======== ('COMMON', 'xx', code7, 2, 2)
def code7():
	align(2);
	label('xx');
	space(2);
# ======== ('DATA', '.28', code8, 0, 1)
def code8():
	label('.28');
	bytes(101,120,99,104,97,110,103,101);
	bytes(40,37,100,44,37,100,41,10);
	bytes(0);
# ======== (epilog)
code=[
	('EXPORT', 'in'),
	('DATA', 'in', code0, 0, 2),
	('EXPORT', 'main'),
	('CODE', 'main', code1),
	('EXPORT', 'putd'),
	('CODE', 'putd', code2),
	('EXPORT', 'sort'),
	('CODE', 'sort', code3),
	('EXPORT', 'quick'),
	('CODE', 'quick', code4),
	('EXPORT', 'partition'),
	('CODE', 'partition', code5),
	('EXPORT', 'exchange'),
	('CODE', 'exchange', code6),
	('IMPORT', 'printf'),
	('IMPORT', 'putchar'),
	('COMMON', 'xx', code7, 2, 2),
	('DATA', '.28', code8, 0, 1) ]
module(code=code, name='tst/sort.c', cpu=5);

# Local Variables:
# mode: python
# indent-tabs-mode: t
# End:
