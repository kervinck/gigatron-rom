#include "_stdio.h"

int (getc)(register FILE *fp) {
	return getc(fp);
}

int (putc)(register int c, register FILE *fp) {
	return putc(c, fp);
}

int (putchar)(register int c) {
	return putchar(c);
}

int (getchar)(void) {
	return getchar();
}

int (ferror)(FILE *fp) {
	return ferror(fp);
}

int (feof)(FILE *fp) {
	return feof(fp);
}

void (clearerr)(FILE *fp) {
	clearerr(fp);
}
