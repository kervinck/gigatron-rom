#VCPUv5

# ======== ('CODE', 'main', code0)
def code0():
	label('main');
	_PROLOGUE(28,10,0xf8); # save=R3-7
	LDW(R8);STW(R7);
	LDW(R9);STW(R6);
	LDW(R7);_CMPIS(1);_BGT('.2');
	_MOVF('.4',[SP,-5+28]);
	_BRA('.3');
	label('.2');
	LDI(2);ADDW(R6);DEEK();STW(R8);
	_MOVW(R8,[SP,2]);
	CALLI('atof');
	LDWI('.5');_FDIV();_MOVF(FAC,[SP,-5+28]);
	label('.3');
	LDI(0);STW(R5);
	label('.6');
	LDW(R5);STW(R22);
	LDI(1);ADDW(R22);STW(R5);
	LDWI('f');STW(R21);
	LDW(R22);LSLW();LSLW();ADDW(R22);ADDW(R21);STW(T2);_MOVF('.4',[T2]);
	label('.7');
	LDW(R5);_CMPIS(127);_BLE('.6');
	LDI(0);STW(R4);
	_BRA('.11');
	label('.10');
	LDWI('f');STW(R22);
	LDW(R3);LSLW();LSLW();ADDW(R3);ADDW(R22);STW(R22);
	_MOVF('.13',FAC);LDW(R22);_FADD();LDW(R22);_MOVF(FAC,[vAC]);
	LDI(1);ADDW(R4);STW(R4);
	label('.11');
	CALLI('getchar');STW(R22);
	STW(R3);
	_LDI(-1);XORW(R22);_BNE('.10');
	LDWI('.14');STW(R8);
	_MOVW(R8,[SP,2]);
	CALLI('printf');
	LDI(0);STW(R5);
	label('.15');
	LDWI('f');STW(R22);
	LDW(R5);LSLW();LSLW();ADDW(R5);ADDW(R22);_MOVF([vAC],F20);
	_MOVF(F20,FAC);LDWI('.4');_FCMP();_BEQ('.19');
	LDW(R4);_STLS(LAC);_FCVI();_MOVF(FAC,F17);
	_MOVF(F20,FAC);LDI(F17);_FDIV();_SP(-5+28);_FCMP();_BLT('.19');
	LDW(R5);_CMPIS(32);_BGT('.21');
	LDWI('.23');STW(R8);
	_MOVW(R8,[SP,2]);
	LDW(R5);STW(R9);
	_MOVW(R9,[SP,4]);
	CALLI('printf');
	_BRA('.22');
	label('.21');
	LDWI('.24');STW(R8);
	_MOVW(R8,[SP,2]);
	LDW(R5);STW(R9);
	_MOVW(R9,[SP,4]);
	CALLI('printf');
	label('.22');
	LDWI('.25');STW(R8);
	_MOVW(R8,[SP,2]);
	LDW(R5);LSLW();LSLW();ADDW(R5);STW(R22);
	LDW(R4);_STLS(LAC);_FCVI();_MOVF(FAC,F19);
	_MOVF('.5',FAC);LDWI('f');ADDW(R22);_FMUL();LDI(F19);_FDIV();_MOVF(FAC,F9);
	_MOVF(F9,[SP,4]);
	CALLI('printf');
	label('.19');
	label('.16');
	LDI(1);ADDW(R5);STW(R5);
	LDW(R5);_CMPIS(127);_BLE('.15');
	LDI(0);
	label('.1');
	_EPILOGUE(28,10,0xf8,saveAC=True);
# ======== ('COMMON', 'f', code1, 640, 1)
def code1():
	label('f');
	space(640);
# ======== ('DATA', '.25', code2, 0, 1)
def code2():
	label('.25');
	bytes(9,37,46,49,102,10,0);
# ======== ('DATA', '.24', code3, 0, 1)
def code3():
	label('.24');
	bytes(37,99,0);
# ======== ('DATA', '.23', code4, 0, 1)
def code4():
	label('.23');
	bytes(37,48,51,111,0);
# ======== ('DATA', '.14', code5, 0, 1)
def code5():
	label('.14');
	bytes(99,104,97,114,9,102,114,101);
	bytes(113,10,0);
# ======== ('DATA', '.13', code6, 0, 1)
def code6():
	label('.13');
	bytes(129,0,0,0,0); # 1
# ======== ('DATA', '.5', code7, 0, 1)
def code7():
	label('.5');
	bytes(135,72,0,0,0); # 100
# ======== ('DATA', '.4', code8, 0, 1)
def code8():
	label('.4');
	bytes(0,0,0,0,0); # 0
# ======== (epilog)
code=[
	('EXPORT', 'main'),
	('CODE', 'main', code0),
	('IMPORT', 'printf'),
	('IMPORT', 'getchar'),
	('IMPORT', 'atof'),
	('COMMON', 'f', code1, 640, 1),
	('DATA', '.25', code2, 0, 1),
	('DATA', '.24', code3, 0, 1),
	('DATA', '.23', code4, 0, 1),
	('DATA', '.14', code5, 0, 1),
	('DATA', '.13', code6, 0, 1),
	('DATA', '.5', code7, 0, 1),
	('DATA', '.4', code8, 0, 1) ]
module(code=code, name='tst/cf.c', cpu=5);

# Local Variables:
# mode: python
# indent-tabs-mode: t
# End:
