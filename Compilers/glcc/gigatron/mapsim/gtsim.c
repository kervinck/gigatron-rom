#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>

#ifndef WIN32
# include <unistd.h>
# include <fcntl.h>
# include <signal.h>
#else
# include <io.h>
# include <fcntl.h>
# define read _read
# define write _write
# define lseek _lseek
# define off_t long
# define close _close
# define fileno _fileno
# define dup2 _dup2
#endif


typedef struct cpustate_s CpuState;

void sys_0x3b4(CpuState*);
void next_0x301(CpuState*);

char *rom = 0;
char *gt1 = 0;
int nogt1 = 0;
const char *trace = 0;
int verbose = 0;
int okopen = 0;
int vmode = 1975;
const char *prof = 0;
long long *pc2cycs = 0;
const int alignlong = 4;

void debug(const char *fmt, ...)
{
  if (verbose) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
  }
}

#ifdef _MSC_VER
__declspec(noreturn)
#endif
void fatal(const char *fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  va_end(ap);
  exit(EXIT_FAILURE);
}


/* ----------------------------------------------- */
/* MARCEL'S EMULATOR                               */
/* ----------------------------------------------- */


struct cpustate_s { // TTL state that the CPU controls
  uint16_t PC;
  uint8_t IR, D, AC, X, Y, OUT, undef;
};

uint8_t ROM[1<<16][2], RAM[1<<16], IN=0xff;
uint16_t opc;
uint8_t  pfx;
uint8_t  cpuselect;
long long ot, vt;
long long t;

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
    case 1: if (!W) B = RAM[addr];        break;
    case 2: B=S.AC;                       break;
    case 3: B=IN;                         break;
  }

  if (W) RAM[addr] = B; // Random Access Memory

  uint8_t ALU = 0; // Arithmetic and Logic Unit
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

void sim(void)
{
  int vgaX = 0;
  int vgaY = 0;
  uint16_t cpupage;
  CpuState S;

  memset(&S, 0, sizeof(S));

  for(t = -2; ; t++)
    {
      // reset
      if (t < 0)
        S.PC = 0;

      // cycle
      CpuState T = cpuCycle(S); 

      // vga timing check
      int hSync = (T.OUT & 0x40) - (S.OUT & 0x40);
      int vSync = (T.OUT & 0x80) - (S.OUT & 0x80);
      if (vSync < 0) 
        vgaY = -36;
      vgaX++;
      if (hSync > 0) {
        if (vgaX != 200 && t >= 6250000)
          fprintf(stderr, "(gtsim) Horizontal timing error:"
                  "vgaY %-3d, vgaX %-3d, t=%0.3f\n", vgaY, vgaX, t/6.25e6);
        vgaX = 0;
        vgaY++;
      }

      // callbacks
      if (S.PC == 0x3b4) {
        if (T.AC != 0x80)
          sys_0x3b4(&T);
        else {
          printf("HALT\n");
          exit(10);
        }
      } else if (S.PC == 0x301) {
        next_0x301(&T);
      } else if (S.PC == 0x307) {
        pfx = *(uint8_t*)(RAM+0x02);
        opc = *(uint16_t*)(RAM+0x16);
        ot = t - 6;
      } else {
        uint16_t cpupage = (RAM[2]+1)<<8;
        if (pfx == 2 && S.PC == (cpupage | 0x10)) {
          vt = t - 8;
          pfx = 1;
        } else if (pfx == 1 && S.PC == (cpupage | 0x01)) {
          ot += t - vt;
          pfx = 2;
        }
      }
      // commit
      S = T;
    }
}



/* ----------------------------------------------- */
/* WELL-KNOWN LOCATIONS                            */
/* ----------------------------------------------- */


typedef uint16_t   word;
typedef uint8_t    byte;
typedef uint32_t   quad;

typedef int16_t   sword;
typedef int8_t    sbyte;
typedef int32_t   squad;

word peek(word a) {
  return RAM[a & 0xffff];
}

void poke(word a, quad x) {
  RAM[a & 0xffff] = (x & 0xff);
}

word deek(word a) {
  if ((a & 0xff) > 0xfe)
    fprintf(stderr, "(gtsim) deek crosses page boundary\n");
  return (word)RAM[a & 0xffff] | (word)(RAM[(a+1) & 0xffff] << 8);
}

void doke(word a, quad x) {
  if ((a & 0xff) > 0xfe)
    fprintf(stderr, "(gtsim) doke crosses page boundary\n");
  RAM[a & 0xffff] = (x & 0xff);
  RAM[(a+1) & 0xffff] = ((x >> 8) & 0xff);
}

quad leek(word a) {
  if ((a & 0xff) > 0xfe)
    fprintf(stderr, "(gtsim) leek crosses page boundary\n");
  return ((quad)RAM[a & 0xffff] | ((quad)RAM[(a+1) & 0xffff]<<8) |
          ((quad)RAM[(a+2) & 0xffff]<<16) | ((quad)RAM[(a+3) & 0xffff]<<24) );
}

void loke(word a, quad x) {
  doke(a, x & 0xffff);
  doke(a+2, (x>>16) & 0xffff);
}

double feek(word a) {
  int exp = RAM[a & 0xffff];
  quad mant = ((quad)RAM[(a+4) & 0xffff] | ((quad)RAM[(a+3) & 0xffff]<<8) |
               ((quad)RAM[(a+2) & 0xffff]<<16) | ((quad)(RAM[(a+1) & 0xffff])<<24) );
  double sign = (mant & 0x80000000UL) ? -1 : +1;
  if (exp)
    return sign * ldexp((double)(mant|0x80000000UL)/0x100000000UL, exp-128);
  return 0;
}

/* Register base. Default is now 0x40.  
   Code that does not advertise regbase with
   SYS_regbase is assumed to use the old value 0x80.
*/

unsigned int regbase = 0x80;

#define vPC       (0x16)
#define vAC       (0x18)
#define vLR       (0x1a)
#define vSP       (0x1c)
#define sysFn     (0x22)
#define sysArgs0  (0x24+0)
#define B0        (0xc1)
#define LAC       (0xc4)
#define T0        (0xc8)
#define R0        (regbase+0)
#define R8        (regbase+16)
#define R9        (regbase+18)
#define R10       (regbase+20)
#define R11       (regbase+22)
#define R12       (regbase+24)
#define SP        (regbase+46)

#define addlo(a,i)  (((a)&0xff00)|(((a)+i)&0xff))


/* ----------------------------------------------- */
/* PROFILE                                         */
/* ----------------------------------------------- */

void save_profile(void)
{
  FILE *f;
  int i;
  const char *comma = "";
  long long sum = 0;
  if (! (prof && pc2cycs))
    return;
  if (! ((f = fopen(prof,"w")))) {
    fprintf(stderr, "Cannot open profile file '%s'\n", prof);
    return;
  }
  fprintf(f, "prof={\n");
  fprintf(f, (" # pc: cycs  Number of cycles spent on vCPU\n"
              " #           instructions at address less than pc"));
  for (i = 0; i < 0x10000; i++)
    if (pc2cycs[i]) {
      fprintf(f, "%s\n 0x%04x: %lld", comma, i, sum += pc2cycs[i]);
      comma = ",";
    }
  fprintf(f, "\n}\n");
  fclose(f);
}

void setup_profile(const char *f)
{
  prof = f;
  pc2cycs = calloc(0x10000, sizeof(long long));
  atexit(save_profile);
#ifndef WIN32
  signal(SIGINT, (void*)exit);
#endif  

}

/* ----------------------------------------------- */
/* CAPTURING SYS CALLS                             */
/* ----------------------------------------------- */

#define SYS_Exec_88       0x00ad
#define SYS_SetMode_v2_80 0x0b00
          

void debugSysFn(void)
{
  debug("SysFn=$%04x SysArgs=", deek(sysFn));
  for (int i=0,c='['; i<8; i++, c=' ')
    debug("%c%02x", c, peek(sysArgs0+i));
  debug("]");
}

word loadGt1(const char *gt1)
{
  int c;
  word addr = 0;
  int len;
  FILE *fp = fopen(gt1, "rb");
  if (! fp)
    fatal("Cannot open file '%s'\n", gt1);
  c = getc(fp);
  do
    {
      // high address byte
      if (c < 0)
        goto eof;
      addr = (c & 0xff) << 8;
      // low address byte
      if ((c = getc(fp)) < 0)
        goto eof;
      addr |= (c & 0xff);
      // length
      if ((c = getc(fp)) < 0)
        goto eof;
      len = (c == 0) ? 256 : (c & 0xff);
      // segment
      while (--len >= 0)
        {
          if ((c = getc(fp)) < 0)
            goto eof;
          poke(addr, c&0xff);
          addr = (addr & 0xff00) | ((addr+1) & 0xff);
        }
      // next high address byte
      if ((c = getc(fp)) < 0)
        goto eof;
    }
  while (c > 0);
  // high start byte
  if ((c = getc(fp)) < 0)
    goto eof;
  addr = (c & 0xff) << 8;
  // low start byte
  if ((c = getc(fp)) < 0)
    goto eof;
  addr |= (c & 0xff);
  // finished
  c = getc(fp);
  fclose(fp);
  if (c >= 0)
    fatal("Extra data in GT1 file '%s'\n", gt1);
  if (pc2cycs)
    memset(pc2cycs, 0, 0x10000 * sizeof(pc2cycs[0]));
  return addr;
 eof:
  fclose(fp);
  fatal("Premature EOF in GT1 file '%s'\n", gt1);
  return addr;
}


/* LIBSIM calls */

void sys_regbase(void)
{
  regbase = deek(vAC);
}

void sys_exit(void)
{
  if (deek(R9))
    printf("%s\n", &RAM[deek(R9)]);
  exit((sword)deek(R8));
}

