#ifndef __FLOAT
#define __FLOAT

#define FLT_ROUNDS	1
#define FLT_RADIX	2

/* Microsoft basic floating point format (5 bytes) 
 * eeeeeeee Smmmmmm mmmmmmm mmmmmmm mmmmmmm
 * Exponent E=eeeeeeee offset by 129
 * Mantissa M=mmmmm... 31 bits with implicit 1
 *   E=0 ==> 0
 *   S=0 ==> (0.5+M)*2^(E-128)
 *   S=1 ==> -(0.5+M)*2^(E-128)
 */

#define FLT_DIG		9
#define FLT_EPSILON	4.656612873077e-10
#define FLT_MANT_DIG	31
#define FLT_MAX		1.701411834209e+38
#define FLT_MAX_10_EXP	38
#define FLT_MAX_EXP	126
#define FLT_MIN		2.938735877056e-39
#define FLT_MIN_10_EXP	-38
#define FLT_MIN_EXP	-128

#define DBL_DIG		FLT_DIG
#define DBL_EPSILON	FLT_EPSILON
#define DBL_MANT_DIG	FLT_MANT_DIG
#define DBL_MAX		FLT_MAX
#define DBL_MAX_10_EXP	FLT_MAX_10_EXP
#define DBL_MAX_EXP	FLT_MAX_EXP
#define DBL_MIN		FLT_MIN	
#define DBL_MIN_10_EXP	-FLT_MIN_10_EXP
#define DBL_MIN_EXP	-FLT_MIN_EXP

#define LDBL_MANT_DIG	DBL_MANT_DIG
#define LDBL_EPSILON	DBL_EPSILON
#define LDBL_DIG	DBL_DIG
#define LDBL_MIN_EXP	DBL_MIN_EXP
#define LDBL_MIN	DBL_MIN
#define LDBL_MIN_10_EXP	DBL_MIN_10_EXP
#define LDBL_MAX_EXP	DBL_MAX_EXP
#define LDBL_MAX	DBL_MAX
#define LDBL_MAX_10_EXP	DBL_MAX_10_EXP

#endif /* __FLOAT */
