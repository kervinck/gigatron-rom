#include "_stdio.h"

int fgetc(register FILE *fp) {
	return getc(fp);
}
