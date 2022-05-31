
/*----------------------------------------------------------------------+
 |                                                                      |
 |      Gigatron.h -- Gigatron system definitions                       |
 |                                                                      |
 +----------------------------------------------------------------------*/

/*----------------------------------------------------------------------+
 |      Common types                                                    |
 +----------------------------------------------------------------------*/

typedef unsigned char  byte;
typedef unsigned short word;

/*----------------------------------------------------------------------+
 |      Definitions from interface.json                                 |
 +----------------------------------------------------------------------*/

enum {
  romTypeValue_ROMv1            = 0x1c,
  romTypeValue_ROMv2            = 0x20,
  romTypeValue_ROMv3            = 0x28,
  romTypeValue_ROMv4            = 0x38,
  romTypeValue_DEVROM           = 0xf8,
};

#define frameCount              (*(volatile byte*)0x000e)
#define serialRaw               (*(volatile byte*)0x000f)
#define buttonState             (*(volatile byte*)0x0011)
#define vAC                     (*(word*)0x0018)
#define romType                 (*(byte*)0x0021)
#define sysFn                   (*(word*)0x0022)
#define sysArgs                 ( (byte*)0x0024)

#define videoTable              ( (byte*)0x0100)
#define videoTop_DEVROM         (*(byte*)0x01f9)
#define screenMemory            ( (byte(*)[256])0x0800)

#define font32up                ( (byte*)0x0700)
#define font82up                ( (byte*)0x0800)

#define buttonRight             (255^1)
#define buttonLeft              (255^2)
#define buttonDown              (255^4)
#define buttonUp                (255^8)
#define buttonStart             (255^16)
#define buttonSelect            (255^32)
#define buttonB                 (255^64)
#define buttonA                 (255^128)

/*
 *  Draw 8 pixels vertically from bit mask
 *
 *  sysArgs[0]    Color 0 (background)
 *  sysArgs[1]    Color 1 (pen)
 *  sysArgs[2]    8 bits, highest bit first (destructive)
 *  sysArgs[4:5]  Position on screen
 */
#define SYS_VDrawBits_134       0x04e1

/*
 *  Update entropy[] and return 16 bits of it in vAC
 */
#define SYS_Random_34           0x04a7

/*
 *  Set video mode to value in vAC
 *       0..3   Full mode to fastest
 *       1975   Disable video loop ("zombie mode")
 *       -1     Enable video loop
 *  Requires ROMv2+
 */
#define SYS_SetMode_v2_80       0xb00

/*
 *  vReset() restarts the Gigatron and returns to the main menu
 */
#define vReset                  ((void(*)(void)) 0x1f0)

/*----------------------------------------------------------------------+
 |      Library definitions                                             |
 +----------------------------------------------------------------------*/

#define Indent 2                // Indent 2 pixels from the left

// TODO: add to interface.json
#define Black                   0x00
#define DarkRed                 0x01
#define Red                     0x02
#define LightRed                0x03
#define DarkGreen               0x04
#define Green                   0x08
#define LightGreen              0x0c
#define DarkBlue                0x10
#define Blue                    0x20
#define LightBlue               0x30
#define White                   0x3f

/*----------------------------------------------------------------------+
 |      Library data                                                    |
 +----------------------------------------------------------------------*/

extern int ScreenPos;           // Really a pointer into screen memory
extern byte Color;              // Pen color
extern byte BgColor;            // Background color

/*----------------------------------------------------------------------+
 |      Library functions                                               |
 +----------------------------------------------------------------------*/

void ClearScreen(void);
void Newline(void);
word Random(void);
byte WaitKey(void);
void BusyWait(int frames);
void PutChar(int c);
void SetMode_v2(int mode);
word ReadRomDir_DEVROM(word prev, char name[8]);

// Not implemented and/or decided:
void VideoScrollY(int dy);
char *IntToString(char s[7], int n);
char *UIntToString(char s[7], unsigned n);
void DrawLine(int dx, int dy);
//int scanf(const char *restrict format, ...);

/*----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------*/

