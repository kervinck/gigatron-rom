
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

#define SYS_VDrawBits_134       0x04e1
/*
  sysArgs[0]    Color 0 (background)
  sysArgs[1]    Color 1 (pen)
  sysArgs[2]    8 bits, highest bit first (destructive)
  sysArgs[4:5]  Position on screen
*/

/*----------------------------------------------------------------------+
 |                                                                      |
 +----------------------------------------------------------------------*/

