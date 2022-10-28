#ifndef __GIGATRON_SYS
#define __GIGATRON_SYS


/* ---- Well known constants from interface.json ---- */

#define romTypeValue_ROMv1    0x1c
#define romTypeValue_ROMv2    0x20
#define romTypeValue_ROMv3    0x28
#define romTypeValue_ROMv4    0x38
#define romTypeValue_ROMv5    0x40
#define romTypeValue_DEVROM   0xf8

#define qqVgaWidth            160
#define qqVgaHeight           120

#define buttonRight           1
#define buttonLeft            2
#define buttonDown            4
#define buttonUp              8
#define buttonStart           16
#define buttonSelect          32
#define buttonB               64
#define buttonA               128

typedef struct channel_s {
	char wavA, wavX;
	char keyL, keyH;
	char oscL, oscH;
} channel_t;

/* ---- Well known rom locations from interface.json ---- */

#define font32up              0x0700
#define font82up              0x0800
#define notesTable            0x0900
#define invTable              0x0a00

/* ---- Well known memory locations from interface.json ---- */

#ifdef __CHAR_UNSIGNED__
# define byte char
# define word unsigned int
#else
# define byte unsigned char
# define word unsigned int
#endif

/* Using macro definitions like this is not very
 * good because it prevents us from declaring local
 * variables with the same name. Best would be 
 * to augment LCC with a way to declare placed
 * global variables (another project).
 */

#define zeroConst             (*(byte*)(0x0000)) // byte zeroConst;
#define memSize               (*(byte*)(0x0001)) // byte memSize;
#define entropy      ( (volatile byte*)(0x0006)) // byte entropy[3];
#define videoY       (*(volatile byte*)(0x0009)) // byte videoY;
#define frameCount   (*(volatile byte*)(0x000e)) // byte frameCount;
#define serialRaw    (*(volatile byte*)(0x000f)) // byte serialRaw;
#define buttonState  (*(volatile byte*)(0x0011)) // byte buttonState;
#define xoutMask              (*(byte*)(0x0014)) // byte xoutMask;
#define vPC          (*(volatile word*)(0x0016)) // word vPC;
#define vPCL         (*(volatile byte*)(0x0016)) // byte vPCL;
#define vPCH         (*(volatile byte*)(0x0017)) // byte vPCH;
#define vAC          (*(volatile word*)(0x0018)) // word vAC;
#define vACL         (*(volatile byte*)(0x0018)) // byte vACL;
#define vACH         (*(volatile byte*)(0x0019)) // byte vACH;
#define vLR                   (*(word*)(0x001a)) // word vLR;
#define vLRL                  (*(byte*)(0x001a)) // byte vLRL;
#define vLRH                  (*(byte*)(0x001b)) // byte vLRH;
#define vSP                   (*(byte*)(0x001c)) // byte vSP;
#define romType               (*(byte*)(0x0021)) // byte romType;
#define channelMask_v4        (*(byte*)(0x0021)) // byte channelMask_v4;
#define sysFn                 (*(word*)(0x0022)) // word sysFn;
#define sysArgs0              (*(byte*)(0x0024)) // byte sysArgs0;
#define sysArgs1              (*(byte*)(0x0025)) // byte sysArgs1;
#define sysArgs2              (*(byte*)(0x0026)) // byte sysArgs2;
#define sysArgs3              (*(byte*)(0x0027)) // byte sysArgs3;
#define sysArgs4              (*(byte*)(0x0028)) // byte sysArgs4;
#define sysArgs5              (*(byte*)(0x0029)) // byte sysArgs5;
#define sysArgs6              (*(byte*)(0x002a)) // byte sysArgs6;
#define sysArgs7              (*(byte*)(0x002b)) // byte sysArgs7;
#define soundTimer            (*(byte*)(0x002c)) // byte soundTimer;
#define ledState_v2           (*(byte*)(0x002e)) // byte ledState;
#define ledTempo              (*(byte*)(0x002f)) // byte ledTempo;
#define userVars              ( (byte*)(0x0030)) // byte *userVars;
#define oneConst              (*(byte*)(0x0080)) // byte oneConst;
#define userVars2             ( (byte*)(0x0081)) // byte *userVars2;
#define v6502_PC              (*(word*)(0x001a)) // word v6502_PC;
#define v6502_PCL             (*(byte*)(0x001a)) // byte v6502_PCL;
#define v6502_PCH             (*(byte*)(0x001b)) // byte v6502_PCH;
#define v6502_A               (*(byte*)(0x0018)) // byte v6502_A;
#define v6502_X               (*(byte*)(0x002a)) // byte v6502_X;
#define v6502_Y               (*(byte*)(0x002b)) // byte v6502_Y;

