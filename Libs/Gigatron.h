
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

#define serialRaw               (*(byte*)0x000f)
#define vAC                     (*(word*)0x0018)
#define sysFn                   (*(word*)0x0022)
#define sysArgs                 ( (byte*)0x0024)
#define screenMemory            ( (byte*)0x0800)

#define font32up                ( (byte*)0x0700)
#define font82up                ( (byte*)0x0800)

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
 *  vReset() restarts the Gigatron and returns to the main menu
 */
#define vReset                  ((void(*)(void)) 0x1f0)

/*----------------------------------------------------------------------+
 |      Library definitions                                             |
 +----------------------------------------------------------------------*/

#define Indent 2                // Indent 2 pixels from the left

/*----------------------------------------------------------------------+
 |      Library data                                                    |
 +----------------------------------------------------------------------*/

extern int ScreenPos;
extern byte Color;
extern byte BgColor;

/*----------------------------------------------------------------------+
 |      Library functions                                               |
 +----------------------------------------------------------------------*/

void ClearScreen(void);
void Newline(void);
word Random(void);
byte WaitKey(void);

// Not implemented/decided
void GotoXY(byte x, byte y);
void PutChar(char c);
void Scroll(void);
void GetLine(void);
void GetChar(void);
void PrintDecimal(int n);
void PutString(char *c);
void DrawLine(int dx, int dy);

/*----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------*/

