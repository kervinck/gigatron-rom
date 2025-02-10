#include <stdlib.h>


/* Gigatron encoding:
   - 0x00-0x7f: ASCII
   - 0x80-0x83: Arrows U+2190 to U+2193
   Also supported here:
   - 0x84-0xff: Latin 1
*/


static wchar_t ctow(register int c)
{
	if ((c & 0xfc) == 0x80)
		c = 0x2190u + (c & 3);
	return c;
}

static int wtoc(register wchar_t w)
{
	if ((w & 0xfffcU) == 0x2190)
		return 0x80 | (w & 0x03);
	if ((w & 0xff00U) || ((w & 0xfc) == 0x80))
		return -1;
	return w;
}

int mblen(register const char *s, register size_t n)
{
	if (s == 0 || *s == 0)
		return 0;
	if (n <= 0)
		return -1;
	return 1;
}

int mbtowc(register wchar_t *pwc, register const char *s, register size_t n)
{
	if (n <= 0)
		return -1;
	if (s == 0 || *s == 0)
		return 0;
	if (pwc)
		*pwc = ctow(*s);
	return 1;
}

int wctomb(register char *s, register wchar_t wc)
{
	int c = wtoc(wc);
	if (s == 0)
		return 0;
	if (c < 0)
		return -1;
	*s = (char)c;
	return 1;
}


size_t mbstowcs(register wchar_t *d, register const char *s, register size_t n)
{
	size_t r = 0;
	if (s != 0) {
		while (*s && r < n) {
			if (d) { *d = ctow(*s); d++; }
			r += 1, s += 1;
		}
	}
	return r;
}

size_t wcstombs(char *d, const wchar_t *s, size_t n)
{
	size_t r = 0;
	if (s != 0) {
		while (*s && r < n) {
			int c = wtoc(*s);
			if (c < 0) { return (size_t)-1; }
			if (d) { *d++ = c; }
			r += 1, s += 1;
		}
	}
	return r;
}

