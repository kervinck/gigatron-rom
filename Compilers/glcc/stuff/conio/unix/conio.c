#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>

#include "conio.h"

static int bgc = 40;
static int fgc = 40;
static int gch = EOF;
static struct termios oldt;
static int inited = 0;
static int oldf;


static void exitf(void)
{
	if (inited) {
		inited = 0;
		fcntl(STDIN_FILENO, F_SETFL, oldf);
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
		printf("\033c\033[m\033[?25h");
		fflush(stdout);
	}
}

static void sigint(int x)
{
	exit(10);
}

static void init(void)
{
	struct termios newt;
	if (! inited) {
		tcgetattr(STDIN_FILENO, &oldt);
		oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
		atexit(exitf);
		signal(SIGINT,sigint);
		signal(SIGTERM,sigint);
		inited = 1;
		newt = oldt;
		cfmakeraw(&newt);
		newt.c_lflag &= ~(ICANON|ECHO);
		newt.c_lflag |= ISIG;
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);
		printf("\033[?25l");
		fflush(stdout);
	}
}

void clreol()
{
	printf("\033[K");
}

void gotoxy(int x,int y)
{
	printf("\033[%d;%df",y,x);
}

void clrscr()
{
	printf("\033[2J\033[1;1f");
}

static int ansicolor(int color)
{
	switch(color){
	case BLACK:          return 0;
	case BLUE:           return 4;
	case GREEN:          return 2;
	case CYAN:           return 6;
	case RED:            return 1;
	case MAGENTA:        return 5;
	case BROWN:          return 3;
	case LIGHTGRAY:      return 7;
	case DARKGRAY:       return 0;
	case LIGHTBLUE:      return 4;
	case LIGHTGREEN:     return 2;
	case LIGHTCYAN:      return 6;
	case LIGHTRED:       return 1;
	case LIGHTMAGENTA:   return 5;
	case YELLOW:         return 3;
	case WHITE:          return 7;
	default:             return -1;
	}
}

void textbackground(int color)
{
	if ((color = ansicolor(color)) >= 0) {
		init();
		bgc = color + 40;
		printf("\033[0;%d;%dm",fgc,bgc);
	}
}

void textcolor(short color)
{  
	if ((color = ansicolor(color)) >= 0) {
		init();
		fgc = color + 30;
		printf("\033[0;%d;%dm",fgc,bgc);
	}
}

int ungetch(int ch)
{
	gch = ch;
	return gch;
}

int kbhit()
{
	int ch;
	init();
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	if (ch == EOF) {
		clearerr(stdin);
		return 0;
	} else {
		gch = ch;
		return 1;
	}
}

int getch()
{
	int ch = gch;
	gch = EOF;
	if (ch == EOF) {
		init();
		ch = getchar();
	}
	return ch;
}

int getche()
{
	int ch = gch;
	gch = EOF;
	if (ch == EOF) {
		init();
		fflush(stdin);
		printf("\033[?25h");
		ch = getchar();
		if (ch != EOF)
			putchar(ch);
		printf("\033[?25l");
	}
	return ch;
}

static void wherexy(int*x,int*y)
{
	int lx = 0;
	int ly = 0;
	int in;
	init();
	fflush(stdin);
	printf("\033[6n");
	fflush(stdout);
	if (getchar() != '\033')
		return;
	if (getchar() != '[')
		return;
	while((in = getchar()) && isdigit(in))
		ly = ly * 10 + in - '0';
	if (in != ';')
		return;
	while((in = getchar()) && isdigit(in))
		lx = lx * 10 + in - '0';
	if (in != 'R')
		return;
	*x = lx;
	*y = ly;
}

int wherex()
{
	int x = 0, y = 0;
	wherexy(&x,&y);
	return x;
}

int wherey()
{
	int x = 0, y = 0;
	wherexy(&x,&y);
	return y;
} 

int putch(const char c)
{
	printf("%c",c);
	return (int)c;
}

int cputs(const char*str)
{
	printf("%s",str);
	return 0;
}

char *cgets (char *buf)
{
	struct termios newt;
	char *s = 0;
	int m = (int)(unsigned char)buf[0] - 3;
	buf[1] = 0;
	if (m >= 0) {
		init();
		printf("\033[?25h");
		tcgetattr(STDIN_FILENO, &newt);
		tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
		fflush(stdin);
		if (gch != EOF)
			ungetc(gch, stdin);
		s = fgets(buf+2, m, stdin);
		tcsetattr(STDIN_FILENO, TCSANOW, &newt);
		if (ferror(stdin))
			return 0;
		if (s)
			buf[1] = strlen(s);
	}
	return buf+2;
}

