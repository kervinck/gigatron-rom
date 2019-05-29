#include <ctype.h>

int isspace(int c)
{
  switch (c) {
  case '\n': case '\r':
  case '\t': case ' ':
//case '\v': case '\f': // Nobody cares about these
    return 1;
  default:
    return 0;
  }
}
