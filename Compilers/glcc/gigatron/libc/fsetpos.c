#include <stdio.h>

int fgetpos(FILE *fp, fpos_t *pp)
{
	if ((*pp = ftell(fp)) >= 0)
		return 0;
	return -1;
}

int fsetpos(FILE *fp, const fpos_t *pp)
{
	return fseek(fp, *pp, SEEK_SET);
}
