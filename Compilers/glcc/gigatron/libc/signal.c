#include <signal.h>
#include <gigatron/libc.h>
#include <gigatron/sys.h>

extern void _exits(register int signo, register int fpeinfo);
extern void _virq_handler(void);

static sig_handler_t sigvec[8];

sig_handler_t signal(int signo, sig_handler_t h)
{
	register sig_handler_t old;
	/* validate arguments */
	if ((signo & ~7u) || (h == SIG_ERR))
		return SIG_ERR;
	/* signal table */
	old = sigvec[signo];
	sigvec[signo] = h;
	/* activate */
	_raise_disposition = RAISE_EMITS_SIGNAL;
	if (signo == SIGVIRQ &&
	    (romType & 0xfc) >= romTypeValue_ROMv5 )
		vIRQ_v5 = (~1u&(unsigned)h)?(unsigned int)_virq_handler:0;
	return old;
}

int _sigcall(register char signo, register char fpeinfo, register const char *msg)
{
	typedef int (*handler)(int,int);
	register handler *vec = (handler*)sigvec;
	register handler h;
	
	signo &= 7;
	h = vec[signo];
	/* Handle SIG_IGN for recoverable signals */
	if ((h == (handler)SIG_IGN) && (signo & 4))
		return 0;
	/* Call signal handler */
	if (~1u & (unsigned)h) {
		vec[signo] = (handler)SIG_DFL;
		if (signo & 4)
			return h(signo, fpeinfo);
		else
			h(signo, fpeinfo);
	}
	_exitm(20, msg);
	return -1;
}