void sys_printf(void)
{
  const char *fmt = (char*)&RAM[deek(R8)];
  word ap = deek(SP) + 4;
  int n = 0;
  if (trace)
    fflush(NULL);
  while(fmt && *fmt)
    {
      if (fmt[0] == '%')
        {
          if (fmt[1] == '%')
            {
              fmt += 1;
            }
          else
            {
              int i = 0;
              char conv = 0;
              char lng = 0;
              char spec[64];
              spec[0] = fmt[0];
              while(fmt[++i])
                {
                  if (i + 1 > sizeof(spec))
                    break;
                  else if (strchr("lLq", fmt[i]))
                    lng = spec[i] = fmt[i];
                  else if (strchr("#0- +0123456789.hlLjzZtq", fmt[i]))
                    spec[i] = fmt[i];
                  else 
                    { conv = spec[i] = fmt[i]; break; }
                }
              if (i+1 < sizeof(spec))
                {
                  spec[i+1] = 0;
                  if (strchr("eEfFgGaA", conv))
                    { n += printf(spec, feek(ap)); ap += 5; }
                  else if (strchr("sS", conv))
                    { ap = (ap+1)&~1; n += printf(spec, &RAM[deek(ap)]); ap += 2; }
                  else if (lng && strchr("ouxX", conv))
                    { ap = (ap+alignlong-1)&~(alignlong-1);
                      n += printf(spec, (long)(quad)leek(ap)); ap += 4; }
                  else if (lng)
                    { ap =  (ap+alignlong-1)&~(alignlong-1);
                      n += printf(spec, (long)(squad)leek(ap)); ap += 4; }
                  else if (strchr("ouxX", conv))
                    { ap = (ap+1)&~1; n += printf(spec, (word)deek(ap)); ap += 2; }
                  else
                    { ap = (ap+1)&~1; n += printf(spec, (sword)deek(ap)); ap += 2; }
                  fmt += i+1;
                  continue;
                }
            }
        }
      putchar(fmt[0]);
      fmt += 1;
      n += 1;
    }
  fflush(stdout);
  doke(vAC, n);
}

/* LIBSIM stdio forwarding */

/* Offsets in _iobuf structure (see stdio.h) */
#define G_IOBUF_FLAG_OFFSET 4
#define G_IOBUF_FILE_OFFSET 6
#define G_IOBUF_V_OFFSET    14

/* Error codes (see errno.h) */
#define G_EINVAL    3
#define G_ENOENT    4
#define G_ENOTDIR   5
#define G_EIO       8
#define G_EPERM     9
#define G_ENOTSUP  10

void sys_io_write(void)
{
  int flg = deek(deek(R8) + G_IOBUF_FLAG_OFFSET);
  int fd = deek(deek(R8) + G_IOBUF_FILE_OFFSET);
  int buf = deek(R9);
  int cnt = deek(R10);
  int ret = 0;
  int err = 0;
  /* Validate */
  if (fd < 0 || (flg & 2) == 0)
    err = G_EINVAL;
  if (buf + cnt >= 0x10000)
    cnt = 0x10000 - buf;
  if (cnt < 0)
    err = G_EINVAL;
  /* Write */
  if (err == 0) {
    if (fd <= 2)
      fflush(stdout);
    if ((ret = write(fd, RAM+buf, cnt)) <= 0)
      err = G_EIO;
  }
  if (err) {
    doke(deek(sysArgs0), err);
    doke(vAC, -1);
  } else {
    doke(vAC, ret);
  }
  return;
}

void sys_io_read(void)
{
  int flg = deek(deek(R8) + G_IOBUF_FLAG_OFFSET);
  int fd = deek(deek(R8) + G_IOBUF_FILE_OFFSET);
  int buf = deek(R9);
  int cnt = deek(R10);
  int ret = 0;
  int err = 0;

  /* Validate */
  if (fd < 0 || (flg & 1) == 0)
    err = EINVAL;
  if (buf + cnt >= 0x10000)
    cnt = 0x10000 - buf;
  if (cnt < 0)
    err = G_EINVAL;
  /* READ */
  if (err == 0) {
    if (fd <= 2)
      fflush(stdout);
    if ((ret = read(fd, RAM+buf, cnt)) < 0)
      err = G_EIO;
  }
  /* Return */
  if (err) {
    doke(deek(sysArgs0), err);
    doke(vAC, -1);
  } else {
    doke(vAC, ret);
  }
}

void sys_io_lseek(void)
{
  int flg = deek(deek(R8) + G_IOBUF_FLAG_OFFSET);
  int fd = deek(deek(R8) + G_IOBUF_FILE_OFFSET);
  off_t off = (squad)leek(R9);
  int whence = deek(R11);
  int err = 0;

  /* Validate */
  if (fd < 0 || flg == 0)
    err = G_EINVAL;
  if (whence == 0)
    whence = SEEK_SET;
  else if (whence == 2)
    whence = SEEK_END;
  else if (whence == 1)
    whence = SEEK_CUR;
  else
    err = G_EINVAL;
  /* Seek */
  if (err == 0) {
    off = lseek(fd, off, whence);
    if (off == (off_t)-1) {
      err = G_ENOTSUP;
      if (errno == EINVAL)
        err = G_EINVAL;
    }
  }
  /* Return */
  if (err) {
    doke(deek(sysArgs0), err);
    loke(LAC, -1);
  } else {
    loke(LAC, (long)off);
  }
}

void sys_io_close(void)
{
  int flg = deek(deek(R8) + G_IOBUF_FLAG_OFFSET);
  int fd = deek(deek(R8) + G_IOBUF_FILE_OFFSET);
  int err = 0;
  /* Validate */
  if (fd < 0 || flg == 0)
    err = G_EINVAL;
  /* Close */
  if (fd > 2 && close(fd) < 0 && err == 0)
    err = G_EIO;
  /* Return */
  if (err) {
    doke(deek(sysArgs0), err);
    doke(vAC, -1);
  } else {
    doke(vAC, 0);
  }
}

void sys_io_openf(void)
{
  int flg = deek(deek(R8) + G_IOBUF_FLAG_OFFSET);
  int rfd = deek(deek(R8) + G_IOBUF_FILE_OFFSET);
  int err = 0;
  
  if (okopen)
    {
      int fd = 0;
      int oflags = 0;
      if ((flg & 1) && (flg & 2))
        oflags = O_RDWR;
      else if (flg & 1)
        oflags = O_RDONLY;
      else if (flg & 2)
        oflags = O_WRONLY | O_CREAT | O_TRUNC;
      else
        err = G_EINVAL;
      if (flg  & 8)
        oflags |= O_APPEND;
      if (err == 0 && (fd = open((void*)(RAM+deek(R9)), oflags, 0666)) < 0) {
        switch (errno) {
        default: err = EIO; break;
        case ENOENT: err = G_ENOENT; break;
        case EACCES: case EPERM: err = G_EPERM; break;
        case ENOTDIR: case EISDIR: err = G_ENOTDIR; break;
        case EINVAL: err = G_EINVAL; break;
        }
      }
      if (err == 0 && rfd >= 0 && dup2(fd, rfd) >= 0) {
        close(fd);
        fd = rfd;
      }
      doke(deek(R8) + G_IOBUF_FILE_OFFSET, fd);
      doke(deek(R8) + G_IOBUF_V_OFFSET, deek(sysArgs0+2));
    }
  else
    {
      char mode[4] = "\0\0\0\0";
      if (flg & 1)
        strcat(mode,"R");
      if (flg & 2)
        strcat(mode,"W");
      if (flg & 8)
        strcat(mode,"A");
      fprintf(stderr, "\n(gtsim) denied attempt to open file '%s' %s. (allow with -f)\n",
              RAM+deek(R9), mode);
      err = G_EPERM;
    }
  /* Return */
  if (err) {
    doke(deek(sysArgs0), err);
    doke(vAC, -1);
  } else {
    doke(vAC, 0);
  }
}

void sys_0x3b4(CpuState *S)
{
  if ((deek(sysFn) & 0xff00) == 0xff00)
    {
      debug("vPC=%#x SYS(%d) ", deek(vPC), S->AC); debugSysFn(); debug("\n");
      /* Pseudo SYS calls are captured here */
      switch(deek(sysFn))
        {
        case 0xffff: sys_regbase(); break;
        case 0xff00: sys_exit(); break;
        case 0xff01: sys_printf(); break;
        case 0xff02: sys_io_write(); break;
        case 0xff03: sys_io_read(); break;
        case 0xff04: sys_io_lseek(); break;
        case 0xff05: sys_io_close(); break;
        case 0xff06: sys_io_openf(); break;
        default: fprintf(stderr,"(gtsim) unimplemented SysFn=%#x\n", sysFn); break;
        }
      /* Return with no action and proper timing */
      S->IR = 0x00; S->D = 0xfa; /* LD(-12/2) */
      S->PC = 0x300;             /* NEXTY */
    }

  if (deek(sysFn) == SYS_Exec_88)
    {
      static int exec_count = 0;
      debug("vPC=%#x SYS(%d) [EXEC] ", deek(vPC), S->AC); debugSysFn(); debug("\n");
      if (++exec_count == 2 && gt1)
        {
          // First exec is Reset.
          // Second exec is MainMenu.
          // Load GT1 instead
          int execaddr = loadGt1(gt1);
          debug("Loading file '%s' with start address %#x\n", gt1, execaddr);
          doke(vPC, addlo(execaddr,-2));
          doke(vLR, execaddr);
          // And return from SYS_Exec
          S->IR = 0x00; S->D = 0xf8; /* LD(-16/2) */
          S->PC = 0x3cb;             /* REENTER */
          ot = t+3;
          nogt1 = 1;
        }
    }

  if (deek(sysFn) == SYS_SetMode_v2_80)
    {
      debug("vPC=%04x SYS(%d) [SETMODE] vAC=%04x\n", deek(vPC), S->AC, deek(vAC));
      if (vmode >= 0 && !nogt1)
        poke(vAC, vmode);
    }
}



/* ----------------------------------------------- */
/* TRACING VCPU                                    */
/* ----------------------------------------------- */



int oper8(word addr, int i, char *operand)
{
  sprintf(operand, "$%02x", peek(addlo(addr,i)));
  return i+1;
}

int oper16(word addr, int i, char *operand)
{
  sprintf(operand, "$%04x", deek(addlo(addr,i)));
  return i+2;
}

int disassemble(word addr, char **pm, char *operand)
{
  switch(peek(addr))
    {
    case 0x5e:  *pm = "ST"; goto oper8;  
    case 0x2b:  *pm = "STW"; goto oper8;
    case 0xec:  *pm = "STLW"; goto oper8;
    case 0x1a:  *pm = "LD"; goto oper8;
    case 0x59:  *pm = "LDI"; goto oper8;
    case 0x11:  *pm = "LDWI"; goto oper16;
    case 0x21:  *pm = "LDW"; goto oper8;
    case 0xee:  *pm = "LDLW"; goto oper8;
    case 0x99:  *pm = "ADDW"; goto oper8;
    case 0xb8:  *pm = "SUBW"; goto oper8;
    case 0xe3:  *pm = "ADDI"; goto oper8;
    case 0xe6:  *pm = "SUBI"; goto oper8;
    case 0xe9:  *pm = "LSLW"; return 1;
    case 0x93:  *pm = "INC"; goto oper8;
    case 0x82:  *pm = "ANDI"; goto oper8;
    case 0xf8:  *pm = "ANDW"; goto oper8;
    case 0x88:  *pm = "ORI"; goto oper8;
    case 0xfa:  *pm = "ORW"; goto oper8;
    case 0x8c:  *pm = "XORI"; goto oper8;
    case 0xfc:  *pm = "XORW"; goto oper8;
    case 0xad:  *pm = "PEEK"; return 1;
    case 0xf6:  *pm = "DEEK"; return 1;
    case 0xf0:  *pm = "POKE"; goto oper8;
    case 0xf3:  *pm = "DOKE"; goto oper8;
    case 0x7f:  *pm = "LUP"; goto oper8;
    case 0x90:  *pm = "BRA"; goto operbr;
    case 0xcf:  *pm = "CALL"; goto oper8;
    case 0xff:  *pm = "RET"; return 1;
    case 0x75:  *pm = "PUSH"; return 1;
    case 0x63:  *pm = "POP"; return 1;
    case 0xdf:  *pm = "ALLOC"; goto oper8;
    case 0xcd:  *pm = "DEF"; goto oper8;
    case 0x85:  *pm = "CALLI"; goto oper16;
    case 0x1f:  *pm = "CMPHS"; goto oper8;
    case 0x97:  *pm = "CMPHU"; goto oper8;
    case 0x35: {
      switch(peek(addlo(addr,1)))
        {
        case 0x3f:  *pm = "BEQ"; break;
        case 0x72:  *pm = "BNE"; break;
        case 0x50:  *pm = "BLT"; break;
        case 0x4d:  *pm = "BGT"; break;
        case 0x56:  *pm = "BLE"; break;
        case 0x53:  *pm = "BGE"; break;
        default:    *pm = "B??"; break;
        }
      sprintf(operand, "$%04x", (addr&0xff00)|((peek(addlo(addr,2))+2)&0xff));
      return 3;
    }
    case 0xb4: {
      sbyte b = peek(addlo(addr,1));
      if (b > -128 && b <= 0) {
        *pm = "SYS"; sprintf(operand, "%d", 2*(14-b));
      } else 
        *pm = (b > 0) ? "S??" : "HALT";
      return 2;
    }
    /* CPU6 instructions below */
    case 0x14: *pm="DEC"; goto oper8;
    case 0x16: *pm="MOVQB"; goto oper88;
    case 0x18: *pm="LSRB"; goto oper8;
    case 0x1c: *pm="LOKEQI"; goto oper16;
    case 0x23: *pm="PEEK+"; goto oper8;
    case 0x25: *pm="POKEI"; goto oper8;
    case 0x27: *pm="LSLV"; goto oper8;
    case 0x29: *pm="ADDBA"; goto oper8;
    case 0x2d: *pm="ADDBI"; goto oper88;
    case 0x32: *pm="DBNE"; goto oper8br;
    case 0x37: *pm="DOKEI"; goto oper16r;
    case 0x39: *pm="PEEKV"; goto oper8;
    case 0x3b: *pm="DEEKV"; goto oper8;
    case 0x3d: *pm="LOKEI"; goto oper16;
    case 0x42: *pm="ADDVI"; goto oper88;
    case 0x44: *pm="SUBVI"; goto oper88;
    case 0x46: *pm="DOKE+"; goto oper8;
    case 0x48: *pm="NOTB"; goto oper8;
    case 0x4a: *pm="DJGE"; goto oper8x16p2;
    case 0x5b: *pm="MOVQW"; goto oper88;
    case 0x60: *pm="DEEK+"; goto oper8;
    case 0x65: *pm="MOV"; goto oper88r;
    case 0x67: *pm="PEEKA"; goto oper8;
    case 0x69: *pm="POKEA"; goto oper8;
    case 0x6b: *pm="TEQ"; goto oper8;
    case 0x6d: *pm="TNE"; goto oper8;
    case 0x6f: *pm="DEEKA"; goto oper8;
    case 0x77: *pm="SUBBA"; goto oper8;
    case 0x79: *pm="INCW"; goto oper8;
    case 0x7b: *pm="DECW"; goto oper8;
    case 0x7d: *pm="DOKEA"; goto oper8;
    case 0x8a: *pm="PEEKA+"; goto oper8;
    case 0x8e: *pm="DBGE"; goto oper8br;
    case 0x95: *pm="INCWA"; goto oper8;
    case 0x9c: *pm="LDNI"; goto oper8n;
    case 0x9e: *pm="ANDBK"; goto oper88r;
    case 0xa0: *pm="ORBK"; goto oper88r;
    case 0xa2: *pm="XORBK"; goto oper88r;
    case 0xa4: *pm="DJNE"; goto oper8x16p2;
    case 0xa7: *pm="CMPI"; goto oper88;
    case 0xa9: *pm="ADDVW"; goto oper88;
    case 0xab: *pm="SUBVW"; goto oper88;
    case 0xbb: *pm="JEQ"; goto oper16p2;
    case 0xbd: *pm="JNE"; goto oper16p2;
    case 0xbf: *pm="JLT"; goto oper16p2;
    case 0xc1: *pm="JGT"; goto oper16p2;
    case 0xc3: *pm="JLE"; goto oper16p2;
    case 0xc5: *pm="JGE"; goto oper16p2;
    case 0xd1: *pm="POKE+"; goto oper8;
    case 0xd3: *pm="LSRV"; goto oper8;
    case 0xd5: *pm="TGE"; goto oper8;
    case 0xd7: *pm="TLT"; goto oper8;
    case 0xd9: *pm="TGT"; goto oper8;
    case 0xdb: *pm="TLE"; goto oper8;
    case 0xdd: *pm="DECWA"; goto oper8;
    case 0xe1: *pm="SUBBI"; goto oper88;
    case 0xb1: {
      switch(peek(addlo(addr,1)))
        {
        case 0x11: *pm = "NOTE"; return 2;
        case 0x14: *pm = "MIDI"; return 2;
        case 0x17: *pm = "XLA"; return 2;
          // more
        case 0x1a: *pm = "ADDLP"; return 2;
        case 0x1d: *pm = "SUBLP"; return 2;
        case 0x20: *pm = "ANDLP"; return 2;
        case 0x23: *pm = "ORLP"; return 2;
        case 0x26: *pm = "XORLP"; return 2;
        case 0x29: *pm = "CMPLPU"; return 2;
        case 0x2c: *pm = "CMPLPS"; return 2;
        default:
          return 2;
        }
    }
    case 0x2f: {
      switch(peek(addlo(addr,2)))
        {
        case 0x11: *pm = "LSLN"; goto oper8p2;
        case 0x13: *pm = "SEXT"; goto oper8p2;
        case 0x15: *pm = "NOTW"; goto oper8p2;
        case 0x17: *pm = "NEGQ"; goto oper8p2;
        case 0x19: *pm = "ANDBA"; goto oper8p2;
        case 0x1c: *pm = "ORBA"; goto oper8p2;
        case 0x1f: *pm = "XORBA"; goto oper8p2;
        case 0x22: *pm = "FREQM"; goto oper8p2;
        case 0x24: *pm = "FREQA"; goto oper8p2;
        case 0x27: *pm = "FREQZ"; goto oper8p2;
        case 0x2a: *pm = "VOLM"; goto oper8p2;
        case 0x2c: *pm = "VOLA"; goto oper8p2;
        case 0x2f: *pm = "MODA"; goto oper8p2;
        case 0x32: *pm = "MODZ"; goto oper8p2;
        case 0x34: *pm = "SMCPY"; goto oper8p2;
        case 0x37: *pm = "CMPWS"; goto oper8p2;
        case 0x39: *pm = "CMPWU"; goto oper8p2;
        case 0x3b: *pm = "LEEKA"; goto oper8p2;
        case 0x3d: *pm = "LOKEA"; goto oper8p2;
        case 0x4e: *pm = "INCL"; goto oper8p2;
        case 0x51: *pm = "DECL"; goto oper8p2;
          // exp
        case 0xcd: *pm = "NCOPY"; goto oper8p2;
        case 0xd0: *pm = "STLU"; goto oper8p2;
        case 0xd3: *pm = "STLS"; goto oper8p2;
        case 0xd6: *pm = "NOTL"; goto oper8p2;
        case 0xd9: *pm = "NEGL"; goto oper8p2;
        oper8p2:
          sprintf(operand, "$%02x", peek(addlo(addr,1)));
        default:
          return 3;
        }
    }
    case 0xc7: {
      switch(peek(addlo(addr,2)))
        {
        case 0x11:  *pm = "STB2"; goto oper16p3;
        case 0x14:  *pm = "STW2"; goto oper16p3;
        case 0x17:  *pm = "XCHGB"; goto oper88p3;
        case 0x19:  *pm = "MOVW"; goto oper88p3;
        case 0x1b:  *pm = "ADDWI"; goto oper16p3;
        case 0x1d:  *pm = "SUBWI"; goto oper16p3;
        case 0x1f:  *pm = "ANDWI"; goto oper16p3;
        case 0x21:  *pm = "ORWI"; goto oper16p3;
        case 0x23:  *pm = "XORWI"; goto oper16p3;
        case 0x25:  *pm = "LDPX"; goto oper88p3;
        case 0x28:  *pm = "STPX"; goto oper88p3;
        case 0x2b:  *pm = "CONDI"; goto oper88p3;
        case 0x2d:  *pm = "CONDB"; goto oper88p3;
        case 0x30:  *pm = "CONDIB"; goto oper88p3;
        case 0x33:  *pm = "CONDBI"; goto oper88p3;
        case 0x35:  *pm = "XCHGW"; goto oper88p3;
        case 0x38:  *pm = "OSCPX"; goto oper88p3;
        case 0x3a:  *pm = "SWAPB"; goto oper88p3;
        case 0x3d:  *pm = "SWAPW"; goto oper88p3;
        case 0x40:  *pm = "NEEKA"; goto oper88p3;
        case 0x43:  *pm = "NOKEA"; goto oper88p3;
          // more
        case 0x67:  *pm = "ANDBI"; goto oper88p3r;
        case 0x6a:  *pm = "ORBI"; goto oper88p3r;
        case 0x6d:  *pm = "XORBI"; goto oper88p3r;
        case 0x70:  *pm = "JMPI"; goto oper16p3;
          // exp
        case 0xcd:  *pm = "MOVL"; goto oper88p3;
        case 0xd0:  *pm = "MOVF"; goto oper88p3;
        case 0xd3:  *pm = "NROL"; goto oper88p3n;
        case 0xd6:  *pm = "NROR"; goto oper88p3nr;
        oper16p3:
          sprintf(operand, "$%02x%02x", peek(addlo(addr,1)),peek(addlo(addr,3)));
          return 4;
        oper88p3r:
          sprintf(operand, "$%02x, $%02x", peek(addlo(addr,1)),peek(addlo(addr,3)));
          return 4;
        oper88p3:
          sprintf(operand, "$%02x, $%02x", peek(addlo(addr,3)),peek(addlo(addr,1)));
          return 4;
        oper88p3n:
          sprintf(operand, "$%02x, $%02x",
                  (peek(addlo(addr,3))-peek(addlo(addr,1)))&0xff,
                  peek(addlo(addr,1)));
          return 4;
        oper88p3nr:
          sprintf(operand, "$%02x, $%02x",
                  (peek(addlo(addr,1))-peek(addlo(addr,3)))&0xff,
                  peek(addlo(addr,3)));
          return 4;
        default:
          return 4;
        }
    }
    default:
      return 2;
    oper8:
      sprintf(operand, "$%02x", peek(addlo(addr,1)));
      return 2;
    oper8n:
      sprintf(operand, "-$%02x", peek(addlo(addr,1)));
      return 2;
    oper88:
      sprintf(operand, "$%02x, $%02x", peek(addlo(addr,1)),peek(addlo(addr,2)));
      return 3;
    oper88r:
      sprintf(operand, "$%02x, $%02x", peek(addlo(addr,2)),peek(addlo(addr,1)));
      return 3;
    oper16:
      sprintf(operand, "$%04x", deek(addlo(addr,1)));
      return 3;
    oper16r:
      sprintf(operand, "$%02x%02x", peek(addlo(addr,1)), peek(addlo(addr,2)));
      return 3;
    oper16p2:
      sprintf(operand, "$%04x", addlo(deek(addlo(addr,1)),2));
      return 3;
    oper8x16p2:
      sprintf(operand, "$%02x, $%04x", peek(addlo(addr,1)), addlo(deek(addlo(addr,2)),2));
      return 4;
    operbr:
      sprintf(operand, "$%04x", (addr&0xff00)|((peek(addlo(addr,1))+2)&0xff));
      return 2;
    oper8br:
      sprintf(operand, "$%02x, $%04x", peek(addlo(addr,2)),
              (addr&0xff00)|((peek(addlo(addr,1))+2)&0xff) );
      return 3;
    }
}

