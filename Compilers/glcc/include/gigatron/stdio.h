#ifndef __STDIO
#define __STDIO

#if !defined(_SIZE_T) && !defined(_SIZE_T_) && !defined(_SIZE_T_DEFINED)
#define _SIZE_T
#define _SIZE_T_
#define _SIZE_T_DEFINED
typedef unsigned int size_t;
#endif

#if !defined(_VA_LIST) && !defined(_VA_LIST_DEFINED)
#define _VA_LIST
#define _VA_LIST_DEFINED
typedef char *__va_list;
#endif

#if !defined(_FPOS_T) && !defined(_FPOS_T_) && !defined(_FPOS_T_DEFINED)
#define _FPOS_T
#define _FPOS_T_
#define _FPOS_T_DEFINED
typedef long fpos_t;
#endif

#ifndef NULL
#define NULL ((void*)0)
#endif

#define _IOFBF   0x40  /* buffered       (needed by c89) */
#define _IONBF   0x80  /* not buffered   (needed by c89) */
#define _IOLBF   0xC0  /* line buffered. (needed by c89) */
#define _IOUNGET 0x10  /* pending ungetc */
#define	_IOEOF   0x08  /* eof flag       */
#define	_IOERR   0x04  /* error flag     */
#define _IORW    0x03  /* r+,w+,a+       */
#define _IOREAD  0x01  /* r              */
#define _IOWRIT  0x02  /* w,a            */

#define FILE         struct _iobuf
#define FILENAME_MAX 256
#define FOPEN_MAX    10
#define BUFSIZ       512     /* nop */
#define EOF          (-1)    /* eof */
#define L_tmpnam     25
#define SEEK_CUR     1
#define SEEK_END     2
#define SEEK_SET     0
#define TMP_MAX      17576

extern struct _iobuf {
	char _flag;		/* flag */
	char _unget;	        /* ungetc buffer */
	struct _iovec *_v;	/* vtable */
	void *_x;		/* more data */
} _iob[];

struct _iovec {
	int  (*read)(FILE *fp, char *buf, size_t cnt);
	int  (*write)(FILE *fp, const char *buf, size_t cnt);
	int  (*flush)(FILE *fp, int close);
	long (*lseek)(FILE *fp, long off, int whence);
};

#define stderr (&_iob[2])
#define stdin  (&_iob[0])
#define stdout (&_iob[1])

extern int remove(const char *);
extern int rename(const char *, const char *);
extern FILE *tmpfile(void);
extern char *tmpnam(char *);
extern int fclose(FILE *);
extern int fflush(FILE *);
extern FILE *fopen(const char *, const char *);
extern FILE *freopen(const char *, const char *, FILE *);
extern void setbuf(FILE *, char *);
extern int setvbuf(FILE *, char *, int, size_t);
extern int fprintf(FILE *, const char *, ...);
extern int fscanf(FILE *, const char *, ...);
extern int printf(const char *, ...);
extern int scanf(const char *, ...);
extern int sprintf(char *, const char *, ...);
extern int sscanf(const char *, const char *, ...);
extern int vfprintf(FILE *, const char *, __va_list);
extern int vprintf(const char *, __va_list);
extern int vsprintf(char *, const char *, __va_list);
extern int fgetc(FILE *);
extern char *fgets(char *, int, FILE *);
extern int fputc(int, FILE *);
extern int fputs(const char *, FILE *);
extern int getc(FILE *);
extern int getchar(void);
extern char *gets(char *);
extern int putc(int, FILE *);
extern int putchar(int);
extern int puts(const char *);
extern int ungetc(int, FILE *);
extern size_t fread(void *, size_t, size_t, FILE *);
extern size_t fwrite(const void *, size_t, size_t, FILE *);
extern int fgetpos(FILE *, fpos_t *);
extern int fseek(FILE *, long int, int);
extern int fsetpos(FILE *, const fpos_t *);
extern long int ftell(FILE *);
extern void rewind(FILE *);
extern void clearerr(FILE *);
extern int feof(FILE *);
extern int ferror(FILE *);
extern void perror(const char *);

#define getc(p)     (fgetc(p))
#define putc(x,p)   (fputc(x,p))
#define getchar()   (fgetc(stdin))
#define putchar(x)  (fputc(x,stdout))
#define ferror(p)   ((p)->_flag & _IOERR)
#define feof(p)     ((p)->_flag & _IOEOF)
#define clearerr(p) ((p)->_flag &= (0xff ^ _IOERR ^ _IOEOF))

#endif /* __STDIO */
