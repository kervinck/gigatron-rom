#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct { // TTL state that the CPU controls
  uint16_t PC;
  uint8_t IR, D, AC, X, Y, OUT, undef;
} CpuState;

uint8_t ROM[1<<16][2], RAM[1<<15], IN=0xff;

CpuState cpuCycle(const CpuState S)
{
  CpuState T = S; // New state is old state unless something changes

  T.IR = ROM[S.PC][0]; // Instruction Fetch
  T.D  = ROM[S.PC][1];

  int ins = S.IR >> 5;       // Instruction
  int mod = (S.IR >> 2) & 7; // Addressing mode (or condition)
  int bus = S.IR&3;          // Busmode
  int W = (ins == 6);        // Write instruction?
  int J = (ins == 7);        // Jump instruction?

  uint8_t lo=S.D, hi=0, *to=NULL; // Mode Decoder
  int incX=0;
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
  uint16_t addr = (hi << 8) | lo;

  int B = S.undef; // Data Bus
  switch (bus) {
    case 0: B=S.D;                        break;
    case 1: if (!W) B = RAM[addr&0x7fff]; break;
    case 2: B=S.AC;                       break;
    case 3: B=IN;                         break;
  }

  if (W) RAM[addr&0x7fff] = B; // Random Access Memory

  uint8_t ALU; // Arithmetic and Logic Unit
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
      int cond = (S.AC>>7) + 2*(S.AC==0);
      if (mod & (1 << cond)) // 74153
        T.PC = (S.PC & 0xff00) | B;
    } else
      T.PC = (S.Y << 8) | B; // Unconditional far jump
  }
  return T;
}

void garble(uint8_t mem[], int len)
{
  for (int i=0; i<len; i++) mem[i] = 0xAA;
}

enum
{
  LDWI = 0x11,
  LD = 0x1a,
  LDW = 0x21,
  STW = 0x2b,
  BCC = 0x35,
  EQ = 0x3f,
  GT = 0x4d,
  LT = 0x50,
  GE = 0x53,
  LE = 0x56,
  LDI = 0x59,
  ST = 0x5e,
  POP = 0x63,
  NE = 0x72,
  PUSH = 0x75,
  LUP = 0x7f,
  ANDI = 0x82,
  ORI = 0x88,
  XORI = 0x8c,
  BRA = 0x90,
  INC = 0x93,
  ADDW = 0x99,
  PEEK = 0xad,
  SYS = 0xb4,
  SUBW = 0xb8,
  DEF = 0xcd,
  CALL = 0xcf,
  ALLOC = 0xdf,
  ADDI = 0xe3,
  SUBI = 0xe6,
  LSLW = 0xe9,
  STLW = 0xec,
  LDLW = 0xee,
  POKE = 0xf0,
  DOKE = 0xf3,
  DEEK = 0xf6,
  ANDW = 0xf8,
  ORW = 0xfa,
  XORW = 0xfc,
  RET = 0xff,
};

int islegalop(uint8_t* src)
{
  uint8_t op = src[0], cc = src[1];
  switch (op) {
    case LDWI:
    case LD:
    case LDW:
    case STW:
    case LDI:
    case ST:
    case POP:
    case PUSH:
    case LUP:
    case ANDI:
    case ORI:
    case XORI:
    case BRA:
    case INC:
    case ADDW:
    case PEEK:
    case SYS:
    case SUBW:
    case DEF:
    case CALL:
    case ALLOC:
    case ADDI:
    case SUBI:
    case LSLW:
    case STLW:
    case LDLW:
    case POKE:
    case DOKE:
    case DEEK:
    case ANDW:
    case ORW:
    case XORW:
    case RET:
	  return 1;
    case BCC:
      switch (cc) {
        case EQ:
        case GT:
        case LT:
        case GE:
        case LE:
        case NE:
		  return 1;
      }
  }
  return 0;
}

