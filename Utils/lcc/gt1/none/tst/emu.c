#ifdef TEST

#define NULL 0

typedef unsigned uint16_t;
typedef unsigned char uint8_t;

#define sysFn ((uint16_t*)(void*)0x0022)

void cls();
void putchar(uint8_t c);

typedef struct { // TTL state that the CPU controls
	uint16_t PC;
	uint8_t IR, D, AC, X, Y, OUT, undef;
} CpuState;

#define ROMSIZE 16
#define RAMSIZE 16

uint8_t ROM0[ROMSIZE] = {
	0x10, 0x14, 0x18, 0xfc, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};
uint8_t ROM1[ROMSIZE] = {
	0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

#define C(c) (c | 0x80)
uint8_t RAM[RAMSIZE] = {
	C('H'), C('e'), C('l'), C('l'), C('o'), C(','), C(' '), C('w'), C('o'), C('r'), C('l'), C('d'), C('!'), 0x80, 0, 0,
};

uint8_t IN=0xff;

CpuState cpuCycle(const CpuState S)
{
	int ins, mod, bus, W, J, incX, B;
	uint8_t lo, hi, *to, ALU;
	uint16_t addr;

	CpuState T = S; // New state is old state unless something changes

	T.IR = ROM0[S.PC & (ROMSIZE-1)]; // Instruction Fetch
	T.D  = ROM1[S.PC & (ROMSIZE-1)];

	ins = S.IR >> 5;       // Instruction
	mod = (S.IR >> 2) & 7; // Addressing mode (or condition)
	bus = S.IR&3;          // Busmode
	W = (ins == 6);        // Write instruction?
	J = (ins == 7);        // Jump instruction?

	lo=S.D, hi=0, *to=NULL; // Mode Decoder
	incX=0;
	if (!J)
	switch (mod) {
		#define E(p) (W?0:p) // Disable AC and OUT loading during RAM write
		case 0: to=E(&T.AC);                          break;
		case 1: to=E(&T.AC); lo=S.X;                  break;
		case 2: to=E(&T.AC);         hi=S.Y;          break;
		case 3: to=E(&T.AC); lo=S.X; hi=S.Y;          break;
		case 4: to=  &T.X;                            break;
		case 5: to=  &T.Y;                            break;
		case 6: to=E(&T.OUT);                         break;
		case 7: to=E(&T.OUT); lo=S.X; hi=S.Y; incX=1; break;
	}
	addr = (hi << 8) | lo;

	B = S.undef; // Data Bus
	switch (bus) {
	case 0: B=S.D;                        break;
	case 1: if (!W) B = RAM[(addr&0x7fff) & (RAMSIZE-1)]; break;
	case 2: B=S.AC;                       break;
	case 3: B=IN;                         break;
	}

	if (W) RAM[(addr&0x7fff) & (RAMSIZE-1)] = B; // Random Access Memory

	// Arithmetic and Logic Unit
	switch (ins) {
	case 0: ALU =        B; break; // LD
	case 1: ALU = S.AC & B; break; // ANDA
	case 2: ALU = S.AC | B; break; // ORA
	case 3: ALU = S.AC ^ B; break; // XORA
	case 4: ALU = S.AC + B; break; // ADDA
	case 5: ALU = S.AC - B; break; // SUBA
	case 6: ALU = S.AC;     break; // ST
	case 7: ALU = -S.AC;    break; // Bcc/JMP
	}

	if (to) *to = ALU; // Load value into register
	if (incX) T.X = S.X + 1; // Increment X

	T.PC = S.PC + 1; // Next instruction
	if (J) {
	if (mod != 0) { // Conditional branch within page
		int cond = (S.AC>>7) + ((S.AC==0) << 1);
		if (mod & (1 << cond)) // 74153
			T.PC = (S.PC & 0xff00) | B;
	} else
		T.PC = (S.Y << 8) | B; // Unconditional far jump
	}
	return T;
}


void main()
{
	CpuState S, T;
	int strobe;
	unsigned i;

	uint8_t* out = (uint8_t*)0x800;
	uint8_t* out2 = (uint8_t*)0x900;

	// MCP100 Power-On Reset
	S.PC = 0;
	S = cpuCycle(S);

	S.PC = 0;
	for (i = 0; i != 64; ++i) {
	T = cpuCycle(S); // Update CPU

	out[0] = T.IR;
	out[1] = T.D;
	out += 2;

	strobe = (T.OUT & 0x80) - (S.OUT & 0x80); // "write strobe"
	if (strobe > 0) {
		*out2 = T.OUT & 0x7f;
		out2++;
		if (T.OUT == 0x80) {
			*sysFn = 0;
			__syscall(28);
		}
	}

	S=T;
	}
}

#else

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "harness.h"

uint8_t trace[] = {
	0x10, 0x00,
	0x14, 0x00,
	0x18, 0x00,
	0xfc, 0x02,
	0x1d, 0x00, // write 'H'
	0x18, 0x00,
	0xfc, 0x02,
	0x1d, 0x00, // write 'e'
	0x18, 0x00,
	0xfc, 0x02,
	0x1d, 0x00, // write 'l'
	0x18, 0x00,
	0xfc, 0x02,
	0x1d, 0x00, // write 'l'
	0x18, 0x00,
	0xfc, 0x02,
	0x1d, 0x00, // write 'o'
	0x18, 0x00,
	0xfc, 0x02,
	0x1d, 0x00, // write ','
	0x18, 0x00,
	0xfc, 0x02,
	0x1d, 0x00, // write ' '
	0x18, 0x00,
	0xfc, 0x02,
	0x1d, 0x00, // write 'w'
	0x18, 0x00,
	0xfc, 0x02,
	0x1d, 0x00, // write 'o'
	0x18, 0x00,
	0xfc, 0x02,
	0x1d, 0x00, // write 'r'
	0x18, 0x00,
	0xfc, 0x02,
	0x1d, 0x00, // write 'l'
	0x18, 0x00,
	0xfc, 0x02,
	0x1d, 0x00, // write 'd'
	0x18, 0x00,
	0xfc, 0x02,
	0x1d, 0x00, // write '!'
	0x18, 0x00,
	0xfc, 0x02,
	0x1d, 0x00, // write '\0'
	0x18, 0x00,
	0xaa, 0xaa,
};

int main(int argc, char* argv[]) {
	char* logFile = NULL;
	if (argc > 1) {
		logFile = argv[1];
	}
	runTest("gigatron.rom", "emu.gt1", logFile, NULL);

	int fail = 0;

	uint8_t* out = &RAM[0x800];
	uint8_t* out2 = &RAM[0x900];
	for (int i = 0; i < sizeof(trace); i++) {
		if (trace[i] != out[i]) {
			fprintf(stderr, "mismatch at byte %d: 0x%02x != 0x%02x\n", i, trace[i], out[i]);
			fail = 1;
		}
	}

	if (strcmp((char*)out2, "Hello, world!") != 0) {
		fprintf(stderr, "expected \"Hello, world!\", got \"%s\"\n", out2);
		fail = 1;
	}

	exit(fail ? EXIT_FAILURE : 0);
}

#endif
