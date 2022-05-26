#include "_stdio.h"
#include <errno.h>

/* This is the generic version of fp->_vec->filbuf.
   It is similar to cons_filbuf but:
   - can use arbitrary buffer or arbitrary size.
   - can allocate a buffer when none is provided,
     but only if malloc is otherwise linked.
   - checks for errors and end-of-file condition.
*/

int _default_filbuf(register FILE *fp)
{
	register int flag = fp->_flag;
	register int n = 0;
	register int bufsiz = 1;
	register char *buf;

	/* Ensure buffer */
	if ((flag & _IOFBF) && !fp->_base) {
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
	/* Cross flush stdout */
	if (fp == stdin)
		_fflush(stdout);
        /* Determine buffer */
	if (flag & _IOFBF) {
		buf = fp->_base->data;
		bufsiz = fp->_base->size;
	} else 
		buf = fp->_buf + sizeof(fp->_buf) - 1;
	/* Read */
	if ((n = fp->_v->read(fp, buf, bufsiz)) <= 0)
		return _serror(fp, (n < 0) ? EIO : EOF);
	/* Prep buffer */
	fp->_ptr = buf + 1;
	fp->_cnt = n - 1;
	return buf[0];
}

