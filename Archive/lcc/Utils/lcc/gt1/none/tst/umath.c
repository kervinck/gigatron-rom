// sysArg0-1 holds the arithmetic function under test
// sysArg2-3 holds the LHS of the operation
// sysArg4-5 holds the RHS of the operation
// sysArg6-7 holds the result

enum {
	ADD, SUB, LSH, RSH, MUL, DIV, MOD,
	GT, LT, NE, EQ, GE, LE
};

#ifdef TEST
// Target platform (Gigatron vCPU)

#define sysFn ((int*)0x0022)
#define sysArgsw ((unsigned*)0x0024)

#define OPT(a, b, op) \
	t = a; \
	u = b; \
	sysArgsw[1] = t; \
	sysArgsw[2] = u; \
	sysArgsw[3] = t op u; \
	__syscall(28); \
	sysArgsw[3] = t op (b); \
	__syscall(28); \

#define ADDT(a, b) OPT(a, b, +)
#define SUBT(a, b) OPT(a, b, -)
#define LSHT(a, b) OPT(a, b, <<)
#define RSHT(a, b) OPT(a, b, >>)
#define MULT(a, b) OPT(a, b, *)
#define DIVT(a, b) OPT(a, b, /)
#define MODT(a, b) OPT(a, b, %)
#define GTT(a, b) OPT(a, b, >)
#define LTT(a, b) OPT(a, b, <)
#define NET(a, b) OPT(a, b, !=)
#define EQT(a, b) OPT(a, b, ==)
#define GET(a, b) OPT(a, b, >=)
#define LET(a, b) OPT(a, b, <=)

void add() {
	unsigned t, u;

	sysArgsw[0] = ADD;
	ADDT(0, 0);
	ADDT(0, 1);
	ADDT(0, 0xffff);
	ADDT(255, 1);
	ADDT(0xffff, 1);
	ADDT(0x7fff, 1);
}

void sub() {
	unsigned t, u;

	sysArgsw[0] = SUB;
	SUBT(0, 0);
	SUBT(0, 1);
	SUBT(0, 0xffff);
	SUBT(255, 1);
	SUBT(0xffff, 1);
	SUBT(0x7fff, 1);
}

void lsh() {
	unsigned t, u;

	sysArgsw[0] = LSH;
	LSHT(1, 0);
	LSHT(1, 1);
	LSHT(1, 8);
	LSHT(0xffff, 16);
	LSHT(0xaaaa, 1);
	LSHT(0x5555, 1);
}

void rsh() {
	unsigned t, u;

	sysArgsw[0] = RSH;
	RSHT(1, 0);
	RSHT(1, 1);
	RSHT(0x100, 8);
	RSHT(0xffff, 16);
	RSHT(0xaaaa, 1);
	RSHT(0x5555, 1);
}

void mul() {
	unsigned t, u;

	sysArgsw[0] = MUL;
	MULT(1, 0);
	MULT(1, 1);
	MULT(8, 32);
	MULT(0xaa, 0x55);
	MULT(255, 255);
	MULT(256, 256);
}

void div() {
	unsigned t, u;

	sysArgsw[0] = DIV;
	DIVT(10000, 3);
	DIVT(60000, 3);
	DIVT(1972, 327);
	DIVT(1972, 65209u);
	DIVT(63564u, 65209u)
	DIVT(63564u, 327);
	DIVT(0x55*0xaa, 0x55);
	DIVT(0x7fffu, 0x8000u);
	DIVT(0x8000u, 2);
	DIVT(0x8000u, 32767);
	DIVT(0x8000u, 0x8000u);
	DIVT(0xffffu, 32767);
	DIVT(0xffffu, 0x8000u);
	DIVT(0xffffu, 0xffffu);
}

void mod() {
	unsigned t, u;

	sysArgsw[0] = MOD;
	MODT(10000, 3);
	MODT(60000, 3);
	MODT(1972, 327);
	MODT(1972, 65209u);
	MODT(63564u, 65209u)
	MODT(63564u, 327);
	MODT(0x55*0xaa, 0x55);
	MODT(0x7fffu, 0x8000u);
	MODT(0x8000u, 2);
	MODT(0x8000u, 32767);
	MODT(0x8000u, 0x8000u);
	MODT(0xffffu, 32767);
	MODT(0xffffu, 0x8000u);
	MODT(0xffffu, 0xffffu);
}

