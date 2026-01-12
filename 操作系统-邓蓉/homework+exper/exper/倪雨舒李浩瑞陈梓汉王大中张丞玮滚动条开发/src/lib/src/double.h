#ifndef DOUBLE_H
#define DOUBLE_H

#define DOUBLE_MANTISSA_BITS 52
#define DOUBLE_UP_OVERFLOW_EXPONENT 1024
#define DOUBLE_DOWN_OVERFLOW_EXPONENT -1023

struct w_int
{
	unsigned int l_int;
	unsigned int h_int;
};

unsigned int double_getSign(double lf);
struct w_int double_getMantissa(double lf);
unsigned int double_getExponent(double lf);
double double_getLeft(double lf);
double double_getRight(double lf);
int double_isZero(double lf);
int double_isUpOverFlow(double lf);
int double_isDownOverFlow(double lf);

#endif

