
#include <Gigatron.h>

word Random(void)
{
  sysFn = SYS_Random_34;
  __syscall(34);
  return vAC;
}

