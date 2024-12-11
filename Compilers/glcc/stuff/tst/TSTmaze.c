#include <gigatron/sys.h>
#include <gigatron/console.h>


char * __near addr = (char*)160;

void print_fslash(void)
{
	int b;
	for (b = 1; (char)b; b <<= 1, addr++) 
		SYS_VDrawBits(CONSOLE_DEFAULT_FGBG, b, addr); 
}

void print_bslash(void)
{
	int b;
	char *p = addr + 8;
	addr = p;
	for (b = 1; (char)b; b <<= 1) 
		SYS_VDrawBits(CONSOLE_DEFAULT_FGBG, b, --p);
}

void scroll(void)
{
	byte *p = videoTable;
	byte b = *p;
	((byte*)&addr)[1] = b;
	((byte*)&addr)[0] = 0;
	for(; p != videoTable + 112 * 2; p += 2)
		p[0] = p[16];
	for(; p != videoTable + 120 * 2; p += 2, b++)
		p[0] = b;
}

void main(void)
{
	while(1) {
		if ((char)(unsigned)addr == 160) {
			scroll();
			_console_clear(addr, CONSOLE_DEFAULT_FGBG, 8);
		}
		if ((int) SYS_Random() < 0)
			print_fslash();
		else
			print_bslash();
	}
}
