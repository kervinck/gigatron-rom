
#include <gigatron/idioms.h>

extern void fw(unsigned);
extern void fp(char*);

int lo1(unsigned x) { return lo(x); }
int lo2(int x)      { return lo(x); }
int lo3(char *x)    { return lo(x); }

int hi1(unsigned x) { return hi(x); }
int hi2(int x)      { return hi(x); }
int hi3(char *x)    { return hi(x); }

void mkw1(int h, int l) { fw(makew(h,l)); }
void mkp1(int l, int h) { fp(makep(h,l)); }
void mkw2(int h, int l) { fw(makew(h+2,l)); }
void mkp2(int l, int h) { fp(makep(h+2,l)); }

void msw1(int h, int l) { fw(mashw(h,l)); }
void msp1(int l, int h) { fp(mashp(h,l)); }
void msw2(int h, int l) { fw(mashw(h+2,l)); }
void msp2(int l, int h) { fp(mashp(h+2,l)); }

int cl1(int var, int x) { copylo(var,x); return var; }
int ch1(int var, int x) { copyhi(var,x); return var; }
char *cpl1(char *ptr, int x) { copylo(ptr,x); return ptr; }
char *cph1(char *ptr, int x) { copyhi(ptr,x); return ptr; }

int sl1(int var, int x) { setlo(var,x); return var; }
int sh1(int var, int x) { sethi(var,x); return var; }
char *spl1(char *ptr, int x) { setlo(ptr,x); return ptr; }
char *sph1(char *ptr, int x) { sethi(ptr,x); return ptr; }

void mkw1c(int h, int l) { fw(makew(h,0)); }
void mkw2c(int h, int l) { fw(makew(0,l)); }
void mkw1d(int h, int l) { fw(makew(h,1)); }
void mkw2d(int h, int l) { fw(makew(1,l)); }

void msw1c(int h, int l) { fw(mashw(h,0)); }
void msw2c(int h, int l) { fw(mashw(0,l)); }
void msw1d(int h, int l) { fw(mashw(h,1)); }
void msw2d(int h, int l) { fw(mashw(257,l)); }
void msw1e(int l, int h) { fw(mashw(h,1)); }
void msw2e(int l, int h) { fw(mashw(257,l)); }

int cl1c(int var, int x) { copylo(var,0); return var; }
int ch1c(int var, int x) { copyhi(var,0); return var; }
int sl1c(int var, int x) { setlo(var,0); return var; }
int sh1c(int var, int x) { sethi(var,0); return var; }

int cl1d(int var, int x) { copylo(var,257); return var; }
int ch1d(int var, int x) { copyhi(var,257); return var; }
int sl1d(int var, int x) { setlo(var,257); return var; }
int sh1d(int var, int x) { sethi(var,257); return var; }