void gt() {
	unsigned t, u;
	sysArgsw[0] = GT;

	GTT(0, 0);
	GTT(0, 327);
	GTT(327, 0);
	GTT(327, 327);
	GTT(327, 1972);
	GTT(1972, 327);
	GTT(32767, 0);
	GTT(0, 32767);
	GTT(32767, 32767);
}

void lt() {
	unsigned t, u;
	sysArgsw[0] = LT;

	LTT(0, 0);
	LTT(0, 327);
	LTT(327, 0);
	LTT(327, 327);
	LTT(327, 1972);
	LTT(1972, 327);
	LTT(32767, 0);
	LTT(0, 32767);
	LTT(32767, 32767);
}

void ne() {
	unsigned t, u;
	sysArgsw[0] = NE;

	NET(0, 0);
	NET(0, 327);
	NET(327, 0);
	NET(327, 327);
	NET(1972, 327);
	NET(32767, 0);
	NET(0, 32767);
	NET(32767, 32767);
	NET(0, 65209);
	NET(65209, 0);
	NET(65209, 65209);
	NET(1972, 65209);
	NET(0xffff, 32767);
}

void eq() {
	unsigned t, u;
	sysArgsw[0] = EQ;

	EQT(0, 0);
	EQT(0, 327);
	EQT(327, 0);
	EQT(327, 327);
	EQT(1972, 327);
	EQT(32767, 0);
	EQT(0, 32767);
	EQT(32767, 32767);
	EQT(0, 65209);
	EQT(65209, 0);
	EQT(65209, 65209);
	EQT(1972, 65209);
	EQT(0xffff, 32767);
}

void ge() {
	unsigned t, u;
	sysArgsw[0] = GE;

	GET(0, 0);
	GET(0, 327);
	GET(327, 0);
	GET(327, 327);
	GET(1972, 327);
	GET(32767, 0);
	GET(0, 32767);
	GET(32767, 32767);
}

void le() {
	unsigned t, u;
	sysArgsw[0] = LE;

	LET(0, 0);
	LET(0, 327);
	LET(327, 0);
	LET(327, 327);
	LET(1972, 327);
	LET(32767, 0);
	LET(0, 32767);
	LET(32767, 32767);
}

void main() {
	*sysFn = 1;

	add();
	sub();
	lsh();
	rsh();
	mul();
	div();
	mod();
	gt();
	lt();
	ne();
	eq();
	ge();
	le();

	*sysFn = 0;
	__syscall(28);
}

#else
// Host platform (Linux/Mac/etc...)

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "harness.h"

int fail = 0;

void sys1() {
	uint16_t* sysArgsw = (uint16_t*)&RAM[0x24];

	uint16_t op = sysArgsw[0];
	uint16_t a = sysArgsw[1];
	uint16_t b = sysArgsw[2];
	uint16_t r = sysArgsw[3];

	char* opStr;
	uint16_t x;
	switch (op) {
	case ADD: opStr = "+", x = a + b; break;
	case SUB: opStr = "-", x = a - b; break;
	case LSH: opStr = "<<", x = b >= 16 ? a : a << b; break;
	case RSH: opStr = ">>", x = b >= 16 ? a : a >> b; break;
	case MUL: opStr = "*", x = a * b; break;
	case DIV: opStr = "/", x = a / b; break;
	case MOD: opStr = "%", x = a % b; break;
	case GT: opStr = ">", x = a > b; break;
	case LT: opStr = "<", x = a < b; break;
	case NE: opStr = "!=", x = a != b; break;
	case EQ: opStr = "==", x = a == b; break;
	case GE: opStr = ">=", x = a >= b; break;
	case LE: opStr = "<=", x = a <= b; break;
	default:
		fprintf(stderr, "error: unknown operation 0x%04x\n", op);
		fail = 1;
		return;
	}

	if (r != x) {
		fprintf(stderr, "error: (%hu %s %hu) == %hu, expected %hu\n", a, opStr, b, r, x);
		fail = 1;
	}
}

int main(int argc, char* argv[]) {
	char* logFile = NULL;
	if (argc > 1) {
		logFile = argv[1];
	}
	runTest("gigatron.rom", "umath.gt1", logFile, sys1);
	exit(fail ? EXIT_FAILURE : 0);
}

#endif
