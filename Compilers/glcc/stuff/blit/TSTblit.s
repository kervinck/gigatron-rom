#VCPUv5

# ======== ('CODE', 'bg', code0)
def code0():
	label('bg');
	_PROLOGUE(12,8,0x80); # save=R7
####{
####	clrscr();
	CALLI('clrscr');
####	SYS_Fill(makew(8,0), 0x10, makew(120,160));
	LDWI(2048);STW(R8);
	LDI(16);STW(R9);
	LDWI(30880);STW(R10);
	CALLI('SYS_Fill');
####	for (i=0; i!=120; i+=10)
	LDI(0);STW(R7);
	label('.8');
####		SYS_Fill(makew(8+i,0), 0x20, makew(5,160));
	LDI(8);ADDW(R7);ST(v(R8)+1);LDI(0);ST(R8);
	LDI(32);STW(R9);
	LDWI(1440);STW(R10);
	CALLI('SYS_Fill');
	label('.9');
####	for (i=0; i!=120; i+=10)
	LDI(10);ADDW(R7);STW(R7);
	XORI(120);_BNE('.8');
####	for (i=0; i!=160; i+=10)
	LDI(0);STW(R7);
	label('.12');
####		SYS_Fill(makew(8,i), 0x30, makew(120,1));
	LD(R7);ST(R8);LDI(hi(2048));ST(v(R8)+1);
	LDI(48);STW(R9);
	LDWI(30721);STW(R10);
	CALLI('SYS_Fill');
	label('.13');
####	for (i=0; i!=160; i+=10)
	LDI(10);ADDW(R7);STW(R7);
	XORI(160);_BNE('.12');
####}
	label('.7');
	_EPILOGUE(12,8,0x80);
# ======== ('CODE', 'rect', code1)
def code1():
	label('rect');
	_PROLOGUE(20,6,0xfc); # save=R2-7
	LDW(R8);STW(R7);
	LDW(R9);STW(R6);
	LDW(R10);STW(R5);
	LDW(R11);STW(R4);
	LDW(R12);STW(R3);
####{
####	unsigned addr = makew(8+y,x);
	LDW(R7);ST(R2);LDI(8);ADDW(R6);ST(v(R2)+1);
####	SYS_Fill(addr, c, makew(1,w));
	LDW(R2);STW(R8);
	LD(R3);STW(R9);
	LD(R5);ST(R10);LDI(hi(256));ST(v(R10)+1);
	CALLI('SYS_Fill');
####	SYS_Fill(makew(8+y+h-1,x), c, makew(1,w));
	LDW(R7);ST(R8);LDI(8);ADDW(R6);ADDW(R4);SUBI(1);ST(v(R8)+1);
	LD(R3);STW(R9);
	LD(R5);ST(R10);LDI(hi(256));ST(v(R10)+1);
	CALLI('SYS_Fill');
####	SYS_Fill(addr, c, makew(h,1));
	LDW(R2);STW(R8);
	LD(R3);STW(R9);
	LD(R4);ST(v(R10)+1);LDI(1);ST(R10);
	CALLI('SYS_Fill');
####	SYS_Fill(makew(8+y,x+w-1), c, makew(h,1));
	LDW(R7);ADDW(R5);SUBI(1);ST(R8);LDI(8);ADDW(R6);ST(v(R8)+1);
	LD(R3);STW(R9);
	LD(R4);ST(v(R10)+1);LDI(1);ST(R10);
	CALLI('SYS_Fill');
####}
	label('.16');
	_EPILOGUE(20,6,0xfc);
# ======== ('CODE', 'target', code2)
def code2():
	label('target');
	_PROLOGUE(20,10,0xf0); # save=R4-7
	LDW(R8);STW(R7);
	LDW(R9);STW(R6);
	LDW(R10);STW(R5);
	LDW(R11);STW(R4);
####{
	_BRA('.19');
	label('.18');
####	while (w > 0 && h > 0) {
####		rect(x, y, w, h, SYS_Random() & 0x3f);
	CALLI('SYS_Random');STW(R23);
	LDW(R7);STW(R8);
	LDW(R6);STW(R9);
	LDW(R5);STW(R10);
	LDW(R4);STW(R11);
	LDW(R23);ANDI(63);STW(R12);
	CALLI('rect');
####		x ++;
	LDI(1);ADDW(R7);STW(R7);
####		y ++;
	LDI(1);ADDW(R6);STW(R6);
####		w -= 2;
	LDW(R5);SUBI(2);STW(R5);
####		h -= 2;
	LDW(R4);SUBI(2);STW(R4);
####	}
	label('.19');
####	while (w > 0 && h > 0) {
	LDW(R5);_BLE('.21');
	LDW(R4);_BGT('.18');
	label('.21');
####}
	label('.17');
	_EPILOGUE(20,10,0xf0);
# ======== ('CODE', 'go', code3)
def code3():
	label('go');
	_PROLOGUE(28,14,0xfc); # save=R2-7
	LDW(R8);STW(R7);
	LDW(R9);STW(R6);
	LDW(R10);STW(R5);
	LDW(R11);STW(R4);
	LDW(R12);STW(R3);
	LDW(R13);STW(R2);
####{
####	bg();
	CALLI('bg');
####	gotoxy(1,1);
	LDI(1);STW(R8);
	STW(R9);
	CALLI('gotoxy');
####	cprintf("%dx%d+%d,%d -> %d,%d", w, h, sx, sy, dx, dy);
	LDWI('.23');STW(R8);
	LDW(R3);STW(R9);
	_STLW(2,src=R9);
	LDW(R2);STW(R10);
	_STLW(4,src=R10);
	LDW(R5);STW(R11);
	_STLW(6,src=R11);
	LDW(R4);STW(R12);
	_STLW(8,src=R12);
	LDW(R7);STW(R13);
	_STLW(10,src=R13);
	LDW(R6);STW(R14);
	_STLW(12,src=R14);
	CALLI('cprintf');
####	rect(10,10,140,100,0x3f);
	LDI(10);STW(R8);
	STW(R9);
	LDI(140);STW(R10);
	LDI(100);STW(R11);
	LDI(63);STW(R12);
	CALLI('rect');
####	target(sx, sy, w, h);
	LDW(R5);STW(R8);
	LDW(R4);STW(R9);
	LDW(R3);STW(R10);
	LDW(R2);STW(R11);
	CALLI('target');
####	SYS_Blit( makew(8+dy, dx), makew(8+sy,sx), makew(h,w));
	LDW(R7);ST(R8);LDI(8);ADDW(R6);ST(v(R8)+1);
	LDW(R5);ST(R9);LDI(8);ADDW(R4);ST(v(R9)+1);
	LDW(R3);ST(R10);LD(R2);ST(v(R10)+1);
	CALLI('SYS_Blit');
####}
	label('.22');
	_EPILOGUE(28,14,0xfc);
# ======== ('CODE', 'main', code4)
def code4():
	label('main');
	_PROLOGUE(16,14,0x0); # save=None
####{
####	go(50, 10, 10, 50, 30, 40); getch();
	LDI(50);STW(R8);
	LDI(10);STW(R9);
	STW(R10);
	LDI(50);STW(R11);
	LDI(30);STW(R12);
	LDI(40);STW(R13);
	CALLI('go');
	CALLI('getch');
####	go(50, 50, 10, 10, 11, 36); getch();
	LDI(50);STW(R8);
	STW(R9);
	LDI(10);STW(R10);
	STW(R11);
	LDI(11);STW(R12);
	LDI(36);STW(R13);
	CALLI('go');
	CALLI('getch');
####	go(50, 10, 100, 50, 32, 32); getch();
	LDI(50);STW(R8);
	LDI(10);STW(R9);
	LDI(100);STW(R10);
	LDI(50);STW(R11);
	LDI(32);STW(R12);
	STW(R13);
	CALLI('go');
	CALLI('getch');
####	go(40, 20, 50, 30, 60, 30); getch();
	LDI(40);STW(R8);
	LDI(20);STW(R9);
	LDI(50);STW(R10);
	LDI(30);STW(R11);
	LDI(60);STW(R12);
	LDI(30);STW(R13);
	CALLI('go');
	CALLI('getch');
####	go(40, 40, 50, 30, 60, 30); getch();
	LDI(40);STW(R8);
	STW(R9);
	LDI(50);STW(R10);
	LDI(30);STW(R11);
	LDI(60);STW(R12);
	LDI(30);STW(R13);
	CALLI('go');
	CALLI('getch');
####	go(60, 20, 50, 30, 60, 30); getch();
	LDI(60);STW(R8);
	LDI(20);STW(R9);
	LDI(50);STW(R10);
	LDI(30);STW(R11);
	LDI(60);STW(R12);
	LDI(30);STW(R13);
	CALLI('go');
	CALLI('getch');
####	go(60, 40, 50, 30, 60, 30); getch();
	LDI(60);STW(R8);
	LDI(40);STW(R9);
	LDI(50);STW(R10);
	LDI(30);STW(R11);
	LDI(60);STW(R12);
	LDI(30);STW(R13);
	CALLI('go');
	CALLI('getch');
####	go(20, 10, 90, 10, 60, 85); getch();
	LDI(20);STW(R8);
	LDI(10);STW(R9);
	LDI(90);STW(R10);
	LDI(10);STW(R11);
	LDI(60);STW(R12);
	LDI(85);STW(R13);
	CALLI('go');
	CALLI('getch');
####	go(100, 10, 30, 10, 60, 85); getch();
	LDI(100);STW(R8);
	LDI(10);STW(R9);
	LDI(30);STW(R10);
	LDI(10);STW(R11);
	LDI(60);STW(R12);
	LDI(85);STW(R13);
	CALLI('go');
	CALLI('getch');
####	go(40, 10, 50, 10, 60, 85); getch();
	LDI(40);STW(R8);
	LDI(10);STW(R9);
	LDI(50);STW(R10);
	LDI(10);STW(R11);
	LDI(60);STW(R12);
	LDI(85);STW(R13);
	CALLI('go');
	CALLI('getch');
####	go(60, 10, 50, 10, 60, 85); getch();
	LDI(60);STW(R8);
	LDI(10);STW(R9);
	LDI(50);STW(R10);
	LDI(10);STW(R11);
	LDI(60);STW(R12);
	LDI(85);STW(R13);
	CALLI('go');
	CALLI('getch');
####	return 0;
	LDI(0);
	label('.24');
	_EPILOGUE(16,14,0x0,saveAC=True);
# ======== ('DATA', '.23', code5, 0, 1)
def code5():
	label('.23');
	bytes(37,100,120,37,100,43,37,100);
	bytes(44,37,100,32,45,62,32,37);
	bytes(100,44,37,100,0);
# ======== (epilog)
code=[
	('EXPORT', 'bg'),
	('CODE', 'bg', code0),
	('EXPORT', 'rect'),
	('CODE', 'rect', code1),
	('EXPORT', 'target'),
	('CODE', 'target', code2),
	('EXPORT', 'go'),
	('CODE', 'go', code3),
	('EXPORT', 'main'),
	('CODE', 'main', code4),
	('IMPORT', 'SYS_Random'),
	('IMPORT', 'SYS_Blit'),
	('IMPORT', 'SYS_Fill'),
	('IMPORT', 'getch'),
	('IMPORT', 'cprintf'),
	('IMPORT', 'clrscr'),
	('IMPORT', 'gotoxy'),
	('DATA', '.23', code5, 0, 1) ]
module(code=code, name='TSTblit.c', cpu=5);

# Local Variables:
# mode: python
# indent-tabs-mode: t
# End:
