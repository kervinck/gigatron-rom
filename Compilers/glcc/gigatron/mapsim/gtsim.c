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
# define UNIX 1
# include <unistd.h>
# include <fcntl.h>
# include <signal.h>
# include <termios.h>
# undef B0
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
void next_0x307(CpuState*);

char *rom = 0;
char *gt1 = 0;
int nogt1 = 0;
int nogarble = 0;
const char *trace = 0;
int verbose = 0;
int okopen = 0;
int vmode = 3; /*1975;*/
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

uint8_t ROM[1<<16][2], RAM[1<<17], IN=0xff;
uint8_t CTRL;
uint32_t bank;
long long t;

uint8_t pfx;
uint16_t da;
long long dt, rt, st;
long long vt[2];


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
  uint32_t xaddr = (addr & 0x8000) ? (addr & 0x7fff) | bank : addr;

  int B = S.undef; // Data Bus
  switch (bus) {
    case 0: B=S.D;                        break;
    case 1: if (!W) B = RAM[xaddr];       break;
    case 2: B=S.AC;                       break;
    case 3: B=IN;                         break;
  }

  if (W) {
    if (bus == 1) {
      CTRL = lo;
      bank = (CTRL & 0xc0) << 9;
    } else
      RAM[xaddr] = B; // Random Access Memory
  }

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
        if (vgaX != 200 && t >= 6250000) {
          fprintf(stderr, "(gtsim) Horizontal timing error:"
                  "vgaY %-3d, vgaX %-3d, t=%0.3f\n", vgaY, vgaX, t/6.25e6);
#if EXTRA_DEBUG
          for(int i=0; i<48; i+=16) {
            fprintf(stderr, "\t%04x:", i);
            for (int j=0; j<16; j++)
              fprintf(stderr, " %02x", RAM[i+j]);
            fprintf(stderr, "\n");
          }
#endif     
        }
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
        rt = t + 6;             /* instruction return time (adjusted for 0x307) */
        pfx = 0;
      } else if (S.PC == 0x307) {
        next_0x307(&T);         /* t-dt = (rt-dt-vt[0]) + (t-rt-vt[1]) + (vt[0]+vt[1])  */
        pfx = 2;                /* cpu prefix */
        vt[0] = vt[1] = 0;      /* video time (inside and outside instruction) */
        dt = t;                 /* instruction dispatch time */
        da = RAM[22]+(RAM[23]<<8);
      } else if (pfx && (S.PC & 0xff) == 1 && (S.PC >> 8) == (RAM[5]+1)) {
        pfx = RAM[5];
      } else if (S.IR == 0xe1 && S.D == 0x1e) { // jmp(Y,[vReturn])
        vt[!pfx] -= t;
      } else if (S.IR == 0xe0 && S.D == 0xff && S.Y == RAM[5]) {
        vt[!pfx] += t - 3;
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

/* Register base.
   Code that does not advertise register bases assume default values
   that match those alllocated by glink in the absence of rom.json
   overrides. GLCC-2.x always advertises the correct numbers
   when compiled with map=sim.
*/

unsigned int regbase = 0x90;
unsigned int flbase  = 0x81;
unsigned int t0base  = 0x24;
unsigned int t2base  = 0x88;
unsigned int b0base  = 0x8c;
unsigned int spbase  = 0x8e;

#define vPC       (0x16)
#define vAC       (0x18)
#define vLR       (0x1a)
#define vSP       (0x1c)
#define sysFn     (0x22)
#define sysArgs0  (0x24+0)
#define LAC       (flbase+3)
#define T0        (t0base)
#define B0        (b0base)
#define T2        (t2base)
#define R0        (regbase+0)
#define R8        (regbase+16)
#define R9        (regbase+18)
#define R10       (regbase+20)
#define R11       (regbase+22)
#define R12       (regbase+24)
#define SP        (spbase)

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
  flbase = peek(sysArgs0);
  t0base = peek(sysArgs0+1);
  t2base = peek(sysArgs0+2);
  b0base = peek(sysArgs0+3);
  spbase = peek(sysArgs0+4);
}

