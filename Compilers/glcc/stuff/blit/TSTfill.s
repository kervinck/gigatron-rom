#VCPUv7

# ======== ('CODE', 'rnd', code0)
def code0():
	label('rnd');
	_PROLOGUE(8,0,0xc0); # save=R6-7
	MOVW(R8,R7);
####{
	label('.8');
####	do { x = SYS_Random() & 0xff; } while (x >= m);
	CALLI('SYS_Random');
	LD(vACL);STW(R6);
	label('.9');
	LDW(R6);_CMPWU(R7);_BGE('.8');
####	return x;
	LDW(R6);
	label('.7');
	_EPILOGUE(8,0,0xc0,saveAC=True);
# ======== ('CODE', 'main', code1)
def code1():
	label('main');
	_PROLOGUE(32,8,0xff); # save=R0-7
####{
####	clrscr();
	CALLI('clrscr');
####	for(;;) {
	label('.12');
####		pse = (serialRaw != 0xff);
	LD('serialRaw');XORI(255);_BEQ('.17');
	LDI(1);_STLW(-4+32);
	_BRA('.18');
	label('.17');
	LDI(0);_STLW(-4+32);
	label('.18');
	_LDLW(-4+32);STW(R7);
####		if (pse)
	_BEQ('.19');
####			clrscr();
	CALLI('clrscr');
	label('.19');
####		c = SYS_Random() & 0x3f;
	CALLI('SYS_Random');
	ANDI(63);STW(R0);
####		x1 = rnd(160);
	MOVQW(160,R8);
	CALLI('rnd');
	STW(R6);
####		y1 = rnd(120);
	MOVQW(120,R8);
	CALLI('rnd');
	STW(R5);
####		x2 = rnd(160);
	MOVQW(160,R8);
	CALLI('rnd');
	STW(R4);
####		y2 = rnd(120);
	MOVQW(120,R8);
	CALLI('rnd');
	STW(R3);
####		if (x1 > x2) { int tmp=x1; x1=x2; x2=tmp; }
	LDW(R6);_CMPWU(R4);_BLE('.21');
	LDW(R6);_STLW(-6+32);
	MOVW(R4,R6);
	STW(R4);
	label('.21');
####		if (y1 > y2) { int tmp=y1; y1=y2; y2=tmp; }
	LDW(R5);_CMPWU(R3);_BLE('.23');
	LDW(R5);_STLW(-6+32);
	MOVW(R3,R5);
	STW(R3);
	label('.23');
####		addr = makew(y1+8,x1);
	LDW(R6);ST(R2);LDI(8);ADDW(R5);ST(v(R2)+1);
####		hw = makew(y2-y1+1,x2-x1+1);
	LDW(R4);SUBW(R6);ADDI(1);ST(R1);LDW(R3);SUBW(R5);ADDI(1);ST(v(R1)+1);
####		SYS_Fill(addr, c, hw);
	MOVW(R2,R8);
	LD(R0);STW(R9);
	MOVW(R1,R10);
	CALLI('SYS_Fill');
####		*(char*)addr = 3;
	LDI(3);POKE(R2);
####		*(char*)(addr + hw - 0x101) = 3;
	LDW(R2);ADDW(R1);ADDWI(-v(257));POKEQ(3);
	label('.25');
####			/**/;
	label('.26');
####		while (pse && serialRaw != 0xff)
	LDW(R7);_BEQ('.28');
	LD('serialRaw');XORI(255);_BNE('.25');
	label('.28');
####	}
####	for(;;) {
	_BRA('.12');
	label('.11');
	_EPILOGUE(32,8,0xff,saveAC=True);
# ======== (epilog)
code=[
	('EXPORT', 'rnd'),
	('CODE', 'rnd', code0),
	('EXPORT', 'main'),
	('CODE', 'main', code1),
	('IMPORT', 'SYS_Random'),
	('IMPORT', 'SYS_Fill'),
	('IMPORT', 'serialRaw'),
	('IMPORT', 'clrscr') ]
module(code=code, name='TSTfill.c', cpu=7);

# Local Variables:
# mode: python
# indent-tabs-mode: t
# End:
