

#ifndef __CONIO
#define __CONIO

#include <stdio.h>

#define BLACK        0
#define BLUE         1
#define GREEN        2
#define CYAN         3
#define RED          4
#define MAGENTA      5
#define BROWN        6
#define LIGHTGRAY    7
#define DARKGRAY     8
#define LIGHTBLUE    9
#define LIGHTGREEN   10
#define LIGHTCYAN    11
#define LIGHTRED     12
#define LIGHTMAGENTA 13
#define YELLOW       14
#define WHITE        15
#define BLINK        128


#ifdef __cplusplus
extern "C" {
#endif
  extern void clreol();
  extern void gotoxy(int x,int y);
  extern void clrscr();
  extern void textbackground(int color);
  extern void textcolor(short color);
  extern int ungetch(int ch);
  extern int getch();
  extern int getche();
  extern int wherex();
  extern int wherey();
  extern int kbhit();
  extern int putch(const char c);
  extern int cputs(const char*str);
  extern char *cgets(char*buf);
#ifdef __cplusplus
}
#endif
  
#define cprintf printf
#define cscanf scanf
#define _cprintf        cprintf
#define _cscanf         cscanf
#define _cputs          cputs
#define _getche         getche
#define _kbhit          kbhit
#define _putch          putch
#define _ungetch        ungetch

#endif

