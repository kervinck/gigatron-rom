#include <stdio.h>

int fclose(FILE *stream)
{
  int r = 0;
  if (stream->_flags & _iowrite)
    r = fflush(stream);
  stream->_flags &= ~(_ioread | _iowrite);
  return r;
}
