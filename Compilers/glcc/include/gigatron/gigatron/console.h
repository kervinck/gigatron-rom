#ifndef __GIGATRON_CONSOLE
#define __GIGATRON_CONSOLE


/* ---- Console state ---- */

#ifndef CONSOLE_MAX_LINES
# define CONSOLE_MAX_LINES 15
#endif

/* Console geometry. */
extern const struct console_info_s {
	int nlines;		                  /* number of lines   */
	int ncolumns;                             /* number of columns */
	unsigned char offset[CONSOLE_MAX_LINES];  /* line offsets      */
} console_info;

/* Console state: colors, cursor, wrapping and scrolling modes.
   These fields can be changed as needed between calls to console functions. */
extern __near struct console_state_s {
	int  fgbg;		/* fg and bg colors   */
	char cy, cx;		/* cursor coordinates */
	char wrapy, wrapx;	/* wrap/scroll enable */
} console_state;

#define console_state_set_cycx(cycx) \
	*(unsigned*)&console_state.cy = (cycx)
#define console_state_set_wrap(wrap) \
	*(unsigned*)&console_state.wrapy = (wrap)



/* ---- Console output ---- */

#define CONSOLE_DEFAULT_FGBG 0x3f20

/* Print up to len characters of the zero terminated string s.
   Understand essential control characters "\b\n\r". More control
   characters can be supported by forcing _console_ctrl to be included
   (as with stdio output) in the build or by defining a customized
   one.  Return the number of characters processed. */
extern int console_print(const char *s, unsigned int len);

/* Works like console_print but prints exactly len characters
   regardless of the presence of a null character. */
extern int console_writall(const char *s, unsigned int len);

/* Reset the video tables and clear the screen. */
extern void console_clear_screen(void);

/* Clear screen from the cursor to the end of line */
extern void console_clear_to_eol(void);


/* ---- Formatted output functions ----*/

#include <gigatron/printf.h>


/* ---- Low level input routines ---- */

#include <gigatron/kbget.h>


/* ---- Console input routines ---- */


/* Returns an input code or -1 if no key is pressed.
   This function merely calls (*kbget)(void). */
extern int console_getkey(void);

/* Wait for a key press with a flashing cursor. */
extern int console_waitkey(void);

/* Input a line with rudimentary editing.
   The resulting characters, including the final newline and a zero
   terminator, are stored into the specified buffer and are guaranteed
   not to exceed the specified size. This function returns the number
   of characters read. */
extern int console_readline(char *buffer, int bufsiz);


/* -------- internal ----------- */

/* Compute the screen address of the console cursor.
   Wrap the line if console_state.cx points beyond the screen and
   console_state.wrapx is nonzero. Scroll the screen if
   console_state.cy points beyond the screen and console_state.wrapy
   is nonzero. Return zero if the cursor is still beyond the
   screen. */
extern char *_console_addr(void);

/* Draws up to `len` characters from string `s` at the screen position
   given by address `addr`.  This assumes that the horizontal offsets
   in the screen table are all zero. All characters are printed on a
   single line (no newline).  The function returns when any of the
   following conditions is met: (1) `len` characters have been
   printed, (2) the next character would not fit horizontally on the
   screen, or (3), an unprintable character has been met. */
extern int _console_printchars(int fgbg, const char *addr, const char *s, int len);

/* Clear with color clr from screen address addr to the end of the row.
   Repeats for nl successive lines. */
extern void _console_clear(char *addr, int clr, int nl);

/* Handle additional control characters in _console_print().
   Override this function to implement more control characters.
   The default version, included when stdio is active, understands
   characters "\t" for tabulation (4 chars) "\f" for clearing the
   screen, "\v" for clearing to the end of the line, and "\a" for an
   audible bell. Return the number of characters consumed. */
extern int _console_ctrl(int c);

/* Sounds the bell for n frames */
extern void _console_bell(int n);

/* Reset videotable and optionally clear screen if fgbg >= 0.
   This is called by _console_setup and console_clear_screen. */
extern void _console_reset(int fgbg);

/* Initialization: called before main */
extern void _console_setup(void);


/* -------- extra ------------ */

/* The following functions are useful for scrolling line ranges.
   They are not used by console_print but are still available for
   backward compatibility. */

/* Clear character row y */
extern void console_clear_line(int y);

/* Rotate rows [y1,y2) by n positions */
extern void console_scroll(int y1, int y2, int n);


#endif
