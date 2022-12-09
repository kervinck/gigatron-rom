#include <stdlib.h>
#include <string.h>
#include <gigatron/libc.h>
#include <gigatron/console.h>
#include <gigatron/sys.h>


extern char altVideoTable[];
extern void smoothScroll(int);

void patchVideoTable()
{
  int vy;
  for(vy=0; vy!=16; vy+=2) {
    altVideoTable[vy] = videoTable[224+vy];
    videoTable[224+vy] = 0x8;
  }
}

void pause(char n)
{
  n += frameCount;
  while (frameCount != n)
    { }
}

void scroll(const char **p)
{
  const char *q;
  char *addr;
  int color;
  int len;

  while (q = *p)
    {
      color = 0x3f00;
      p++;
      if (*q == '\a')
        {
          color = (q[1]<<8);
          q += 2;
        }
      if (*q == '\f')
        {
          smoothScroll(q[1] & 0xf8);
          q += 2;
        }
      len = strlen(q);
      smoothScroll(8);
      addr = (char*)((altVideoTable[0]<<8) | (80-(len*3)));
       _console_printchars(color, addr, q, 32767);
    }
}

const char *text[] = {
  "\f\60",
  "The Gigatron TTL computer",
  "was brought to you by",
  "",
  "\a\013Marcel van Kervinck",
  "\a\013Walter Belgers",
  "",
  "Bricks",
  "\a\013xbx",
  "",
  "GtMine",
  "\a\013hans61",
  "",
  "Special thanks",
  "\a\013Marc Paul Ivana Oscar",
  "\a\013Martijn Erik Chuck Ben",
  "\a\013Dieter Martin Brad Lou",
  "\a\013Phil Brian David Davei HG",
  "and",
  "\a\013The Gigatron community!",
  "\f\70",
  "Gigatron TTL",
  "\a\60http://gigatron.io",
  "",
  "\a\013Marcel van Kervinck",
  "\a\013Walter Belgers",
  "\f\40",
  0,
};

void main()
{
  patchVideoTable();
  _console_clear((char*)0x800, 0, 120);
  while(1) {
    scroll(text);
    pause(240);
  }
}