void print_trace(CpuState *S)
{
  char operand[32];
  char *mnemonic = "???";
  word addr = addlo(deek(vPC),2);
  operand[0] = 0;
  disassemble(addr, &mnemonic, operand);
  fprintf(stderr, "%04x: [", addr);
  if (strchr(trace, 'n')) {
    fprintf(stderr, "  AC=%02x YX=%02x%02x vTicks=%d t=%+06ld\n\t", S->AC, S->Y, S->X, (int)(char)peek(0x15), (long)((t-ot) % 1000000));
  }
  fprintf(stderr, " vAC=%04x vLR=%04x", deek(vAC), deek(vLR));
  if (strchr(trace, 's'))
    fprintf(stderr, " vSP=%02x", peek(vSP));
  if (strchr(trace, 'l'))
    fprintf(stderr, " B[0-2]=%02x %02x %02x LAC=%08x",
            peek(B0), peek(B0+1), peek(B0+2), leek(LAC));
  if (strchr(trace, 't'))
    fprintf(stderr, " T[0-3]=%04x %04x %04x %04x",
            deek(T0), deek(T0+2), deek(T0+4), deek(T0+6));
  if (strchr(trace, 'f')) {
    int as = peek(B0);
    int ae = peek(B0+1);
    int64_t am = leek(LAC-1) + ((int64_t)peek(LAC+3) << 32);
    int be = peek(T0+5);
    int64_t bm = leek(T0) + ((int64_t)peek(T0+4) << 32);
    fprintf(stderr, "\n\t AS=%02x AE=%02x AM=%010llx BE=%02x BM=%010llx\n\t FAC=%.8g FARG=%.8g",
            as, ae, (long long)am, be, (long long)bm,
            ((as&0x80) ? -1.0 : +1.0) * ((ae) ? ldexp((double)am, ae-168) : 0.0),
            ((as&0x80)^((as&1)<<7) ? -1.0 : +1.0) * ((be) ? ldexp((double)bm, be-168) : 0.0) );
  }
  if (strchr(trace, 'S')) {
    int i;
    fprintf(stderr, "\n\t sysFn=%04x sysArgs=%02x", deek(sysFn), peek(sysArgs0));
    for(i=1; i<7; i++)
      fprintf(stderr, " %02x", peek(sysArgs0+i));
  }
  if (strchr(trace, 'r')) {
    int i;
    fprintf(stderr, "\n\t R[00-07]=%04x", deek(R0));
    for (i=1; i<8; i++)
      fprintf(stderr, " %04x", deek(R0+i+i));
    fprintf(stderr, "\n\t R[08-15]=%04x", deek(R0+16));
    for (i=9; i<16; i++)
      fprintf(stderr, " %04x", deek(R0+i+i));
    fprintf(stderr, "\n\t R[16-23]=%04x", deek(R0+32));
    for (i=17; i<24; i++)
      fprintf(stderr, " %04x", deek(R0+i+i));
    fprintf(stderr, "=SP");
  }
  fprintf(stderr, " ]  %-5s %-18s\n",  mnemonic, operand);
}

