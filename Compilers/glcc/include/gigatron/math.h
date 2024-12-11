#ifndef __MATH
#define __MATH

#define HUGE_VAL 1.701411834209e+38

extern double acos(double);
extern double asin(double);
extern double atan(double);
extern double atan2(double, double);
extern double cos(double);
extern double sin(double);
extern double tan(double);
extern double cosh(double);
extern double sinh(double);
extern double tanh(double);
extern double exp(double);
extern double frexp(double, int *);
extern double ldexp(double, int);
extern double log(double);
extern double log10(double);
extern double modf(double, double *);
extern double pow(double, double);
extern double sqrt(double);
extern double ceil(double);
extern double fabs(double);
extern double floor(double);
extern double fmod(double, double);
extern double copysign(double, double);


/* Not C89 */

extern double _pi, _pi_over_2, _pi_over_4;

#define M_PI     (_pi)
#define M_PI_2   (_pi_over_2)
#define M_PI_4   (_pi_over_4)

#define M_E      (2.7182818284590452353)
#define M_LOG2E	 (1.4426950408889634073)
#define M_LOG10E (0.4342944819032518276)
#define M_LN2    (0.6931471805599453094)
#define M_LN10	 (2.3025850929940456840)



#endif /* __MATH */
