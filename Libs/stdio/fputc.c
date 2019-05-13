#include <stdio.h>

/*
 *  Return the character written, or EOF if an error occurs
 */
int fputc(int c, FILE *stream)
{
#if 0
  // Issue #73
  return --stream->_avail ? (*stream->_ptr++ = c) : stream->_flush(c, stream);
#else
  if (--stream->_avail) {
    *stream->_ptr++ = c;
    return c;
  } else
    return stream->_flush(c, stream);
#endif
}