void next_0x301(CpuState *S)
{
  if (nogt1) {
    if (trace)
      print_trace(S);
    if (pc2cycs)
      pc2cycs[opc] += t - ot;
  }
}



/* ----------------------------------------------- */
/* MAIN                                            */
/* ----------------------------------------------- */


void garble(uint8_t mem[], int len)
{
  for (int i=0; i<len; i++) mem[i] = rand();
}

void usage(int exitcode)
{
  fprintf(stderr,"Usage: gtsim [options] -rom romfile gt1file\n");
  if (exitcode == EXIT_SUCCESS) {
    fprintf(stderr,"\n"
            "Simulate the Gigatron executing <gt1file>\n"
            "The simulator captures the SYS_Exec calls to load <gt1file>\n"
            "instead of the main menu.\n"
            "\n"
            "Options:\n"
            "  -v: print debug messages\n"
            "  -rom romfile: load rom from <romfile>\n"
            "  -f: enable file system access\n"
            "  -nogt1: do not override main menu and run forever\n"
            "  -vmode v: set video mode 0,1,2,3,1975\n"
            "  -t<letters>: trace VCPU execution\n"
            "  -prof fn: writes profiling information into file <fn>\n");
    
  }
  exit(exitcode);
}

int main(int argc, char *argv[])
{
  // Initialize with randomized data
  srand(time(NULL));
  garble((void*)ROM, sizeof ROM);
  garble((void*)RAM, sizeof RAM);
  // Parse options
  for (int i=1; i<argc; i++)
    {
      if (!strcmp(argv[i],"-h"))
        {
          usage(EXIT_SUCCESS);
        }
      else if (! strcmp(argv[i], "-nogt1"))
        {
          nogt1 = 1;
        }
      else if (! strcmp(argv[i], "-v"))
        {
          verbose = 1;
        }
      else if (! strncmp(argv[i], "-t", 2))
        {
          trace = argv[i]+2;
        }
      else if (! strcmp(argv[i], "-f"))
        {
          okopen = 1;
        }
      else if (! strcmp(argv[i], "-prof"))
        {
          if (i+1 >= argc)
            fatal("Missing argument for option -prof\n");
          if (prof)
            fatal("Duplicate option -prof\n");
          setup_profile(argv[++i]);
        }
      else if (! strcmp(argv[i], "-rom"))
        {
          if (i+1 >= argc)
            fatal("Missing argument for option -rom\n");
          if (rom)
            fatal("Duplicate option -rom\n");
          rom = argv[++i];
        }
      else if (! strcmp(argv[i],"-vmode"))
        {
          if (i+1 >= argc)
            fatal("Missing argument for option -vmode\n");
          char *s = argv[++i], *e = 0;
          vmode = strtol(s, &e, 0);
          if (e == s || *e)
            fatal("Invalid value '%s' for option -vmode\n", s);
          if (vmode!= 1975 && (vmode < 0 || vmode > 3))
            fatal("Invalid value '%s' for option -vmode\n", s);
        }
      else if (argv[i][0] == '-')
        {
          fatal("Unrecognized option %s\n", argv[i]);
        }
      else
        {
          if (gt1)
            usage(EXIT_FAILURE);
          gt1 = argv[i];
        }
    }
  if (! gt1 && ! nogt1)
    usage(EXIT_FAILURE);
  
  // Read rom
  if (! rom)
    rom = "../gigatron-rom/dev.rom";
  FILE *fp = fopen(rom, "rb");
  if (!fp)
    fatal("Failed to open rom file '%s'\n", rom);
  if (fread(ROM, 1, sizeof(ROM), fp) != sizeof(ROM))
    fatal("Failed to read enough data from rom file '%s'\n", rom);
  fclose(fp);
  
  // Simulate
  sim();
  return 0;
}


/*---- END CODE --*/

/* Local Variables: */
/* mode: c */
/* c-basic-offset: 2 */
/* indent-tabs-mode: () */
/* End: */
