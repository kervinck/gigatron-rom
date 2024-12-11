
#include <math.h>
#include <stdio.h>

#ifndef RADIUS
# define RADIUS 26
#endif

#ifndef ZOOM
# define ZOOM 1.15
#endif

#ifndef ANGLE
# define ANGLE (-M_PI/20.0)
#endif


double mr, mi;
int lastx, lasty;

#define CENTERX 80
#define CENTERY 60
#define ADDR(x,y) ((x + CENTERX) | ((y + CENTERY + 8)<<8))


void point(int x, int y)
{
	double x2f = mr * x + mi * y;
	double y2f = mr * y - mi * x;
	int x2 = (int)(x2f);
	int y2 = (int)(y2f);
	printf("\t");
	if (x == lastx + 1 && y == lasty)
		printf("INC(T3);");
	else if (x == lastx && y == lasty + 1)
		printf("INC(T3+1);");
	else
		printf("_MOVIW(0x%04x,T3);", ADDR(x, y));
	lastx = x;
	lasty = y;
	if (fabs(x2f) < 1 && fabs(y2f) < 1)
		printf("SYS(34);");
	else
		printf("LDWI(0x%04x);_PEEK1();", ADDR(x2, y2));
	printf("POKE(T3)\n");
}



int main()
{
	int r, x, y;

	mr = cos(ANGLE) / ZOOM;
	mi = sin(ANGLE) / ZOOM;
	int i = 0;

	printf("def code():\n%s",
	       "\tlabel('table')\n"
	       "\tdef _PEEK1(): PEEKA(vAC) if args.cpu >= 6 else PEEK()\n"
	       "\tPUSH()\n");

	for (r = RADIUS; r; r--) {
		for(x = -r, y = -r; x <= r; x++)
			point(x, y);
		for(x = -r, y = r; x <= r; x++)
			point(x, y);
		for(x = -r, y = -r + 1; y < r; y++)
			point(x, y);
		for(x = r, y = -r + 1; y < r; y++)
			point(x, y);
	}
	printf("\ttryhop(2);POP();RET()\n%s\n",
	       "\n"
	       "module(name='table.s',\n"
	       "       code=[('EXPORT', 'table'),\n"
	       "             ('CODE', 'table', code),\n"
	       "             ('PLACE', 'table', 0x8000, 0xffff) ] )\n"
	       "\n"
	       "# Local Variables:\n"
	       "# mode: python\n"
	       "# indent-tabs-mode: ()\n"
	       "# End:\n");
	return 0;
}
