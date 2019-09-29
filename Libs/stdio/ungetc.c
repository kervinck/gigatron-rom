#include <stdio.h>

int ungetc(int c, FILE *stream)
{
#if 0 // Issue #76
  if (c == EOF)
    return c;                   // Fail without changing the FILE object
#endif
  *(--stream->_ptr) = c;        // XXX We don't really know if there's space
  stream->_n++;
  stream->_flags &= ~_ioeof;
  return c;
}
