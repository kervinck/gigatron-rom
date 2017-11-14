
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef unsigned short word;
typedef unsigned char byte;

typedef struct { // All TTL state that the CPU controls
  word PC;
  byte IR, D, AC, X, Y, OUT;
  byte undefined;
} State;

static
State cpuCycle(const State S, word ROM[], byte RAM[], byte IN)
{
  State T = S; // New state is old state unless something changes

  // Instruction fetch
  T.IR = ROM[S.PC] & 0x00ff;
  T.D  = ROM[S.PC] >> 8;

  // Control Unit
  int ins =  S.IR >> 5;      // Instruction
  int mod = (S.IR >> 2) & 7; // Addressing mode (or condition)
  int bus =  S.IR & 3;       // Bus mode

  // Instruction Decoder
  int W = (ins == 6); // Write instruction
  int J = (ins == 7); // Jump instructions

  // Mode Decoder
  byte lo = S.D, hi = 0, *to = NULL;
  int incX = 0;
  if (!J) {
    switch (mod) {
      #define M(p) (W?NULL:p) // No AC and OUT update during write
      case 0: to=M(&T.AC);                          break;
      case 1: to=M(&T.AC); lo=S.X;                  break;
      case 2: to=M(&T.AC);         hi=S.Y;          break;
      case 3: to=M(&T.AC); lo=S.X; hi=S.Y;          break;
      case 4: to=  &T.X;                            break;
      case 5: to=  &T.Y;                            break;
      case 6: to=M(&T.OUT);                         break;
      case 7: to=M(&T.OUT); lo=S.X; hi=S.Y; incX=1; break;
    }
  }
  int addr = (hi << 8) | lo;

  // Data Bus
  int B = S.undefined;
  switch (bus) {
    case 0: B=S.D;                        break;
    case 1: if (!W) B = RAM[addr&0x7fff]; break;
    case 2: B=S.AC;                       break;
    case 3: B=IN;                         break;
  }

  // Random Access Memory
  if (W) {
    //printf("W %04x %02x\n", addr, B);
    RAM[addr&0x7fff] = B;
  }

  // Arithmetic and Logic Unit
  byte ALU;
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

  // Registers
  if (to)
    *to = ALU;
  if (incX)
    T.X = S.X + 1; // Increment X

  // Condition Decoder and Program Counter
  T.PC = (S.PC + 1) & 0xffff; // next instruction
  if (J) {
    if (mod != 0) {
      int cond = (S.AC>>7) + 2*!ALU;
      assert(cond < 3);
      if (mod & (1 << cond))
        T.PC = (S.PC & 0xff00) | B; // branch within page
    } else
      T.PC = (S.Y << 8) | B; // far jump
  }

  return T;
}

static void garble(byte mem[], int len)
{
  for (int i=0; i<len; i++)
    mem[i] = rand();
}

int main(void)
{
  srand(time(NULL));

  word ROM[1<<16]; // 64 Kword
  garble((void*)ROM, sizeof ROM);

  FILE *fp = fopen("theloop.2.rom", "rb");
  if (!fp) {
    fprintf(stderr, "Error: failed to open ROM file\n");
    exit(EXIT_FAILURE);
  }
  fread(ROM, 1, sizeof(ROM), fp); // Assumes little-endian host system
  if (ferror(fp)) {
    fprintf(stderr, "Error: error while reading ROM file\n");
    exit(EXIT_FAILURE);
  }
  fclose(fp);

  byte RAM[1<<15]; // 32 KByte
  garble((void*)RAM, sizeof RAM);

  State S;
  garble((void*)&S, sizeof S);

  byte IN = 0xff; // Default is all-ones due to pullup R19

  int vgaX = 0, vgaY = 0;
  long long t = -3;

  for (;;) {
    if (t < 0)
      S.PC = 0; // Power-on reset circuit
    //printf("PC %04x IR %02x D %02x AC %02x X %02x Y %02x OUT %02x\n", S.PC, S.IR, S.D, S.AC, S.X, S.Y, S.OUT);
    State T = cpuCycle(S, ROM, RAM, IN);
    t++;

    // VGA monitor
    int hSync = (T.OUT & 0x40) - (S.OUT & 0x40);
    int vSync = (T.OUT & 0x80) - (S.OUT & 0x80);
    if (vSync > 0) // Rising vSync edge
      vgaY = -28;
    if (vgaX < 200) {
      if (!hSync) putchar(32 + (S.OUT & 63));
      if (hSync < 0) putchar('|');
    }
    if (hSync > 0) {
      printf("|%c line %d XOUT %02x t %0.3f\n", vgaX-200 ? '*' : ' ', vgaY, T.AC, t/6.25e6);
      vgaX = 0;
      vgaY++;
    }
    vgaX++;

    S=T; // The clock propagates the new state
  }
}

