
#include <Gigatron.h>

word ReadRomDir_DEVROM(word prev, char name[8])
{
  sysFn = 0xef;         // SYS_ReadRomDir_DEVROM_80;
  vAC = prev;
  __syscall(80);
  prev = vAC;
  memcpy(name, sysArgs, 8);
  return prev;
}

