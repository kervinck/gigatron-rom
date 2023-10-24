#ifndef __CONIO
#define __CONIO



/* This file provides a semi-standard conio interface to the gigatron
   console. The output routines are thin wrappers to the
   gigatron/console.h ones. Both the conio.h input routines and the
   gigatron/console.h input routines use the low level driver selected
   in gigatron/kbget.h. */


/* ---- Conio output functions ---- */

/* The following functions are thin wrappers to the corresponding
   routines in gigatron/console.h. By default they only process
   the essential control characters BS, CR, and LF, but this can
   be expanded by redefining _console_ctrl whose prototype appears
   in gigatron/console.h */

/* Writes one character to the console. Return c. */
extern int putch(int c);

/* Writes one character string to the console. */
extern void cputs(const char *s);

/* Cursor position functions.
   Unlike the console_state variables cx and cy, these function
   address the top left screen corner as (1,1).
   The wherex/wherey functions return the current coordinates.
   The gotoxy function sets the current coordinates. */
void gotoxy(int x, int y);
int wherex(void);
int wherey(void);

/* Color functions.
   Either use one of the macros or use six bit gigatron colors. */
void textcolor(int color);
void textbackground(int color);

#define BLACK		(0x00)
#define BLUE		(0x20)
#define GREEN		(0x08)
#define CYAN		(0x28)
#define RED		(0x02)
#define MAGENTA		(0x22)
#define BROWN		(0x16)
#define LIGHTGRAY	(0x2a)
#define DARKGRAY	(0x15)
#define LIGHTBLUE	(0x3a)
#define LIGHTGREEN	(0x1d)
#define LIGHTCYAN	(0x3d)
#define LIGHTRED	(0x17)
#define LIGHTMAGENTA	(0x37)
#define YELLOW		(0x0f)
#define WHITE		(0x3f)

/* Clear screen and reset cursor */
void clrscr(void);

/* Clear from the cursor to the end of line */
void clreol(void);

/* Print at specific cursor position without moving the cursor. */
void cputsxy(int x, int y, const char *s);


/* ---- Formatted output functions ----*/

#include <gigatron/printf.h>


/* ---- Low level input routines ---- */

#include <gigatron/kbget.h>
   

/* ---- Conio input functions ---- */

/* Returns non zero when a following getch() will return immediately. */
extern int kbhit(void);

/* Returns an input code, waiting without screen feedback as necessary. */
extern int getch(void);

/* Puts a character back into the getch buffer.
   The next getch() will immediately return c.
   This function can only be called once before the next read.
   Returns c or EOF */
extern int ungetch(int c);

/* These functions work like getch() but provide feedback
   on the console by displaying a flashing square at the
   console cursor position and by printing the received
   character on the console.
     See also console_waitkey() defined in gigatron/console.h
   which displays the flashing cursor but does not echo
   the received character. */
extern int getche(void);

/* Gets a character string from the console and stores it in the
   character array pointed to by buffer. The array's first element,
   buffer[0], must contain the maximum length of the string to be
   read. The string actual length will be stored into buffer[1].
   Function cgets reads characters and stores them in the buffer
   starting at buffer[2] until it meets a line feed. It returns
   a pointer to buffer[2] which is where the string is stored.
     See also console_readline() defined in gigatron/console.h
   which takes different arguments and is used to implement cgets(). */
extern char *cgets(char *buffer);



/* ---- Conio alternate names ---- */

#define _putch   putch
#define _cputs   cputs
#define _cprintf cprintf
#define _kbhit   kbhit
#define _getch   getch
#define _getche  getche
#define _ungetch ungetch
#define _cgets   cgets


#endif