void sys_exit(void)
{
  if (deek(R9))
    printf("%s\n", &RAM[deek(R9)]);
  if (pc2cycs)
    printf("\ntotal %lld cycles (with video & overhead)\n", (t-10)-(st-6));
  exit((sword)deek(R8));
}

void sys_printf(void)
{
  const char *fmt = (char*)&RAM[deek(R8)];
  word ap = deek(SP) + 2;
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
#define G_IOBUF_FLAG_OFFSET 0
#define G_IOBUF_VTBL_OFFSET 2
#define G_IOBUF_FILE_OFFSET 4

/* Error codes (see errno.h) */
#define G_EINVAL    3
#define G_ENOENT    4
#define G_ENOTDIR   5
#define G_EIO       8
#define G_EPERM     9
#define G_ENOTSUP  10

void sys_io_writall(void)
{
  int flg = deek(deek(R8) + G_IOBUF_FLAG_OFFSET);
  int fd  = deek(deek(R8) + G_IOBUF_FILE_OFFSET);
  int buf = deek(R9);
  int cnt = deek(R10);
  int ret = 0;
  int tot = 0;
  int err = 0;
  /* Validate */
  if (fd < 0 || (flg & 2) == 0)  /* _IOWRITE */
    err = G_EINVAL;
  if (buf + cnt >= 0x10000)
    cnt = 0x10000 - buf;
  if (cnt < 0)
    err = G_EINVAL;
  /* Write */
  if (err == 0) {
    if (fd <= 2)
      fflush(stdout);
    while (cnt > 0) {
      if ((ret = write(fd, RAM+buf, cnt)) <= 0)
        err = G_EIO;
      tot += ret;
      cnt -= ret;
    }
  }
  if (err) {
    doke(deek(sysArgs0), err);
    doke(vAC, -1);
  } else {
    doke(vAC, tot);
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
  if (fd < 0 || (flg & 1) == 0) /* _IOREAD */
    err = EINVAL;
  if (buf + cnt >= 0x10000)
    cnt = 0x10000 - buf;
  if (cnt < 0)
    err = G_EINVAL;
  /* READ */
  if (err == 0) {
#ifdef UNIX
    struct termios told, tnew;
    if (isatty(fd)) {
      tcgetattr(fd, &told);
      if (! (flg & 0x40)) {     /* _IOFBF */
        tnew = told;
        cfmakeraw(&tnew);
        tcsetattr(fd, TCSAFLUSH, &tnew);
      }
    }
#endif
    if (fd <= 2)
      fflush(stdout);
    if ((ret = read(fd, RAM+buf, cnt)) < 0)
      err = G_EIO;
#ifdef UNIX
    if (isatty(fd))
      tcsetattr(fd, TCSANOW, &told);
#endif
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

void sys_io_flush(void)
{
  int flg = deek(deek(R8) + G_IOBUF_FLAG_OFFSET);
  int fd = deek(deek(R8) + G_IOBUF_FILE_OFFSET);
  int err = 0;
  /* Validate */
  if (fd < 0 || flg == 0)
    err = G_EINVAL;
  /* Close */
  if (fd > 2 && deek(R9) && close(fd) < 0 && err == 0)
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
      doke(deek(R8) + G_IOBUF_FILE_OFFSET, fd);
      doke(deek(R8) + G_IOBUF_VTBL_OFFSET, deek(sysArgs0+2));
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
      fprintf(stderr, "\n(gtsim) denied attempt to open file '%s' [%s]. (allow with -f)\n",
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
        case 0xff02: sys_io_writall(); break;
        case 0xff03: sys_io_read(); break;
        case 0xff04: sys_io_lseek(); break;
        case 0xff05: sys_io_flush(); break;
        case 0xff06: sys_io_openf(); break;
        default:
          fprintf(stderr,"(gtsim) unimplemented SysFn=%#x\n", deek(sysFn));
          break;
        }
      /* Return with no action and proper timing */
      S->IR = 0x00; S->D = 0xfa; /* LD(-12/2) */
      S->PC = 0x300;             /* NEXTY */
    }

  if (deek(sysFn) == SYS_Exec_88)
    {
      static int exec_count = 0;
      int pc = deek(vPC);
      debug("vPC=%#x SYS(%d) [EXEC] ", pc, S->AC); debugSysFn(); debug("\n");
      if (exec_count==0 && pc>=0x1f0 && pc<0x1f8)
        {
          debug("Going into Reset.gt1\n");
        }
      else if (++exec_count == 1 && gt1)
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
          nogt1 = 1;
          st = dt = rt = t + 9;
          da = 0;
        }
    }

  if (deek(sysFn) == SYS_SetMode_v2_80)
    {
      debug("vPC=%04x SYS(%d) [SETMODE] vAC=%04x\n", deek(vPC), S->AC, deek(vAC));
      if (vmode >= 0 && !nogt1)
        doke(vAC, vmode);
    }
}



/* ----------------------------------------------- */
/* TRACING VCPU                                    */
/* ----------------------------------------------- */



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
        case 0x00:  *pm = "ADDL";  return 2;       /* v7 */
        case 0x02:  *pm = "?ADDX"; return 2;       /* v7 */
        case 0x04:  *pm = "SUBL";  return 2;       /* v7 */
        case 0x06:  *pm = "ANDL";  return 2;       /* v7 */
        case 0x08:  *pm = "ORL";   return 2;       /* v7 */
        case 0x0a:  *pm = "XORL";  return 2;       /* v7 */
        case 0x0c:  *pm = "NEGVL"; goto operx8;    /* v7 */
        case 0x0e:  *pm = "NEGX";  return 2;       /* v7 */
        case 0x10:  *pm = "LSLVL"; goto operx8;    /* v7 */
        case 0x12:  *pm = "LSLXA"; return 2;       /* v7 */
        case 0x14:  *pm = "CMPLS"; return 2;       /* v7 */
        case 0x16:  *pm = "CMPLU"; return 2;       /* v7 */
        case 0x18:  *pm = "LSRXA"; return 2;       /* v7 */
        case 0x1a:  *pm = "RORX";  return 2;       /* v7 */
        case 0x1c:  *pm = "MACX";  return 2;       /* v7 */
        case 0x1e:  *pm = "LDLAC"; return 2;       /* v7 */
        case 0x20:  *pm = "STLAC"; return 2;       /* v7 */
        case 0x23:  *pm = "INCVL"; goto operx8;    /* v7 */
        case 0x25:  *pm = "STFAC"; return 2;       /* v7 */
        case 0x27:  *pm = "LDFAC"; return 2;       /* v7 */
        case 0x29:  *pm = "LDFARG";return 2;       /* v7 */
        case 0x39:  *pm = "RDIVS"; goto operx8;    /* v7 */
        case 0x3b:  *pm = "RDIVU"; goto operx8;    /* v7 */
        case 0x3d:  *pm = "MULW";  goto operx8;    /* v7 */
        case 0x3f:  *pm = "BEQ";   goto operxbr;
        case 0x4d:  *pm = "BGT";   goto operxbr;
        case 0x50:  *pm = "BLT";   goto operxbr;
        case 0x53:  *pm = "BGE";   goto operxbr;
        case 0x56:  *pm = "BLE";   goto operxbr;
        case 0x5c:  *pm = "RESET"; return 2;       /* v7 */
        case 0x62:  *pm = "DOKEI"; goto operx16r;  /* v7 */
        case 0x72:  *pm = "BNE";   goto operxbr;
        case 0x7d:  *pm = "ADDIV"; goto operx8x2;  /* v7 */
        case 0x9c:  *pm = "SUBIV"; goto operx8x2;  /* v7 */
        case 0xcb:  *pm = "COPY";  return 2;       /* v7 */
        case 0xcf:  *pm = "COPYN"; goto operx8;    /* v7 */
        case 0xdb:  *pm = "MOVL";  goto operx8x2r; /* v7 */
        case 0xdd:  *pm = "MOVF";  goto operx8x2r; /* v7 */
        default:    *pm = "???";   goto unknown;
        operx8:
          sprintf(operand, "$%02x", peek(addlo(addr,2)));
          return 3;
        operx8x2:
          sprintf(operand, "$%02x, $%02x", peek(addlo(addr,2)), peek(addlo(addr,3)));
          return 4;
        operx8x2r:
          sprintf(operand, "$%02x, $%02x", peek(addlo(addr,3)), peek(addlo(addr,2)));
          return 4;
        operxbr:
          sprintf(operand, "$%04x", (addr&0xff00)|((peek(addlo(addr,2))+2)&0xff));
          return 3;
        operx16:
          sprintf(operand, "$%02x%02x", peek(addlo(addr,3)),peek(addlo(addr,2)));
          return 4;
        operx16r:
          sprintf(operand, "$%02x%02x", peek(addlo(addr,2)),peek(addlo(addr,3)));
          return 4;
        }
    }
    case 0xb4: {
      sbyte b = peek(addlo(addr,1));
      if (b > -128 && b <= 0) {
        *pm = "SYS"; sprintf(operand, "%d", 2*(14-b));
      } else 
        *pm = (b > 0) ? "S??" : "HALT";
      return 2;
    }
    case 0x18:  *pm = "NEGV";  goto oper8;    /* v7 */
    case 0x39:  *pm = "POKEA"; goto oper8;    /* v7 */
    case 0x3b:  *pm = "DOKEA"; goto oper8;    /* v7 */
    case 0x3d:  *pm = "DEEKA"; goto oper8;    /* v7 */
    case 0x3f:  *pm = "JEQ";   goto oper16p2; /* v7 */
    case 0x41:  *pm = "DEEKV"; goto oper8;    /* v7 */
    case 0x44:  *pm = "DOKEQ"; goto oper8;    /* v7 */
    case 0x46:  *pm = "POKEQ"; goto oper8;    /* v7 */
    case 0x48:  *pm = "MOVQB"; goto oper8x2r; /* v7 */
    case 0x4a:  *pm = "MOVQW"; goto oper8x2r; /* v7 */
    case 0x4d:  *pm = "JGT";   goto oper16p2; /* v7 */
    case 0x50:  *pm = "JLT";   goto oper16p2; /* v7 */
    case 0x53:  *pm = "JGE";   goto oper16p2; /* v7 */
    case 0x56:  *pm = "JLE";   goto oper16p2; /* v7 */
    case 0x66:  *pm = "ADDV";  goto oper8;    /* v7 */
    case 0x68:  *pm = "SUBV";  goto oper8;    /* v7 */
    case 0x6a:  *pm = "LDXW";  goto oper816;  /* v7 */
    case 0x6c:  *pm = "STXW";  goto oper816;  /* v7 */
    case 0x6e:  *pm = "LDSB";  goto oper8;    /* v7 */
    case 0x70:  *pm = "INCV";  goto oper8;    /* v7 */
    case 0x72:  *pm = "JNE";   goto oper16p2; /* v7 */
    case 0x78:  *pm = "LDNI";  goto oper8n;   /* v7 */
    case 0x7d:  *pm = "MULQ";  goto oper8;    /* v7 */
    case 0xb1:  *pm = "MOVIW"; goto oper16r8; /* v7 */
    case 0xbb:  *pm = "MOVW";  goto oper8x2r; /* v7 */
    case 0xd3:  *pm = "CMPWS"; goto oper8;    /* v7 */
    case 0xd6:  *pm = "CMPWU"; goto oper8;    /* v7 */
    case 0xd9:  *pm = "CMPIS"; goto oper8;    /* v7 */
    case 0xdb:  *pm = "CMPIU"; goto oper8;    /* v7 */
    case 0xdd:  *pm = "PEEKV"; goto oper8;    /* v7 */
    case 0xe1:  *pm = "PEEKA"; goto oper8;    /* v7 */
    default:    *pm = "???";   goto unknown;
    oper8:
      sprintf(operand, "$%02x", peek(addlo(addr,1)));
      return 2;
    oper8n:
      sprintf(operand, "$ff%02x", peek(addlo(addr,1)));
      return 2;
    oper16:
      sprintf(operand, "$%04x", deek(addlo(addr,1)));
      return 3;
    oper16r:
      sprintf(operand, "$%02x%02x", peek(addlo(addr,1)), peek(addlo(addr,2)));
      return 3;
    operbr:
      sprintf(operand, "$%04x", (addr&0xff00)|((peek(addlo(addr,1))+2)&0xff));
      return 2;
    oper16p2:
      sprintf(operand, "$%02x%02x", peek(addlo(addr,2)), ((peek(addlo(addr,1))+2)&0xff));
      return 2;
    oper8x2:
      sprintf(operand, "$%02x,$%02x", peek(addlo(addr,1)), peek(addlo(addr,2)));
      return 3;
    oper8x2r:
      sprintf(operand, "$%02x,$%02x", peek(addlo(addr,2)), peek(addlo(addr,1)));
      return 3;
    oper16r8:
      sprintf(operand, "$$%02x%02x,$%02x", 
              peek(addlo(addr,2)), peek(addlo(addr,3)), peek(addlo(addr,1)) );
      return 4;
    oper816:
      sprintf(operand, "$%02x,$%02x%02x", peek(addlo(addr,1)),
              peek(addlo(addr,3)), peek(addlo(addr,2)));
      return 4;
    unknown:
      sprintf(operand, "%02x%02x%02x%02x", peek(addlo(addr,0)), peek(addlo(addr,1)),
              peek(addlo(addr,2)), peek(addlo(addr,3)));
      return 2;
    }
}

