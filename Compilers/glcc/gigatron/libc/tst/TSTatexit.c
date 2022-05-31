#include <stdio.h>
#include <stdlib.h>
#include <gigatron/libc.h>



#define handler(name,message)\
	void name(void) { printf("%s\n", message); }

handler(hnd0,"static")
handler(hnd1,"atexit1")
handler(hnd2,"atexit2")


DECLARE_FINI_FUNCTION(hnd0);


int main()
{
	printf("Registering atexit handlers\n");
	atexit(hnd1);
	atexit(hnd2);
	printf("Exiting\n");
	exit(0);
	return 0;
}
