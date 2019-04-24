
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

#define sysFn                   (*(word*)0x0022)
#define sysArgs                 ( (byte*)0x0024)
#define screenMemory            ( (byte*)0x0800)

#define font32up                ( (byte*)0x0700)
#define font82up                ( (byte*)0x0800)

#define SYS_VDrawBits_134       0x04e1
/*
  sysArgs[0]    Color 0 (background)
  sysArgs[1]    Color 1 (pen)
  sysArgs[2]    8 bits, highest bit first (destructive)
  sysArgs[4:5]  Position on screen
*/

/*
 *  vReset() restarts the Gigatron and returns to the main menu
 */
#define vReset                  ((void(*)(void)) 0x1f0)

/*----------------------------------------------------------------------+
 |      Library functions                                               |
 +----------------------------------------------------------------------*/

#define _Indent 2
extern int _ScreenPos = 0;

void ClearScreen(byte color);
void Newline(void);

// Not implemented/decided
void SetColor(byte color);
void SetBgColor(byte color);
void GotoXY(byte x, byte y);
void PutChar(char c);            // Needed?
void Scroll(void);
void GetLine(void);
void GetChar(void);
void PrintDecimal(int n);
void PutString(char *c);
void DrawLine(int dx, int dy);
word Random16(void);

/*----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------*/

