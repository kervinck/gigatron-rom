#include "stdlib.h"
#include "string.h"
#include "gigatron/console.h"
#include "gigatron/sys.h"

int main()
{
  // Display a red line at the top of the screen
  byte savedCtrlBits = ctrlBits_v5;
  SYS_ExpanderControl(0x7c);
  memset((void*)0x8200u, 0x3, 160);
  SYS_ExpanderControl(savedCtrlBits);
  
  // Print something
  cprintf("\n\nHello World\n\n");
  
  return 0;
}
