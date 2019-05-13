#include <stdio.h>

#include <Gigatron.h>

#define N 26

struct _stdout {
  FILE iobuf;
  char buffer[N-1];
};

static int _flush_stdout(int c, FILE *stream);

struct _stdout _stdout = {
  {
    _stdout.buffer,             // _ptr
    N,                          // _avail
    &_flush_stdout,             // _flush
    _iowrite,                   // _flags
  }
};

static int _flush_stdout(int c, FILE *stream)
{
  int n = N - 1 - stream->_avail;
  int i;

  stream->_avail = N;
  stream->_ptr = ((struct _stdout*)stream)->buffer;

  for (i=0; i<n; i++)
    PutChar(stream->_ptr[i]);
  if (c >= 0)
    PutChar(c);
  return 0;
}
