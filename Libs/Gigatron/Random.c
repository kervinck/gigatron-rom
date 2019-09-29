
#include <Gigatron.h>

word Random(void)
{
  sysFn = SYS_Random_34;
  __syscall(253); // 270-34/2
  return vAC;
}

