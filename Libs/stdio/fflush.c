#include <errno.h>
#include <stdio.h>

int fflush(FILE *stream)
{
  if (stream->_flags & _iowrite) {
    stream->_avail--;
    return stream->_flush(EOF, stream);
  } else {
    errno = -EBADF;
    return EOF;
  }
}