void print_trace(CpuState *S)
{
  char operand[32];
  char *mnemonic = "???";
  word addr = deek(vPC);
  operand[0] = 0;
  disassemble(addr, &mnemonic, operand);
  fprintf(stderr, "%04x: [", addr);
  if (strchr(trace, 'n')) {
    fprintf(stderr, "  AC=%02x YX=%02x%02x vTicks=%d t=%+06ld%+06ld%+06ld\n\t",
            S->AC, S->Y, S->X, (int)(char)peek(0x15),
            (long)(rt-dt-vt[0]), (long)(t-rt-vt[1]), (long)(vt[0]+vt[1]));
  }
  fprintf(stderr, " vAC=%04x vLR=%04x SP=%04x", deek(vAC), deek(vLR), deek(SP));
  if (strchr(trace, 's'))
    fprintf(stderr, " vSP=%04x", deek(vSP));
  if (strchr(trace, 't'))
    fprintf(stderr, " T[0-3]=%04x %04x %04x %04x B[0-1]=%02x %02x",
            deek(T0), deek(T0+2), deek(T2), deek(T2+2),
            peek(B0), peek(B0+1));
  if (strchr(trace, 'f')) {
    int as = peek(LAC-3);
    int ae = peek(LAC-2);
    int64_t am = leek(LAC-1) + ((int64_t)peek(LAC+3) << 32);
    int be = peek(T2);
    int64_t bm = leek(T0) + ((int64_t)peek(T0+4) << 32);
    fprintf(stderr,
            "\n\t FAC=%02x/%02x/%010llx (%.8g) FARG=%02x/%010llx (%.8g)",
            as, ae, (long long)am, 
            ((as&0x80) ? -1.0 : +1.0) * ((ae) ? ldexp((double)am, ae-168) : 0.0),
            be, (long long)bm,
            ((as&0x80)^((as&1)<<7) ? -1.0 : +1.0) * ((be) ? ldexp((double)bm, be-168) : 0.0) );
  } else if (strchr(trace, 'l'))
    fprintf(stderr, "\n\t LAC=%08x LAX=%08x%02x",
            leek(LAC), leek(LAC), peek(LAC-1));
  if (strchr(trace, 'S')) {
    int i;
    fprintf(stderr, "\n\t sysFn=%04x sysArgs=%02x", deek(sysFn), peek(sysArgs0));
    for(i=1; i<=7; i++)
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
  }
  fprintf(stderr, " ]  %-5s %-18s\n",  mnemonic, operand);
}

