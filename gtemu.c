 
/*
 *  Gigatron simulator
 *
 *  2017-11-14 (marcelk) Initial version tested OK with ROM image
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef unsigned short word;
typedef unsigned char byte;

const int romSize = 1<<16; // 32 Kword
const int ramSize = 1<<15; // 32 KByte

typedef struct { // All TTL state that the CPU controls
  word PC;
  byte IR, D, AC, X, Y, OUT;
  byte undef;
} CpuState;

static
CpuState cpuCycle(const CpuState S, word ROM[], byte RAM[], byte IN)
{
  CpuState T = S; // New state is old state unless something changes

  // Instruction fetch
  T.IR = ROM[S.PC&(romSize-1)] & 0x00ff;
  T.D  = ROM[S.PC&(romSize-1)] >> 8;

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
      #define L(p) (W?0:p) // Don't load AC and OUT during RAM write
      case 0: to=L(&T.AC);                          break;
      case 1: to=L(&T.AC); lo=S.X;                  break;
      case 2: to=L(&T.AC);         hi=S.Y;          break;
      case 3: to=L(&T.AC); lo=S.X; hi=S.Y;          break;
      case 4: to=  &T.X;                            break;
      case 5: to=  &T.Y;                            break;
      case 6: to=L(&T.OUT);                         break;
      case 7: to=L(&T.OUT); lo=S.X; hi=S.Y; incX=1; break;
    }
  }
  int addr = (hi << 8) | lo;

  // Data Bus
  int B = S.undef;
  switch (bus) {
    case 0: B=S.D;                             break;
    case 1: if (!W) B = RAM[addr&(ramSize-1)]; break;
    case 2: B=S.AC;                            break;
    case 3: B=IN;                              break;
  }

  // Random Access Memory
  if (W)
    RAM[addr&(ramSize-1)] = B;

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
    *to = ALU; // Load value
  if (incX && to != &T.X) // 74161's precedences
    T.X = S.X + 1; // Increment

  // Condition Decoder and Program Counter
  T.PC = (S.PC + 1) & 0xffff; // Next instruction
  if (J) {
    if (mod != 0) {
      int cond = (S.AC>>7) + 2*!ALU; // 74153
      if (mod & (1 << cond))
        T.PC = (S.PC & 0xff00) | B; // Branch within page
    } else
      T.PC = (S.Y << 8) | B; // Far jump
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
  word ROM[romSize]; // 27C1024
  byte RAM[ramSize]; // 62256
  CpuState S;
  byte IN = 0xff; // Default due to pullup R19

  // Initialize with randomized data
  srand(time(NULL));
  garble((void*)ROM, sizeof ROM);
  garble((void*)RAM, sizeof RAM);
  garble((void*)&S, sizeof S);

  // Load ROM file
  const char *filename = "theloop.2.rom";
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    fprintf(stderr, "Error: failed to open ROM file %s\n", filename);
    exit(EXIT_FAILURE);
  }
  fread(ROM, 1, sizeof(ROM), fp); // Assumes little-endian host system
  if (ferror(fp)) {
    fprintf(stderr, "Error: error while reading ROM file\n");
    exit(EXIT_FAILURE);
  }
  fclose(fp);

  int vgaX = 0, vgaY = 0;
  long long t = -3;

  for (;;) {
    if (t < 0)
      S.PC = 0; // MCP100 Power-On Reset

    // Debugging
    //printf("PC %04x IR %02x D %02x AC %02x X %02x Y %02x OUT %02x\n", S.PC, S.IR, S.D, S.AC, S.X, S.Y, S.OUT);

    // Update CPU
    CpuState T = cpuCycle(S, ROM, RAM, IN);

    // Update VGA monitor
    int hSync = (T.OUT & 0x40) - (S.OUT & 0x40);
    int vSync = (T.OUT & 0x80) - (S.OUT & 0x80);
    if (vSync < 0) // Falling vSync edge
      vgaY = -35-1; // First visible line becomes 0, for convenience
    if (vgaX < 200) {
      if (hSync) putchar('|');            // Visual indicator of hSync
      else if (vgaX == 199) putchar('>'); // Too many pixels
      else if (S.OUT == 0x40) putchar('^'); // Visualize vBlank pulse
      else putchar(32 + (S.OUT & 63));    // Plot pixel
    }
    vgaX++;
    if (hSync > 0) { // Rising hSync edge
      printf("%s line %d xout %02x t %0.3f\n",
             vgaX-200 ? "~" : "", // Mark horizontal cycle errors
             vgaY, T.AC, t/6.250e+06);
      vgaX = 0;
      vgaY++;
      T.undef = rand() & 0xff; // Change this sometimes
    }

    // Advance in time
    S=T; // The clock propagates the new state
    t++;
  }
}

