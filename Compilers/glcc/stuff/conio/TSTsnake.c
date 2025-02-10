/* Snake -- Inspired by petersieg */

#include <stdlib.h>
#include <string.h>
#include <conio.h>

#ifdef __gigatron

# include <gigatron/sys.h>

/* Field size */
# define FW 26
# define FH 14

/* Keys */
# define KEY_RIGHT (0xff ^ buttonRight)
# define KEY_UP    (0xff ^ buttonUp)
# define KEY_LEFT  (0xff ^ buttonLeft)
# define KEY_DOWN  (0xff ^ buttonDown)

/* For page zero variables */ 
# define near __near

/* For timing */
# define clock_t signed char
# define CLOCKS_PER_SEC 60
# define clock() frameCount

# pragma glcc option("PRINTF_SIMPLE")
# pragma glcc option("KBGET_AUTOBTN")

#else /* ! __gigatron */

# include <time.h>
# define FW 80
# define FH 23
# define CHARS " :#*+O0"
# define KEY_RIGHT 'd'
# define KEY_UP    'w'
# define KEY_LEFT  'a'
# define KEY_DOWN  's'
# define near

#endif /* ! __gigatron */


/* State variables */
static char field[FH][FW];
near int foodLeft;
near int hy, hx, ty, tx;
near int score;

enum what_s {
	blank  = 0x0,
	wall   = 0x1,
	poison = 0x2,
	food   = 0x3,
	tail   = 0x4,
	head   = 0x5,
	corpse = 0x6
};

#ifdef CHARS
static const char chars[] = CHARS;
static const char colors[] = { BLUE, LIGHTGRAY, RED, YELLOW, GREEN, GREEN, RED }; 
#else
static const char chars[] = " \x7f#*+O0";
static const char colors[] = { BLUE, LIGHTGRAY, RED, YELLOW, GREEN, GREEN, RED }; 
#endif

static void addStuff(int n, int stuff)
{
	while (n > 0) {
		int x = rand() % FW;
		int y = rand() % FH;
		if (field[y][x])
			continue;
		field[y][x] = stuff;
		n -= 1;
	}
}

static void resetField(void)
{
	int i, j;
	memset(field, 0, sizeof(field));
	for (i = 0; i != FW; i++) {
		field[0][i] = wall;
		field[FH-1][i] = wall;
	}
	for (i = 0; i != FH; i++) {
		field[i][0] = wall;
		field[i][FW-1] = wall;
	}
	hx = tx = FW / 2;
	hy = ty = FH / 2;
	field[hy][hx] = head;
	addStuff(5, poison);
	addStuff(foodLeft, food);
}

static void printField(void)
{
	int i, j, kind;
	for (i=0; i!=FH; i++) {
		char *p = field[i];
		for (j=0; j!=FW; j++,p++) {
			if ((kind = *p & 0xf)) {
				gotoxy(j+1,i+1);
				textcolor(colors[kind]);
				putch(chars[kind]);
			}
		}
	}
}

static int dir(int key)
{
	switch(key) {
	case KEY_RIGHT: return 0;
	case KEY_UP: return 1;
	case KEY_LEFT: return 2;
	case KEY_DOWN: return 3;
	default: break;
	}
	return -1;
}

static signed char dx[] = { 1, 0, -1, 0 };
static signed char dy[] = { 0, -1, 0, 1 };

static void setField(int x, int y, int kind)
{
	field[y][x] = kind;
	kind &= 0xf;
	gotoxy(x+1, y+1);
	textcolor(colors[kind]);
	putch(chars[kind]);
}

static int move(register int d)
{
	register int p;
	
	if (d >= 0) {
		setField(hx, hy, tail | (d << 4));
		hx += dx[d];
		hy += dy[d];
		p = field[hy][hx];
		setField(hx, hy, head);
		if (p == food) {
			score += 1;
			foodLeft -= 1;
			if (foodLeft)
				return 0;
			return 1;
		}
		d = (field[ty][tx] & 0xf0) >> 4;
		setField(tx, ty, blank);
		tx += dx[d];
		ty += dy[d];
		if (p != 0) {
			setField(hx, hy, corpse);
			return -1;
		}
		return 0;
	}
	return 0;
}

int main()
{
	int d, r, level;
	clock_t clk, nclk, delay;
	
	for(;;) {
		d = -1;
		nclk = clock();
		delay = CLOCKS_PER_SEC / 3;
		score = level = 0;
		foodLeft = 15;
		textbackground(BLUE);
		clrscr();
		resetField();
		printField();
		for(;;) {
			clk = clock();
			if (kbhit() && (r = dir(getch())) >= 0)
				d = r;
			else if ((int)(clock_t)(clk - nclk) < 0)
				continue;
			nclk = clk + delay;
			r = move(d);
			if (r < 0)
				break;
			if (r > 0) {
				foodLeft = 15;
				addStuff(foodLeft, food);
				printField();
				delay = (delay * 3) >> 2;
				level += 1;
			}
			textcolor(WHITE);
			gotoxy(1, FH+1);
			cprintf("S%03d L%d", score, level);
		}
		textcolor(MAGENTA);
		gotoxy(1, FH+1);
		cprintf("S%03d L%d <END>", score, level);
		while(kbhit()) {}
		while(!kbhit()) {}
	}
}
