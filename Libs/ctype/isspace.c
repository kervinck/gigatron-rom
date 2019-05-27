#include <ctype.h>

int isspace(int c)
{
  switch (c) {
  case '\n': case '\r':
  case '\t': case ' ':
    return 1;
  default:
    return 0;
  }
}
