#define EXPECT 100

#ifdef TEST

#define sysFn ((unsigned*)(void*)0x0022)
#define sysArgsw ((unsigned*)(void*)0x0024)

int echo(int c) {
	return c;
}

void main() {
	sysArgsw[0] = echo(EXPECT);

	*sysFn = 0;
	__syscall(28);
}

#else

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "harness.h"

int main(int argc, char* argv[]) {
	char* logFile = NULL;
	if (argc > 1) {
		logFile = argv[1];
	}
    runTest("gigatron.rom", "echo.gt1", logFile, NULL);

	int fail = 0;
	uint16_t* sysArgsw = (uint16_t*)&RAM[0x0024];
	if (sysArgsw[0] != EXPECT) {
		fprintf(stderr, "expected sysArgs0 to be 0x%04x, found 0x%04x\n", EXPECT, sysArgsw[0]);
		fail = 1;
	}

	exit(fail ? EXIT_FAILURE : 0);
}

#endif