void disassemble(FILE* fp, uint8_t* src)
{
  // The longest vCPU instruction is three bytes. For simplicity, we'll always read three, even if we don't
  // need them all.
  uint8_t b[3];
  b[0] = src[0], b[1] = src[1], b[2] = src[2];

  // Lookup the instruction's information.
  const char* fmt;
  uint16_t op = 0;
  uint16_t size = 1;
  switch (b[0]) {
    case LDWI: fmt = "LDWI %04hX", size = 3, op = (uint16_t)b[1] | ((uint16_t)b[2] << 8); break;
    case LD: fmt = "LD %02hX", size = 2, op = b[1]; break;
    case LDW: fmt = "LDW %02hX", size = 2, op = b[1]; break;
    case STW: fmt = "STW %02hX", size = 2, op = b[1]; break;
    case LDI: fmt = "LDI %02hX", size = 2, op = b[1]; break;
    case ST: fmt = "ST %02hX", size = 2, op = b[1]; break;
    case POP: fmt = "POP", size = 1; break;
    case PUSH: fmt = "PUSH", size = 1; break;
    case LUP: fmt = "LUP %02hX", size = 2, op = b[1]; break;
    case ANDI: fmt = "ANDI %02hX", size = 2, op = b[1]; break;
    case ORI: fmt = "ORI %02hX", size = 2, op = b[1]; break;
    case XORI: fmt = "XORI %02hX", size = 2, op = b[1]; break;
    case BRA: fmt = "BRA %02hX", size = 2, op = b[1]; break;
    case INC: fmt = "INC %02hX", size = 2, op = b[1]; break;
    case ADDW: fmt = "ADDW %02hX", size = 2, op = b[1]; break;
    case PEEK: fmt = "PEEK", size = 1; break;
    case SYS: fmt = "SYS %02hX", size = 2, op = b[1]; break;
    case SUBW: fmt = "SUBW %02hX", size = 2, op = b[1]; break;
    case DEF: fmt = "DEF %02hX", size = 2, op = b[1]; break;
    case CALL: fmt = "CALL %02hX", size = 2, op = b[1]; break;
    case ALLOC: fmt = "ALLOC %02hX", size = 2, op = b[1]; break;
    case ADDI: fmt = "ADDI %02hX", size = 2, op = b[1]; break;
    case SUBI: fmt = "SUBI %02hX", size = 2, op = b[1]; break;
    case LSLW: fmt = "LSLW", size = 1; break;
    case STLW: fmt = "STLW %02hX", size = 2, op = b[1]; break;
    case LDLW: fmt = "LDLW %02hX", size = 2, op = b[1]; break;
    case POKE: fmt = "POKE %02hX", size = 2, op = b[1]; break;
    case DOKE: fmt = "DOKE %02hX", size = 2, op = b[1]; break;
    case DEEK: fmt = "DEEK", size = 1; break;
    case ANDW: fmt = "ANDW %02hX", size = 2, op = b[1]; break;
    case ORW: fmt = "ORW %02hX", size = 2, op = b[1]; break;
    case XORW: fmt = "XORW %02hX", size = 2, op = b[1]; break;
    case RET: fmt = "RET", size = 1; break;
    case BCC:
      switch (b[1]) {
        case EQ: fmt = "BEQ %02hX"; break;
        case GT: fmt = "BGT %02hX"; break;
        case LT: fmt = "BLT %02hX"; break;
        case GE: fmt = "BGE %02hX"; break;
        case LE: fmt = "BLE %02hX"; break;
        case NE: fmt = "BNE %02hX"; break;
        default: fmt = "(ILLEGAL)"; break;
      }
      size = 3;
      op = b[2];
      break;
    default:
      fmt = "(ILLEGAL)";
      break;
  }
  
  fprintf(fp, fmt, op);
}

#define sysFn 0x0022

void runTest(char *romfile, char *testfile, char *logfile)
{
  CpuState S;
  srand(time(NULL)); // Initialize with randomized data
  garble((void*)ROM, sizeof ROM);
  garble((void*)RAM, sizeof RAM);
  garble((void*)&S, sizeof S);

  FILE *fp = fopen(romfile == NULL ? "gigatron.rom" : romfile, "rb");
  if (!fp) {
    fprintf(stderr, "Error: failed to open ROM file\n");
    exit(EXIT_FAILURE);
  }
  fread(ROM, 1, sizeof ROM, fp);
  fclose(fp);

  FILE *log = NULL;
  if (logfile != NULL) {
    log = fopen(logfile, "w");
    if (!log) {
      fprintf(stderr, "Error: failed to open log file\n");
      exit(EXIT_FAILURE);
    }
  }

  // Reset the GT1, wait for the firmware to load and initialize, and wait for the vCPU interpreter loop to land
  // at a safe spot before loading the program.
  S.PC = 0; // Power-on reset
  for (long long t = 0; t < 2000000 && S.PC != 0x0300; t++) {
    S = cpuCycle(S);
  }

  // Load the GT1 file
  fp = fopen(testfile, "rb");
  if (!fp) {
    fprintf(stderr, "Error: failed to open GT1 file\n");
    exit(EXIT_FAILURE);
  }

  for (int i = 0; ; i++) {
    uint8_t ahi, alo, sz;
    fread(&ahi, 1, sizeof ahi, fp);
    if (i > 0 && ahi == 0) {
      break;
    }
    fread(&alo, 1, sizeof alo, fp);
    fread(&sz, 1, sizeof sz, fp);
    fread(&RAM[((uint16_t)ahi<<8) | (uint16_t)alo], 1, sz, fp);
    if (log) {
      fprintf(log, "Loaded segment %d of %d bytes at 0x%04x\n", i, sz, ((uint16_t)ahi<<8) | (uint16_t)alo);
    }
  }

  uint8_t shi, slo;
  fread(&shi, 1, sizeof shi, fp);
  fread(&slo, 1, sizeof slo, fp);
  if (log) {
    fprintf(log, "Entry point is 0x%04x\n", (uint16_t)slo | ((uint16_t)shi << 8));
  }
  fclose(fp);

  uint16_t start = ((uint16_t)slo | ((uint16_t)shi << 8)) - 2;
  RAM[0x0016] = start & 0xff;
  RAM[0x0017] = shi;

  // Run the loaded program and dump an execution trace. Stop when the program executes a SYS instruction with
  // sysFn set to 0 or after 187500000 cycles (30 seconds of emulated time)
  for (unsigned t = 0; t < 187500000; t++) {
    if (S.PC == 0x0308) {
      uint16_t vpc = (uint16_t)RAM[0x0016] | ((uint16_t)RAM[0x0017] << 8);
      uint16_t vac = (uint16_t)RAM[0x0018] | ((uint16_t)RAM[0x0019] << 8);
      if (log) {
        fprintf(log, "t%u: vPC: 0x%04x    vAC: 0x%04x    ", t, vpc, vac);
        disassemble(log, &RAM[vpc]);
        fprintf(log, "\n");
      }
      if (!islegalop(&RAM[vpc])) {
        fprintf(stderr, "Error: illegal instruction 0x%02x at vPC 0x%04x\n", RAM[vpc], vpc);
        exit(EXIT_FAILURE);
      }
      if (RAM[vpc] == SYS && *(uint16_t*)(&RAM[sysFn]) == 0) {
        return;
      }
    }

    S = cpuCycle(S); // Update CPU
  }

  fprintf(stderr, "Error: test timed out\n");
  exit(EXIT_FAILURE);
}
