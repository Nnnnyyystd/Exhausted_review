#ifdef _UNITTEST
#include "float.h"
#else
#include "float.h"
#endif


unsigned int float_getSign(float f)
{
	unsigned int* pf = (unsigned int*)(&f);
	return (*pf) >> 31;
}

unsigned int float_getMantissa(float f)
{
	unsigned int* pf = (unsigned int*)(&f);
	return (*pf) & 0x7fffff;
}

int float_getExponent(float f)
{
	unsigned int* pf = (unsigned int*)(&f);
	int exponent = ((*pf) >> 23) & 0xff;
	exponent -= 0x7f;
	return exponent;
}

/* 得到整数部分 */
float float_getLeft(float f)
{
	int exponent = float_getExponent(f);	
	unsigned int* pf = &f;
	unsigned int mask = 0xffffffff;
	int prec = FLOAT_MANTISSA_BITS - exponent;
	unsigned int maskbits = 1;
	int i = 0;
	if ( exponent < 0 ) return 0.0;
	if ( exponent > FLOAT_MANTISSA_BITS ) return f;
	for ( i = 0; i < prec; i++ )
	{
		mask -= maskbits;
		maskbits = maskbits << 1;
	}
	(*pf) &= mask;
	return f;
}

float float_getRight(float f)
{
	float left = float_getLeft(f);
	return f - left;
}

int float_isZero(float f)
{
	return float_getMantissa(f) == 0 && float_getExponent(f) == 0;
}

/* 上溢 */
int float_isUpOverFlow(float f)
{
	return float_getExponent(f) == FLOAT_UP_OVERFLOW_EXPONENT ;
}
/* 下溢 */
int float_isDownOverFlow(float f)
{
	return (float_getExponent(f) == FLOAT_DOWN_OVERFLOW_EXPONENT) && (float_getMantissa(f) == 0) ;
}

