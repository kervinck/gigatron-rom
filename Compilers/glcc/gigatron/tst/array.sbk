#VCPUv5

# ======== ('CODE', 'main', code0)
def code0():
	label('main');
	_PROLOGUE(36,6,0xe0); # save=R5-7
	LDI(0);STW(R6);
	label('.2');
	LDI(0);STW(R7);
	label('.6');
	LDWI('x');STW(R22);
	LDW(R6);_SHLI(3);ADDW(R22);STW(R22);
	LDW(R7);LSLW();ADDW(R22);STW(R22);
	LDW(R6);_MULI(1000);ADDW(R7);DOKE(R22);
	label('.7');
	LDI(1);ADDW(R7);STW(R7);
	LDW(R7);_CMPIS(4);_BLT('.6');
	LDWI('y');STW(R22);
	LDW(R6);LSLW();ADDW(R22);STW(R22);
	LDWI('x');STW(R21);
	LDW(R6);_SHLI(3);ADDW(R21);DOKE(R22);
	label('.3');
	LDI(1);ADDW(R6);STW(R6);
	LDW(R6);_CMPIS(3);_BLT('.2');
	CALLI('f');
	LDI(0);STW(R6);
	label('.10');
	_SP(-24+36);STW(R22);
	LDW(R6);_SHLI(3);ADDW(R22);STW(R22);
	STW(R5);
	LDWI('y');STW(R21);
	LDW(R6);LSLW();ADDW(R21);STW(R21);
	LDW(R22);DOKE(R21);
	LDI(0);STW(R7);
	label('.14');
	LDW(R7);LSLW();STW(R22);
	ADDW(R5);STW(R21);
	LDWI('x');STW(R20);
	LDW(R6);_SHLI(3);ADDW(R20);ADDW(R22);DEEK();DOKE(R21);
	label('.15');
	LDI(1);ADDW(R7);STW(R7);
	LDW(R7);_CMPIS(4);_BLT('.14');
	label('.11');
	LDI(1);ADDW(R6);STW(R6);
	LDW(R6);_CMPIS(3);_BLT('.10');
	_SP(-24+36);STW(R8);
	_MOVW(R8,[SP,2]);
	LDWI('y');STW(R9);
	_MOVW(R9,[SP,4]);
	CALLI('g');
	LDI(0);
	label('.1');
	_EPILOGUE(36,6,0xe0,saveAC=True);
# ======== ('CODE', 'f', code1)
def code1():
	label('f');
	_PROLOGUE(12,6,0xc0); # save=R6-7
	LDI(0);STW(R6);
	label('.19');
	LDI(0);STW(R7);
	label('.23');
	LDWI('.27');STW(R8);
	_MOVW(R8,[SP,2]);
	LDWI('x');STW(R22);
	LDW(R6);_SHLI(3);ADDW(R22);STW(R22);
	LDW(R7);LSLW();ADDW(R22);DEEK();STW(R9);
	_MOVW(R9,[SP,4]);
	CALLI('printf');
	label('.24');
	LDI(1);ADDW(R7);STW(R7);
	LDW(R7);_CMPIS(4);_BLT('.23');
	label('.20');
	LDI(1);ADDW(R6);STW(R6);
	LDW(R6);_CMPIS(3);_BLT('.19');
	LDWI('.28');STW(R8);
	_MOVW(R8,[SP,2]);
	CALLI('printf');
	LDI(0);STW(R6);
	label('.29');
	LDI(0);STW(R7);
	label('.33');
	LDWI('.27');STW(R8);
	_MOVW(R8,[SP,2]);
	LDWI('y');STW(R22);
	LDW(R6);LSLW();ADDW(R22);DEEK();STW(R22);
	LDW(R7);LSLW();ADDW(R22);DEEK();STW(R9);
	_MOVW(R9,[SP,4]);
	CALLI('printf');
	label('.34');
	LDI(1);ADDW(R7);STW(R7);
	LDW(R7);_CMPIS(4);_BLT('.33');
	label('.30');
	LDI(1);ADDW(R6);STW(R6);
	LDW(R6);_CMPIS(3);_BLT('.29');
	LDWI('.28');STW(R8);
	_MOVW(R8,[SP,2]);
	CALLI('printf');
	LDI(0);
	label('.18');
	_EPILOGUE(12,6,0xc0,saveAC=True);
# ======== ('CODE', 'g', code2)
def code2():
	label('g');
	_PROLOGUE(16,6,0xf0); # save=R4-7
	LDW(R8);STW(R7);
	LDW(R9);STW(R6);
	LDI(0);STW(R4);
	label('.38');
	LDI(0);STW(R5);
	label('.42');
	LDWI('.27');STW(R8);
	_MOVW(R8,[SP,2]);
	LDW(R4);_SHLI(3);ADDW(R7);STW(R22);
	LDW(R5);LSLW();ADDW(R22);DEEK();STW(R9);
	_MOVW(R9,[SP,4]);
	CALLI('printf');
	label('.43');
	LDI(1);ADDW(R5);STW(R5);
	LDW(R5);_CMPIS(4);_BLT('.42');
	label('.39');
	LDI(1);ADDW(R4);STW(R4);
	LDW(R4);_CMPIS(3);_BLT('.38');
	LDWI('.28');STW(R8);
	_MOVW(R8,[SP,2]);
	CALLI('printf');
	LDI(0);STW(R4);
	label('.46');
	LDI(0);STW(R5);
	label('.50');
	LDWI('.27');STW(R8);
	_MOVW(R8,[SP,2]);
	LDW(R4);LSLW();ADDW(R6);DEEK();STW(R22);
	LDW(R5);LSLW();ADDW(R22);DEEK();STW(R9);
	_MOVW(R9,[SP,4]);
	CALLI('printf');
	label('.51');
	LDI(1);ADDW(R5);STW(R5);
	LDW(R5);_CMPIS(4);_BLT('.50');
	label('.47');
	LDI(1);ADDW(R4);STW(R4);
	LDW(R4);_CMPIS(3);_BLT('.46');
	LDWI('.28');STW(R8);
	_MOVW(R8,[SP,2]);
	CALLI('printf');
	LDI(0);
	label('.37');
	_EPILOGUE(16,6,0xf0,saveAC=True);
# ======== ('COMMON', 'y', code3, 6, 2)
def code3():
	align(2);
	label('y');
	space(6);
# ======== ('COMMON', 'x', code4, 24, 2)
def code4():
	align(2);
	label('x');
	space(24);
# ======== ('DATA', '.28', code5, 0, 1)
def code5():
	label('.28');
	bytes(10,0);
# ======== ('DATA', '.27', code6, 0, 1)
def code6():
	label('.27');
	bytes(32,37,100,0);
# ======== (epilog)
code=[
	('EXPORT', 'main'),
	('CODE', 'main', code0),
	('EXPORT', 'f'),
	('CODE', 'f', code1),
	('EXPORT', 'g'),
	('CODE', 'g', code2),
	('IMPORT', 'printf'),
	('COMMON', 'y', code3, 6, 2),
	('COMMON', 'x', code4, 24, 2),
	('DATA', '.28', code5, 0, 1),
	('DATA', '.27', code6, 0, 1) ]
module(code=code, name='tst/array.c', cpu=5);

# Local Variables:
# mode: python
# indent-tabs-mode: t
# End:
