#VCPUv5

# ======== ('CODE', 'addpoint', code0)
def code0():
	label('addpoint');
	PUSH();
	_SP(4+0);STW(R22);
	_SP(8+0);DEEK();STW(R21);
	_SP(4+0);DEEK();ADDW(R21);DOKE(R22);
	_SP(6+0);STW(R22);
	_SP(10+0);DEEK();STW(R21);
	_SP(6+0);DEEK();ADDW(R21);DOKE(R22);
	LDW(R8);STW(T2);_MOVM([SP,4+0],[T2],4,2);
	label('.1');
	tryhop(2);POP();RET()
# ======== ('CODE', 'canonrect', code1)
def code1():
	label('canonrect');
	_PROLOGUE(12,2,0x0); # save=None
	_SP(8+12);DEEK();STW(R22);
	_SP(4+12);DEEK();_CMPWS(R22);_BGE('.10');
	_SP(4+12);DEEK();STW(R19);
	_BRA('.11');
	label('.10');
	_SP(8+12);DEEK();STW(R19);
	label('.11');
	_SP(-8+12);STW(R22);
	LDW(R19);DOKE(R22);
	_SP(10+12);DEEK();STW(R22);
	_SP(6+12);DEEK();_CMPWS(R22);_BGE('.20');
	_SP(6+12);DEEK();STW(R18);
	_BRA('.21');
	label('.20');
	_SP(10+12);DEEK();STW(R18);
	label('.21');
	_SP(-6+12);STW(R22);
	LDW(R18);DOKE(R22);
	_SP(8+12);DEEK();STW(R22);
	_SP(4+12);DEEK();_CMPWS(R22);_BLE('.26');
	_SP(4+12);DEEK();STW(R17);
	_BRA('.27');
	label('.26');
	_SP(8+12);DEEK();STW(R17);
	label('.27');
	_SP(-4+12);STW(R22);
	LDW(R17);DOKE(R22);
	_SP(10+12);DEEK();STW(R22);
	_SP(6+12);DEEK();_CMPWS(R22);_BLE('.37');
	_SP(6+12);DEEK();STW(R16);
	_BRA('.38');
	label('.37');
	_SP(10+12);DEEK();STW(R16);
	label('.38');
	_SP(-2+12);STW(R22);
	LDW(R16);DOKE(R22);
	LDW(R8);STW(T2);_MOVM([SP,-8+12],[T2],8,2);
	label('.5');
	_EPILOGUE(12,2,0x0);
# ======== ('CODE', 'makepoint', code2)
def code2():
	label('makepoint');
	_PROLOGUE(8,2,0x0); # save=None
	_SP(-4+8);STW(R22);
	LDW(R9);DOKE(R22);
	_SP(-2+8);STW(R22);
	LDW(R10);DOKE(R22);
	LDW(R8);STW(T2);_MOVM([SP,-4+8],[T2],4,2);
	label('.39');
	_EPILOGUE(8,2,0x0);
# ======== ('CODE', 'makerect', code3)
def code3():
	label('makerect');
	_PROLOGUE(24,12,0x80); # save=R7
	LDW(R8);STW(R7);
	_SP(-8+24);STW(T2);_MOVM([SP,4+24],[T2],4,2);
	_SP(-4+24);STW(T2);_MOVM([SP,8+24],[T2],4,2);
	LDW(R7);STW(R8);
	_SP(4);STW(T2);_MOVM([SP,-8+24],[T2],8,2);
	CALLI('canonrect');
	label('.42');
	_EPILOGUE(24,12,0x80);
# ======== ('CODE', 'ptinrect', code4)
def code4():
	label('ptinrect');
	PUSH();
	_SP(2+0);DEEK();STW(R22);
	_SP(6+0);DEEK();_CMPWS(R22);_BGT('.53');
	_SP(10+0);DEEK();_CMPWS(R22);_BLE('.53');
	_SP(8+0);DEEK();STW(R22);
	_SP(4+0);DEEK();_CMPWS(R22);_BLT('.53');
	_SP(12+0);DEEK();STW(R22);
	_SP(4+0);DEEK();_CMPWS(R22);_BGE('.53');
	LDI(1);STW(R19);
	_BRA('.54');
	label('.53');
	LDI(0);STW(R19);
	label('.54');
	LDW(R19);
	label('.45');
	tryhop(2);POP();RET()
# ======== ('DATA', 'y', code5, 3, 1)
def code5():
	label('y');
	bytes(97,98);
	bytes(0);
# ======== ('CODE', 'odd', code6)
def code6():
	label('odd');
	_PROLOGUE(12,6,0x0); # save=None
	_SP(-3+12);STW(T2);_MOVM([SP,2+12],[T2],3,1);
	LDWI('.56');STW(R8);
	_MOVW(R8,[SP,2]);
	_SP(-3+12);STW(R9);
	_MOVW(R9,[SP,4]);
	CALLI('printf');
	LDI(0);
	label('.55');
	_EPILOGUE(12,6,0x0,saveAC=True);
# ======== ('DATA', '.58', code7, 0, 2)
def code7():
	align(2);
	label('.58');
	words(0); # 0
	words(0); # 0
# ======== ('DATA', '.59', code8, 0, 2)
def code8():
	align(2);
	label('.59');
	words(320); # 320
	words(320); # 320
# ======== ('DATA', '.60', code9, 0, 2)
def code9():
	align(2);
	label('.60');
	words(65535); # -1
	words(65535); # -1
	words(1); # 1
	words(1); # 1
	words(20); # 20
	words(300); # 300
	words(500); # 500
	words(400); # 400
# ======== ('CODE', 'main', code10)
def code10():
	label('main');
	_PROLOGUE(68,14,0x80); # save=R7
	_MOVM('.58',[SP,-32+68],4,2);
	_MOVM('.59',[SP,-36+68],4,2);
	_MOVM('.60',[SP,-16+68],16,2);
	_SP(-40+68);STW(R8);
	_LDI(-10);STW(R22);
	STW(R9);
	LDW(R22);STW(R10);
	_MOVW(R10,[SP,6]);
	CALLI('makepoint');
	_SP(-44+68);STW(R8);
	_SP(4);STW(T2);_MOVM([SP,-36+68],[T2],4,2);
	_SP(8);STW(T2);_MOVM([SP,-40+68],[T2],4,2);
	CALLI('addpoint');
	_SP(-48+68);STW(R8);
	LDI(10);STW(R9);
	LDI(10);STW(R10);
	_MOVW(R10,[SP,6]);
	CALLI('makepoint');
	_SP(-52+68);STW(R8);
	_SP(4);STW(T2);_MOVM([SP,-32+68],[T2],4,2);
	_SP(8);STW(T2);_MOVM([SP,-48+68],[T2],4,2);
	CALLI('addpoint');
	_SP(-24+68);STW(R8);
	_SP(4);STW(T2);_MOVM([SP,-44+68],[T2],4,2);
	_SP(8);STW(T2);_MOVM([SP,-52+68],[T2],4,2);
	CALLI('makerect');
	LDI(0);STW(R7);
	_BRA('.64');
	label('.61');
	_SP(-28+68);STW(R8);
	LDW(R7);_SHLI(2);STW(R22);
	_SP(-16+68);ADDW(R22);DEEK();STW(R9);
	_SP(-14+68);ADDW(R22);DEEK();STW(R10);
	_MOVW(R10,[SP,6]);
	CALLI('makepoint');
	LDWI('.65');STW(R8);
	_MOVW(R8,[SP,2]);
	_SP(-16+68);STW(R22);
	LDW(R7);_SHLI(2);ADDW(R22);DEEK();STW(R9);
	_MOVW(R9,[SP,4]);
	_SP(-26+68);DEEK();STW(R10);
	_MOVW(R10,[SP,6]);
	CALLI('printf');
	_SP(2);STW(T2);_MOVM([SP,-28+68],[T2],4,2);
	_SP(6);STW(T2);_MOVM([SP,-24+68],[T2],8,2);
	CALLI('ptinrect');
	_BNE('.68');
	LDWI('.70');STW(R8);
	_MOVW(R8,[SP,2]);
	CALLI('printf');
	label('.68');
	LDWI('.71');STW(R8);
	_MOVW(R8,[SP,2]);
	_SP(-24+68);DEEK();STW(R9);
	_MOVW(R9,[SP,4]);
	_SP(-22+68);DEEK();STW(R10);
	_MOVW(R10,[SP,6]);
	_SP(-20+68);DEEK();STW(R11);
	_MOVW(R11,[SP,8]);
	_SP(-18+68);DEEK();STW(R12);
	_MOVW(R12,[SP,10]);
	CALLI('printf');
	label('.62');
	LDI(1);ADDW(R7);STW(R7);
	label('.64');
	LDW(R7);_CMPIU(4);_BLT('.61');
	_SP(2);STW(T2);_MOVM('y',[T2],3,1);
	CALLI('odd');
	LDI(0);STW(R8);
	_MOVW(R8,[SP,2]);
	CALLI('exit');
	LDI(0);
	label('.57');
	_EPILOGUE(68,14,0x80,saveAC=True);
# ======== ('DATA', '.71', code11, 0, 1)
def code11():
	label('.71');
	bytes(119,105,116,104,105,110,32,91);
	bytes(37,100,44,37,100,59,32,37);
	bytes(100,44,37,100,93,10,0);
# ======== ('DATA', '.70', code12, 0, 1)
def code12():
	label('.70');
	bytes(110,111,116,32,0);
# ======== ('DATA', '.65', code13, 0, 1)
def code13():
	label('.65');
	bytes(40,37,100,44,37,100,41,32);
	bytes(105,115,32,0);
# ======== ('DATA', '.56', code14, 0, 1)
def code14():
	label('.56');
	bytes(37,115,10,0);
# ======== (epilog)
code=[
	('EXPORT', 'addpoint'),
	('CODE', 'addpoint', code0),
	('EXPORT', 'canonrect'),
	('CODE', 'canonrect', code1),
	('EXPORT', 'makepoint'),
	('CODE', 'makepoint', code2),
	('EXPORT', 'makerect'),
	('CODE', 'makerect', code3),
	('EXPORT', 'ptinrect'),
	('CODE', 'ptinrect', code4),
	('EXPORT', 'y'),
	('DATA', 'y', code5, 3, 1),
	('EXPORT', 'odd'),
	('CODE', 'odd', code6),
	('DATA', '.58', code7, 0, 2),
	('DATA', '.59', code8, 0, 2),
	('DATA', '.60', code9, 0, 2),
	('EXPORT', 'main'),
	('CODE', 'main', code10),
	('IMPORT', 'exit'),
	('IMPORT', 'printf'),
	('DATA', '.71', code11, 0, 1),
	('DATA', '.70', code12, 0, 1),
	('DATA', '.65', code13, 0, 1),
	('DATA', '.56', code14, 0, 1) ]
module(code=code, name='tst/struct.c', cpu=5);

# Local Variables:
# mode: python
# indent-tabs-mode: t
# End:
