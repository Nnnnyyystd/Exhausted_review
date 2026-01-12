#ifdef _UNITTEST
#include "double.h"
#else
#include <double.h>
#endif

unsigned int double_getSign(double lf)
{
	unsigned char* plf = (unsigned char*)(&lf);
	return plf[7] >> 7;
}
struct w_int double_getMantissa(double lf)
{
	struct w_int* plf = (struct w_int*)(&lf);
	plf->h_int &= 0xfffff;
    return *plf;
}
unsigned int double_getExponent(double lf)
{
	int* plf = (int*)(&lf);
	int exponent = (plf[1] >> 20) & 0x7ff;
	exponent -= 0x3ff;
	return exponent;
}
double double_getLeft(double lf)
{
	int exponent = double_getExponent(lf);
	struct w_int* plf = &lf;
	unsigned int mask = 0xffffffff;
	int prec = DOUBLE_MANTISSA_BITS - exponent;
	unsigned int maskbits = 1;
	int intlenght = sizeof(unsigned int) * 8;
	int i = 0;
	if ( exponent < 0 ) return 0.0;
	if ( exponent > DOUBLE_MANTISSA_BITS ) return lf;
	for ( i = 0; i < prec && i < intlenght; i++ )
	{
		mask -= maskbits;
		maskbits = maskbits << 1;
	}
	plf->l_int &= mask;
	if ( prec > intlenght )
	{
		prec -= intlenght;
		mask = 0xffffffff;
		maskbits = 1;
		for ( i = 0; i < prec; i++ )
		{
			mask -= maskbits;
			maskbits = maskbits << 1;
		}
		plf->h_int &= mask;
	}
	return lf;
}
double double_getRight(double lf)
{
	double left = double_getLeft(lf);
	return lf - left;
}
int double_isZero(double lf)
{
	struct w_int mantissa = double_getMantissa(lf);
	return mantissa.l_int == 0 && mantissa.h_int == 0;
}
int double_isUpOverFlow(double lf)
{
	return double_getExponent(lf) == DOUBLE_UP_OVERFLOW_EXPONENT ;
}
int double_isDownOverFlow(double lf)
{
	struct w_int mantissa = double_getMantissa(lf);
	return (double_getExponent(lf) == DOUBLE_DOWN_OVERFLOW_EXPONENT) && (mantissa.h_int == 0) && (mantissa.l_int == 0);
}