void next_0x307(CpuState *S)
{
  if (nogt1) {
    if (trace)
      print_trace(S);
    if (pc2cycs && da)
      pc2cycs[da] += t - dt - vt[0] - vt[1];
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
            "  -v              print debug messages\n"
            "  -rom=<romfile>  load rom from <romfile>\n"
            "  -f              enable file system access\n"
            "  -nogt1          do not override main menu and run forever\n"
            "  -nogarble       do not garble memory to ensure repeatable runs\n"
            "  -vmode=<mode>   set video mode 0,1,2,3,1975\n"
            "  -t<letters>     trace VCPU execution\n"
            "  -prof=<fn>      writes profiling information into file <fn>\n");
    
  }
  exit(exitcode);
}


int optind;
char *optarg;

int optargcmp(int argc, char *argv[], const char *opt)
{
  int l = strlen(opt);
  char *arg = argv[optind];
  if (! strncmp(arg, opt, l)) {
    if (arg[l] == '=') {
      optarg = arg + l + 1;
      return 0;
    } else if (! arg[l]) {
      if (1 + optind >= argc)
        fatal("Missing argument for option %s\n", opt);
      optarg = argv[++optind];
      return 0;
    }
  }
  return -1;
}

int main(int argc, char *argv[])
{
  // Parse options
  for (optind=1; optind<argc; optind++)
    {
      char *arg = argv[optind];
      if (!strcmp(arg,"-h"))
        {
          usage(EXIT_SUCCESS);
        }
      else if (! strcmp(arg, "-nogt1"))
        {
          nogt1 = 1;
        }
      else if (! strcmp(arg, "-nogarble"))
        {
          nogarble = 1;
        }
      else if (! strcmp(arg, "-v"))
        {
          verbose = 1;
        }
      else if (! strncmp(arg, "-t", 2))
        {
          trace = arg+2;
        }
      else if (! strcmp(arg, "-f"))
        {
          okopen = 1;
        }
      else if (! optargcmp(argc, argv, "-prof"))
        {
          if (prof)
            fatal("Duplicate option -prof\n");
          setup_profile(optarg);
        }
      else if (! optargcmp(argc, argv, "-rom"))
        {
          if (rom)
            fatal("Duplicate option -rom\n");
          rom = optarg;
        }
      else if (! optargcmp(argc, argv, "-vmode"))
        {
          char *s = optarg;
          char *e = 0;
          vmode = strtol(s, &e, 0);
          if (e == s || *e)
            fatal("Invalid value '%s' for option -vmode\n", s);
          if (vmode!= 1975 && (vmode < 0 || vmode > 3))
            fatal("Invalid value '%s' for option -vmode\n", s);
        }
      else if (arg[0] == '-')
        {
          fatal("Unrecognized option %s\n", arg);
        }
      else
        {
          if (gt1)
            usage(EXIT_FAILURE);
          gt1 = arg;
        }
    }
  if (! gt1 && ! nogt1)
    usage(EXIT_FAILURE);
  if (! nogarble) {
    // Initialize with randomized data
    srand(time(NULL));
    garble((void*)ROM, sizeof ROM);
    garble((void*)RAM, sizeof RAM);
  }
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
