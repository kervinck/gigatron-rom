#ifndef __CTYPE
#define __CTYPE

/* Table based ctype does not make sense in the gigatron 
   because it requires too much contiguous memory. */

extern int isascii(int);
extern int isalnum(int);
extern int isalpha(int);
extern int iscntrl(int);
extern int isdigit(int);
extern int isgraph(int);
extern int islower(int);
extern int isprint(int);
extern int ispunct(int);
extern int isspace(int);
extern int isupper(int);
extern int isxdigit(int);
extern int tolower(int);
extern int toupper(int);

#define isascii(c)      (!((c)&~0x7fU))

/* Macro alternatives that might evaluate c multiple times. 
   Using a-b?0 instead of a?b to use SUBI over _CMPWI */

#define	_isalpha(c)	((int)(((c)|0x20)-'a')>=0 && (int)(((c)|0x20)-'z')<=0)
#define	_isdigit(c)	((int)((c)-'0')>=0 && (int)((c)-'9')<=0)
#define	_islower(c)	((int)((c)-'a')>=0 && (int)((c)-'z')<=0)
#define	_isspace(c)	((c)==32 || (int)((c)-9)>=0 && (int)((c)-13)<=0)
#define	_isupper(c)	((int)((c)-'A')>=0 && (int)((c)-'Z')<=0)
#define	_isxdigit(c)	(_isdigit(c) || (int)(((c)|0x20)-'a')>=0 && (int)(((c)|0x20)-'z')<=0)

#endif /* __CTYPE */
