#include <errno.h>
#include <stdio.h>

FILE *fopen(const char *path, const char *mode)
{
  errno = -ENOENT;
  return NULL;
}
