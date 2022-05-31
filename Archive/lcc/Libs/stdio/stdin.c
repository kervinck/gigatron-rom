#include <stdio.h>

#include <Gigatron.h>

/*
 *  Provide very simple line editting, with cursor and delete, until
 *  no more new characters fit on the line, or hitting enter/return.
 */
#define N (160/6)

struct _stdin {
  FILE iobuf;
  char buffer[N];
};

static int _flush_stdin(int c, FILE *stream);

struct _stdin _stdin = {
  {
    NULL,                       // _buffer
    0,                          // _n -> how much still buffered for reading
    &_flush_stdin,              // _flush -> enter the line editor
    _ioread,                    // _flags
  }
};

static int _flush_stdin(int c /*ignored*/, FILE *stream)
{
  char *ptr = ((struct _stdin*)stream)->buffer;
  int n = 0;

  if (stream->_flags & _ioeof)
    return EOF;

  for (;;) {
    PutChar(127);               // Cursor symbol
    ScreenPos -= 6;             // Go back
    c = WaitKey();
    PutChar(' ');               // Remove cursor
    ScreenPos -= 6;             // Go back

    if (32 <= c && c < 127) {
      ptr[n++] = c;             // Put character in buffer
      PutChar(c);               // Put character on screen
      if ((ScreenPos & 255) > 160-6)
        break;
    } else if (c == 127) {      // Delete key
      if (n != 0) {
        ScreenPos -= 6;
        n--;
      }
    } else if (c == '\n') {
      ptr[n++] = c;             // Put newline in buffer
      PutChar(c);               // Advance one line
      break;                    // Stop line edit
    } else {
      // TODO: handle Ctrl-D (EOF)
      // TODO: handle other control chars
    }
  }

  stream->_ptr = ptr;
  stream->_n = n;

  return 0;
}
