
#include <stdlib.h>
#include <stdio.h>
#include <gigatron/libc.h>


#define TSTU(x,r) \
	printf("utoa(%u,buffer,%d)=\"%s\"\n", (unsigned)(x), (r), utoa((x), buffer, r)); 
#define TSTI(x,r) \
	printf("itoa(%d,buffer,%d)=\"%s\"\n", (x), (r), itoa((x), buffer, r)); 
#define TSTUL(x,r) \
	printf("ultoa(%lu,buffer,%d)=\"%s\"\n", (unsigned long)(x), (r), ultoa((x), buffer, r)); 
#define TSTL(x,r) \
	printf("ltoa(%ld,buffer,%d)=\"%s\"\n", (long)(x), (r), ltoa((x), buffer, r)); 


int main()
{
	char buffer[16];
	TSTU(0,10);
	TSTU(34566u,10);
	TSTU(65520u,16);
	TSTU(34566u,8);
	TSTU(67u,8);
	
	TSTI(0,10);
	TSTI(3345,10);
	TSTI(-16385,16);
	TSTI(-16385,10);
	TSTI(672,8);

	TSTUL(0,10);
	TSTUL(45,10);
	TSTUL(12034,10);
	TSTUL(12034056,10);
	TSTUL(12034056,16);
	TSTUL(0xfffffffful,10);
	
	TSTL(0,10);
	TSTL(-45,10);
	TSTL(12034,10);
	TSTL(-12034056,10);
	TSTL(12034056,16);
	TSTL(0x7ffffffful,10);
	TSTL((long)0x80000000ul,10);	

	return 0;
}
