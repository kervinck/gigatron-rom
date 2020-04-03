#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
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
  for (int i=0; i<len; i++) mem[i] = rand();
}

int main(int argc, char *argv[])
{
  CpuState S;
  srand(time(NULL)); // Initialize with randomized data
  garble((void*)ROM, sizeof ROM);
  garble((void*)RAM, sizeof RAM);
  garble((void*)&S, sizeof S);

  FILE *fp = fopen(argc>1 ? argv[1] : "gigatron.rom", "rb");
  if (!fp) {
    fprintf(stderr, "Error: failed to open ROM file\n");
    exit(EXIT_FAILURE);
  }
  fread(ROM, 1, sizeof ROM, fp);
  fclose(fp);

  int vgaX=0, vgaY=0;
  for (long long t=-2; ; t++) {
    if (t < 0) S.PC = 0; // MCP100 Power-On Reset

    CpuState T = cpuCycle(S); // Update CPU

    int hSync = (T.OUT & 0x40) - (S.OUT & 0x40); // "VGA monitor" (use simple stdout)
    int vSync = (T.OUT & 0x80) - (S.OUT & 0x80);
    if (vSync < 0) vgaY = -36; // Falling vSync edge
    if (vgaX++ < 200) {
      if (hSync) putchar('|');              // Visual indicator of hSync
      else if (vgaX == 200) putchar('>');   // Too many pixels
      else if (~S.OUT & 0x80) putchar('^'); // Visualize vBlank pulse
      else putchar(32 + (S.OUT & 63));      // Plot pixel
    }
    if (hSync > 0) { // Rising hSync edge
      printf("%s line %-3d xout %02x t %0.3f\n",
             vgaX!=200 ? "~" : "", // Mark horizontal cycle errors
             vgaY, T.AC, t/6.250e+06);
      vgaX = 0;
      vgaY++;
      T.undef = rand() & 0xff; // Change this once in a while
    }
    S=T;
  }
  return 0;
}
