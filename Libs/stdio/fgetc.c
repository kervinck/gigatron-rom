#include <errno.h>
#include <stdio.h>

int fgetc(FILE *stream)
{
  if ((stream->_flags & _ioread) == 0) {
    errno = -EBADF;
    return EOF;
  }
  if (stream->_n == 0) {
    int r = stream->_flush(0 /*dummy*/, stream); // Read new data
    if (r < 0)
      return r;
  }
  stream->_n--;
  return *stream->_ptr++;
}
