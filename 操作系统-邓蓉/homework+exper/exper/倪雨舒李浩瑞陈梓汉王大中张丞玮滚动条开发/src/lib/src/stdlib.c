#include "stdlib.h"
#include "float.h"
#include "double.h"
#include "string.h"

char* _itoa( unsigned long value, int neg_sign, char* buffer, int radix)
{
	char* bret = buffer;
	unsigned int num = value;
	char* bufferStart = 0;
	
	if ( !buffer || radix <= 0 || radix > 16 ) 
		return 0;
	
	if ( neg_sign )
	{
		*buffer++ = '-';
	}
	
	bufferStart = buffer;
	*buffer = '0'; /* if num == 0 then ...*/
	while ( num ) 
	{
		char ch = num % radix;
		*buffer++ =  ch + ( ch < 10 ? '0' : 'a' - 10 );
		num /= radix;
	}
	if ( value ) *buffer = 0;
	else *++buffer = 0;

	buffer--;
	while ( bufferStart < buffer ) /* reserve the string */
	{
		char tch = *bufferStart;
		*bufferStart = *buffer;
		*buffer = tch;
		bufferStart++;
		buffer--;
	}
	return bret;	
}

char* itoa( unsigned long value, char* buffer, int radix )
{
	int s =  value < 0 ? 1 : 0;
	if ( s ) value = -value;
	return _itoa( value, s, buffer, radix );
}
char* uitoa( unsigned long value, char* buffer, int radix )
{
	return _itoa( value, 0, buffer, radix );
}


char* ftoa( float value, char* buffer, int precision )
{
	char* bp = buffer;
	char* sbp; /* start point of buffer */
	char* pdot = 0;
	float right = 0;
	float left = 0;
	int prec = precision;
	if ( precision < 0 && !buffer  ) return 0;
	/* check overflow and put #INF# */
	if ( float_isUpOverFlow(value) )
	{
		if ( float_getSign(value) ) *bp++ = '-';
		strcpy(bp,"1.#INF0");
		return buffer;
	}

	if ( value < 0 ) 
	{
		*bp++ = '-';
		value = -value;
	}
	sbp = bp;
	right = float_getRight(value);
	left = float_getLeft(value);
	if ( left == 0.0 ) *bp++ = '0';
	while (	left >= 1.0 )
	{
		float tf = float_getRight(left / 10.0);
		*bp++ = (char)(tf * 10.0 + 0.5) + '0';
		left /= 10.0;
		left = float_getLeft(left);
	}
	pdot = bp--;
	/* reserve it */
	while ( sbp < bp )
	{
		char ch = *bp;
		*bp = *sbp;
		*sbp = ch;
		bp--;
		sbp++;
	}
	*pdot = '.';
	bp = precision > 0 ? pdot + 1 : pdot;
	while ( precision-- )
	{
		*bp++ = (char)(right * 10.0) + '0';
		right = float_getRight(right * 10);
	}
	if ( prec && right >= 0.5 ) *(bp-1) += 1;
	*bp = 0;
	return buffer;
}
char* lftoa( double value, char* buffer, int precision )
{
 	char* bp = buffer;
	char* sbp; /* start point of buffer */
	char* pdot = 0;
	double right = 0;
	double left = 0;
	if ( precision < 0 && !buffer  ) return 0;
	/* check overflow and put #INF# */
	if ( double_isUpOverFlow(value) )
	{
		if ( double_getSign(value) ) *bp++ = '-';
		strcpy(bp, "1.#INF0");
		return buffer;
	}

	if ( value < 0 )
	{
		*bp++ = '-';
		value = -value;
	}
	sbp = bp;
	right = double_getRight(value);
	left = double_getLeft(value);
	if ( left == 0.0) *bp++ = '0';
	while( left >= 1.0 )
	{
		double tlf = double_getRight( left / 10.0 );
		*bp++ = (char)(tlf * 10.0 + 0.5) + '0';
		left /= 10.0;
		left = double_getLeft(left);
	}	
	pdot = bp--;
	
	while( sbp < bp )
	{
		char ch = *bp;
		*bp = *sbp;
		*sbp= ch;
		bp--;
		sbp++;
	}
	*pdot = '.';
	bp = precision > 0 ? pdot + 1 : pdot;
	while ( precision-- )
	{
		*bp++ = (char)(right * 10.0) + '0';
		right = double_getRight(right * 10);
	}
	//if ( prec && right > 0.5 ) *(bp-1) += 1; /*四舍五入*/
	*bp = 0;
	return buffer;
}

char* exlftoa( double value, char* buffer, int precision, char e )
{
	char exp[10];
	char* pexp = exp;
	int padding;
	char* bp = buffer;
	char* sbp;
	char* pdot = 0;
	double right = 0;
	double left = 0;
	int prec = precision;
	int exponent = 0;
	if ( precision < 0 && !buffer ) return 0;
	/* check overflow and put #INF# */
	if ( double_isUpOverFlow(value) )
	{
		if ( double_getSign(value) ) *bp++ = '-';
		strcpy(bp, "1.#INF0");
		return buffer;
	}
	if ( value < 0 )
	{
		*bp++ = '-';
		value = -value;
	}
	sbp = bp;
	right = double_getRight(value);
	left = double_getLeft(value);
	if ( left < 1.0 && value != 0.0)
	{
		while ( right < 1 )
		{
			right *= 10.0;
			exponent--;
		}
		left = double_getLeft(right);
		right = double_getRight(right);
		*bp++ = (int)left + '0';
		*bp = '.';
		pdot = bp + 1;
	}
	else if ( value == 0.0 )
	{
		*bp++ = '0';
		*bp = '.';
		pdot = bp + 1;
	}
	else
	{
		char* ssbp = sbp;
		while ( left >= 1.0 )
		{
			double tlf = double_getRight( left / 10.0 );
			*bp++ = (char)(tlf * 10.0 + 0.5) + '0';
			left /= 10.0;
			left = double_getLeft(left);
			exponent++;
		}		
		exponent--;
		pdot = bp--;
		while( sbp < bp )
		{
			char ch = *bp;
			*bp = *sbp;
			*sbp= ch;
			bp--;
			sbp++;
		}
		bp = pdot;
		sbp = pdot - 1;
        while ( sbp != ssbp ) *bp-- = *sbp--;
		*bp = '.';
		pdot = prec == 0 ? bp : ( ssbp + 2 +  (exponent > prec  ? prec : exponent) );
		prec = exponent > prec ? 0 : prec - exponent;
	}
	bp = pdot;
	while ( prec-- )
	{
		*bp++ = (char)(right * 10.0) + '0';
		right = double_getRight(right * 10);
	}
	//if ( precision && right > 0.5 ) *(bp-1) += 1; /*四舍五入*/
	*bp++ = e;
	itoa(exponent, exp, 10);	
	if ( *pexp == '-' )
	{
		*bp++ = *pexp++;		
	}
	else
	{
		*bp++ = '+';
	}
	padding = strlen(pexp);
	padding = 3 - padding;
	while ( padding-- ) *bp++ = '0';
	while ( *pexp ) *bp++ = *pexp++;
	*bp = 0;
	return buffer;
}

