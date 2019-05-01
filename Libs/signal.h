typedef void (*sig_t) (int);

#define SIGINT 2

//sig_t signal(int sig, sig_t func);
#define signal(sig, func) ((sig_t)0)
