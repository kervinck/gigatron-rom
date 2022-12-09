#include <string.h>
#include <stdio.h>
#include <signal.h>

int a = 3;
long b = 323421L;
volatile int vblcount = 0;
extern char frameCount;

int handler(int signo, int fpeinfo)
{
	printf("handle %d %d\n", signo, fpeinfo);
	return 1234;
}

long lhandler(int signo, int fpeinfo)
{
	printf("handle %d %d\n", signo, fpeinfo);
	return 1234L;
}

int main()
{
	signal(SIGFPE, (sig_handler_t)handler);
	printf("%d/0 = %d\n", a, a / 0);
	signal(SIGFPE, (sig_handler_t)lhandler);
	printf("%ld/0 = %ld\n", b , b / 0);
	return 0;
}
