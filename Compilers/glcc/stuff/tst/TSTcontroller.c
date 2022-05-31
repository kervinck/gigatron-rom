#include <stdlib.h>
#include <stdio.h>
#include <gigatron/console.h>
#include <gigatron/libc.h>
#include <gigatron/sys.h>




void prbyte(char x)
{
	char buffer[9];
	int b = x;
	int i;
	for (i=0; i!=8; i++) {
		b <<= 1;
		buffer[i] = '0' + ((b >> 8) & 1);
	}
	buffer[i] = 0;	
	cprintf(" 0x%02x\n 0b%s\n", x, buffer);
}
	

void main(void)
{
	for(;;) {
		console_state.cx = 0;
		console_state.cy = 0;
		cprintf("\nserialRaw\n");
		prbyte(serialRaw);
		cprintf("\nbuttonState\n");
		prbyte(buttonState);
	}
}
