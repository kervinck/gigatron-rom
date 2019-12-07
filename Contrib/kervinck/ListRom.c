
#include <stdio.h>
#include <Gigatron.h>

static char name[9];
static word addr;

int main(void)
{
  if (romType < romTypeValue_DEVROM)
    return 1;

  addr = ReadRomDir_DEVROM(0, &name[0]);
  while (addr != 0) {
    printf("%5u %s\n", addr, name);
    addr = ReadRomDir_DEVROM(addr, &name[0]);
  }
  return 0;
}

