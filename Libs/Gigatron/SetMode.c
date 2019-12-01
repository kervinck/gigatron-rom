
#include <Gigatron.h>

void SetMode_v2(int mode)
{
  sysFn = SYS_SetMode_v2_80;
  vAC = mode;
  __syscall(253); // 270-34/2
}

