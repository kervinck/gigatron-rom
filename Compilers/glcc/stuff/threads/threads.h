
#ifndef _GT_THREADS_H
#define _GT_THREADS_H


/* The thread structure can be located at offset 0xaa in any page */
typedef struct gt_thread_s {
	// offset 0xaa
	struct gt_thread_s *next;    /* 0x--aa         */
	int  *wchan;		     /* 0x--ac aa^0x06 */
	long *wclk;                  /* 0x--ae aa^0x04 */
	// offset 0xb0
	int regs[24];                /* 0x--b0 aa^0x1a */
	// offset 0xe0 as in VSAVE
	int vAC, vPC, vLR, vSP, vLAC[2], vT2, vT3;
	char sysArgs[8]; int sysFn;
	char vFAS, vFAE, vLAX0, vCpuSelect;
	char irqFlag, irqMask;
} *gt_thread_p;

/* Define a thread at the proper offset */
#define GT_THREAD(name) struct gt_thread_s __attribute__((offset(0xaa))) name

/* Define a stack of size bytes */
#define GT_STACK(name,size) long name[(size<128)?(128>>2):((size|3)>>2)]

/* Return the number of elapsed frames since the program start (the clock.) */
extern long gt_clock(void);

/* Call the scheduler and possibly run another thread for a while */
extern void gt_yield(void);

/* Kill the specified thread. */
extern void gt_kill(gt_thread_p tp);

/* Kill the currently running thread */
extern void gt_exit(void);

/* Returns a pointer to the current thread */
extern gt_thread_p gt_current(void);

/* Tells whether thread tp is running (i.e., started and not killed) */
#define gt_running(tp)  ((tp)->next!=0)

/* Tells whether thread tp is ready (i.e., running and not waiting) */
#define gt_ready(tp)    ((tp)->next&&(!(tp)->wchan||!*(tp)->wchan))

/* Starts thread tp with stack stk of size stksz.
   Argument f is the function to execute. Argument arg is passed to f. */
extern gt_thread_p gt_start(gt_thread_p tp, void *stk, int stksz,
			    void(*f)(void*), void *arg);

/* Wait until termination of thread tp  */
extern void gt_join(gt_thread_p tp);

/* Suspend the thread until the clock reaches clk */
extern void gt_wait(long clk);

/* Suspend the thread for t frames */
extern void gt_sleep(long t);

/* Only one thread can lock a mutex at any given time. */
typedef char gt_mutex_t, *gt_mutex_p;

/* Define a mutex */
#define GT_MUTEX_INIT (0)
#define GT_MUTEX(name) gt_mutex_t name = GT_MUTEX_INIT

/* Lock mutex m, blocking until available */
extern void gt_mutex_lock(gt_mutex_p m);

/* Try locking mutex m without blocking, returning 0 on success, -1 on failure */
extern int gt_mutex_trylock(gt_mutex_p m);

/* Unlock mutex m, allowing other threads to lock it. */
extern void gt_mutex_unlock(gt_mutex_p m);

/* Condition variable. */
typedef char gt_cond_t, *gt_cond_p;

/* Define a condition variable */
#define GT_COND_INIT (1)
#define GT_COND(name)  gt_cond_t name = GT_COND_INIT

/* Signal a condition variable, releasing one waiting thread. */
extern void gt_cond_signal(gt_cond_p m);

/* Atomically unlock mutex m and wait until condition variable c is
   signaled, then relock the mutex and return. Spurious wakeups can
   happen.  Callers must therefore check that the expected condition
   and wait again until fulfilled. */
extern void gt_cond_wait(gt_cond_p c, gt_mutex_p m);

/* Same as gt_cond_wait but stops waiting if clock exceeds clk */
extern void gt_cond_timedwait(gt_cond_p c, gt_mutex_p m, long clk);

#endif
