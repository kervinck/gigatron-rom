// Gigatron-1

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#ifdef __DATE__
static char rcsid[] = __DATE__;
#else
static char rcsid[] = "<unknown>";
#endif

#ifndef PREFIX
# define PREFIX "/usr/local"
#endif
#ifndef LCCDIR
# define LCCDIR PREFIX "/lib/gigatron-lcc/"
#endif


char *suffixes[] = { ".c", ".i", ".s", ".o;.s;.a", ".gt1", 0 };
char inputs[256] = "";

char *cpp[] = { LCCDIR "cpp", "-D__gigatron", "-D__gigatron__", "-D__CHAR_UNSIGNED__", "$1", "$2", "$3", 0 };
char *com[] =  { LCCDIR "rcc", "-target=gigatron", "-cpu=5", "$1", "$2", "$3", "", 0 };
char *include[] = { "-I" LCCDIR "include", 0 };
char *as[] = { "/bin/cp", "$2", "$3", 0 };
char *ld[] = { LCCDIR "glink", "-cpu=5", "-rom=v5a", "-map=32k", "-o", "$3", "$1", "$2", "-lc", 0 };

extern char *concat(char *, char *);
extern int access(const char *, int);

int option(char *arg) {
	if (strncmp(arg, "-lccdir=", 8) == 0) {
		putenv(concat("LCCDIR=", &arg[8]));
		cpp[0] = concat(&arg[8], "/cpp");
		include[0] = concat("-I", concat(&arg[8], "/include"));
		com[0] = concat(&arg[8], "/rcc");
		ld[0] = concat(&arg[8], "/glink");
	} else if (strncmp(arg, "-cpu=", 5) == 0) {
		ld[1] = com[2] = concat("-cpu=", &arg[5]);
	} else if (strncmp(arg, "-rom=", 5) == 0) {
		ld[2] = concat("-rom=", &arg[5]);
	} else if (strncmp(arg, "-map=", 5) == 0) {
		ld[3] = concat("-map=", &arg[5]);
	} else
		return 0;
	return 1;
}
