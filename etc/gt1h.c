// Gigatron-1

#include <string.h>

static char rcsid[] = "$Id$";

#ifndef LCCDIR
#define LCCDIR "/usr/local/lib/lcc/"
#endif

char *suffixes[] = { ".c", ".i", ".s", ".o", ".out", 0 };
char inputs[256] = "";
char *cpp[] = { LCCDIR "cpp", "$1", "$2", "$3", 0 };
char *com[] =  { LCCDIR "rcc", "-target=gt1", "$1", "$2", "$3", "", 0 };
char *include[] = { "-I" LCCDIR "include", 0 };
char *as[] = { "/bin/cp", "$2", "$3", 0 };
char *ld[] = { LCCDIR "gtlink.py", "-o", "$3", LCCDIR "rt.py", "$1", "$2", 0 };

extern char *concat(char *, char *);
extern int access(const char *, int);

int option(char *arg) {
	if (strncmp(arg, "-lccdir=", 8) == 0) {
		cpp[0] = concat(&arg[8], "/cpp");
		include[0] = concat("-I", concat(&arg[8], "/include"));
		com[0] = concat(&arg[8], "/rcc");
		ld[0] = concat(&arg[8], "/gtlink.py");
		ld[3] = concat(&arg[8], "/rt.py");
	} else {
		return 0;
	}
	return 1;
}