#define videoTable            ( (byte*)(0x0100)) // byte *videoTable;
#define vReset         ((void(*)(void))(0x01f0)) // void  vReset(void);
#define vIRQ_v5               (*(word*)(0x01f6)) // word  vIRQ_v5;
#define videoTop_v5           (*(byte*)(0x01f9)) // byte  videoTop_v5;
#define userCode              ( (void*)(0x0200)) // void *userCode;
#define soundTable            ( (void*)(0x0700)) // byte  soundTable[]
#define screenMemory    ((byte(*)[256])(0x0800)) // byte  screenMemory[][256]

#define channel1         (*(channel_t*)(0x01fa)) // chanel_t channel1
#define channel2         (*(channel_t*)(0x02fa)) // chanel_t channel2
#define channel3         (*(channel_t*)(0x03fa)) // chanel_t channel3
#define channel4         (*(channel_t*)(0x04fa)) // chanel_t channel4
#define channel(c)       (*(channel_t*)(((1+((c-1)&0xff))<<8)|0xfa))

/* ---- Unofficial memory locations ---- */

#define ctrlBits_v5           (*(byte*)(0x01f8))  // byte  ctrlBits_v5;


/* ---- Calling SYS functions ---- */

/* All stubs are in gigatron/libc/gigatron.s */

/* -- SYS_Lup -- */
int SYS_Lup(unsigned int addr);
#define has_SYS_Lup() 1

/* -- SYS_Random -- */
unsigned int SYS_Random(void);
#define has_SYS_Random() 1

/* -- SYS_VDrawBits -- */
void SYS_VDrawBits(int fgbg, char bits, char *address);
#define has_SYS_VDrawBits() 1

/* -- SYS_Exec */
void SYS_Exec(void *romptr, void *vlr);
#define has_SYS_Exec() 1

/* -- SYS_SetMode */
void SYS_SetMode(int);
#define has_SYS_SetMode 1

/* -- SYS_ReadRomDir
   Notes: the name is copied into buf8 */
void* SYS_ReadRomDir(void *romptr, char *buf8);
#define has_SYS_ReadRomDir() \
	((romType & 0xfc) >= romTypeValue_ROMv5)

/* -- SYS_ExpanderControl --
   Notes: Calling this from C is risky.
   Notes: This exists in v4 but overwrites 0x81 with ctrlBits. 
   Notes: We depend on ctrlBits being nonzero when an expansion card is present. */
int SYS_ExpanderControl(unsigned int ctrl);
#define has_SYS_ExpanderControl() \
	(((romType & 0xfc) >= romTypeValue_ROMv5) && (ctrlBits_v5 != 0))

/* -- SYS_SpiExchangeBytes --
   Notes: This exists in v4 but depends on 0x81 containing ctrlBits.
   Notes: only the high 8 bits of `dst` are used.
   Notes: only the low 8 bits of `srcend` are used. */
void SYS_SpiExchangeBytes(void *dst, void *src, void *srcend);
#define has_SYS_SpiExchangeBytes() \
	(((romType & 0xfc) >= romTypeValue_ROMv5) && (ctrlBits_v5 != 0))



#endif
