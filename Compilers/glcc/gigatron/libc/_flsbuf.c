#include "_stdio.h"
#include <errno.h>

#ifdef WITH_LINEBUFFERING
# define HAS_BUF(flag) (flag & _IOFBF))
#else
# define HAS_BUF(flag) (!(flag & _IONBF))
#endif

/* This is the generic version of fp->_v->filbuf.
   - can use arbitrary buffer or arbitrary size.
   - can allocate a buffer when none is provided,
     but only if malloc is otherwise referenced.
   - optionally handles line buffering.
   - checks for errors and end-of-file condition. */

int _default_flsbuf(register int c, register FILE *fp)
{
	register int flag = fp->_flag;
	register int cnt = 0;
	register int n = 0;
	register char *buf;

	/* Allocate buffer */
	if (HAS_BUF(flag) && !fp->_base) {
		struct _sbuf *sb = 0;
		if (__glink_weak_malloc)
			sb = __glink_weak_malloc(BUFSIZ);
		if ((fp->_base = sb)) {
			sb->size = BUFSIZ - sizeof(sb) + 2;
			flag = (flag | _IOMYBUF);
		} else
			flag = (flag & ~_IOBUFMASK) | _IONBF;
		fp->_flag = flag;
	}
	/* Locate buffer */
	if (HAS_BUF(flag)) {
		cnt = fp->_base->size - 1;
		buf = fp->_base->data;
		if (fp->_ptr)
			n = fp->_ptr - buf;
	} else 
		buf = fp->_buf;
	/* Append c to buffer */
	if (c >= 0)
		buf[n++] = (char) c;
	/* Line buffering */
#if WITH_LINEBUFFERING
	if ((flag & _IOLBF) == _IOLBF) {
		if (c >= 0 && c != '\n' && c != '\r' && cnt - n > 0) {
			fp->_ptr = buf + n;
			fp->_cnt = 0;
			return c;
		}
		cnt = 0;
	}
#endif
	/* Set ptr and cnt for future putc */
	fp->_ptr = buf;
	fp->_cnt = cnt;
	/* Write data */
	while (n > 0) {
		if ((cnt = fp->_v->write(fp, buf, n)) <= 0)
			return _serror(fp, EIO);
		buf += cnt;
		n -= cnt;
	}
	if (c < 0)
		return 0;
	return c;
}

