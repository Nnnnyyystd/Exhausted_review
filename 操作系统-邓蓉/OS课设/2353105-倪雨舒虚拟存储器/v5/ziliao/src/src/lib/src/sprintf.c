#ifdef _UNITTEST
#include "ctype.h"
#include "print_parse.h"
#include "stdio.h"
#include "string.h"
#include "valist.h"
#else
#include <ctype.h>
#include <print_parse.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <valist.h>
#endif

/* return number add to buffer */
int sprintf_char(char* buffer, va pva, unsigned int* pva_offset)
{
	char ch = getva(char, pva, *pva_offset);
    /* it's quite strang that the char size is 4, i didn't read any to book to prove my assumption, perhaps because of bound is 4 */ 
	*pva_offset += CHAR_SIZE;
	*buffer = ch;
	return 1;
}

int sprintf_string(char* buffer, struct print_info* info, va pva, unsigned int* pva_offset)
{
	char* pstr = getva(char*, pva, *pva_offset);
	int strl = strlen(pstr);
	int padding = 0;
	char* bp = buffer;
	int i;
	*pva_offset += sizeof(char*);
	if ( info->width == -1 ) info->width = strl; /* 用户并没有设置宽度值 */
	if ( info->prec == -1 ) info->prec = strl;
	/* 如果字符串精度大于宽度，那么宽度等于精度 */
	if ( info->prec > info->width) info->width = info->prec;
	padding = info->width - (strl > info->prec ? info->prec : strl);
	if ( !info->left )/* padding at left */
	{
		while ( bp < buffer + padding ) *bp++ = ' ';
	}
	for ( i = 0; *pstr && i < info->width - padding; *bp++ = *pstr++, i++ );
	if ( info->left ) 
	{
		while( bp < buffer + info->width ) *bp++ = ' ';
	}	
	return info->width;
}
int sprintf_interger(char* buffer, struct print_info* info, va pva, unsigned int* pva_offset)
{
	char num[256];
	char* pn = buffer;
	char pre[20]; /* 前缀 */
	char* bp = 0;
	char* bpre = pre;
	int strl = 0;
	int padding = 0;
	/* get num string */
	/* this is signed num*/
	if ( info->spec == 'i' || info->spec == 'd' )
	{
		char c_num;
		short s_num;
		int i_num;
		if ( info->is_char )
		{
			c_num = getva(char, pva, *pva_offset);
			*pva_offset += CHAR_SIZE;
			bp = itoa( (int)c_num, num, 10);
		}
		else if ( info->is_short )
		{
			s_num = getva(short, pva, *pva_offset);
			*pva_offset += SHORT_SIZE;
			bp = itoa( (int)s_num, num, 10);
		}
		else
		{
			i_num = getva(int, pva, *pva_offset);
			*pva_offset += INT_SIZE;
			bp = itoa( i_num, num, 10);
		}
	}
	/* for unsigned num */
	else
	{
		unsigned char uc_num;
		unsigned short us_num;
		unsigned int ui_num;
		int radix = 10;
		switch ( info->spec )
		{
		case 'x':
		case 'X':
			radix = 16;
			break;
		case 'o':
			radix = 8;
			break;
		case 'u':
			radix = 10;
			break;
		}
		if ( info->is_char )
		{
			uc_num = getva(char, pva, *pva_offset);
			*pva_offset += CHAR_SIZE;
			bp = itoa( (int)uc_num, num, radix);
		}
		else if ( info->is_short )
		{
			us_num = getva(short, pva, *pva_offset);
			*pva_offset += SHORT_SIZE;
			bp = itoa( (int)us_num, num, radix);
		}
		else
		{
			ui_num = getva(int, pva, *pva_offset);
			*pva_offset += INT_SIZE;
			bp = itoa( ui_num, num, radix);
		}
	}
	if ( !bp ) return 0;
	if ( info->alt )
	{
		switch ( info->spec )
		{
		case 'x':
		case 'X':
			*bpre++ = '0';
			*bpre++ = info->spec;
			break;
		case 'o':
			*bpre++ = '0';
			break;
		}
	}
	if ( info->showsign && *bp != '-' && (info->spec == 'd' || info->spec == 'i')) *bpre++ = '+';
	*bpre = 0;
	strl = strlen(bp) + strlen(bpre);
	if ( info->width < strl ) info->width = strl;
	padding = info->width - strl;
	if ( !info->left )
	{
		while ( pn < buffer + padding ) *pn++ = ' ';
	}
	bpre = pre;
	while ( *bpre && (*pn++ = *bpre++) );
	while ( *bp && (*pn++ = *bp++) );
	if ( info->left )
	{
		while ( pn < buffer + info->width ) *pn++ = ' ';
	}
	return info->width;	
}

int sprintf_double(char* buffer, struct print_info* info, va pva, unsigned int* pva_offset)
{
	char num[256];
	char ex_num[256];
	char* pnum;
	char* bp = buffer;
	int strl = 0;
	int showplus = 0;
	int padding = 0;
	double dnum = 0.0;
	//if ( info->is_long ) /*lf,lF,le,lE,lg,lG*/
	//{
		dnum = getva(double, pva, *pva_offset);
		*pva_offset += DOUBLE_SIZE;
	//}
	//else
	//{
	//	dnum = getva(float, pva, *pva_offset);
	//	*pva_offset += FLOAT_SIZE;
	//}
		if ( info->prec == -1 ) info->prec = 8; /* if the prec is not set, set as default 8); */
	switch( info->spec )
	{
	case 'f':
	case 'F':
		pnum = lftoa(dnum, num, info->prec);
		if ( pnum == 0 ) return 0;
		break;
	case 'e':
	case 'E':
		pnum = exlftoa(dnum, ex_num, info->prec, info->spec);
		if ( pnum == 0 ) return 0;
		break;
	case 'g':
	case 'G':
		if (lftoa(dnum, num, info->prec) == 0 ) return 0;
		if (exlftoa(dnum, ex_num, info->prec,info->spec - 'G' + 'E') == 0) return 0;
		pnum = strlen(num) < strlen(ex_num) ? num : ex_num;
		break;
	default:
		return 0;
	}
	if ( info->showsign && *pnum != '-' ) showplus = 1;
    strl = strlen(pnum) + showplus;
	if ( info->width < strl ) info->width = strl;
	padding = info->width - strl;
	if ( !info->left )
	{
		while ( bp < buffer + padding ) *bp++= ' ';
	}
	if ( showplus ) *bp++ = '+';
	while ( *pnum && ( *bp++ = *pnum++ ));
	if ( info->left )
	{
		while ( bp < buffer + info->width ) *bp++ = ' ';
	}
	return info->width;
}

int _sprintf(char* buffer, char* fmt, va pva)
{
	unsigned int va_offset = 0;
    struct print_spec spec;
	char* bp = buffer;

	if ( buffer == 0 ) return -1;
	
	/* init spec stuct */
	spec.fmt = spec.start_fmt = spec.end_fmt = fmt;
	while( find_spec(&spec) >= 0 )
	{
		char* sbp = spec.end_fmt;		
		while ( sbp < spec.start_fmt ) *bp++ = *sbp++;
		parse_spec(&spec);		
		switch( spec.info.spec )
		{
		/* char */
		case 'c':
			bp += sprintf_char(bp, pva, &va_offset);			
			break;
		/* string */
		case 's':
			bp += sprintf_string(bp, &spec.info, pva, &va_offset);
			break;			
		/* signed int */
		case 'd':
		case 'i':
		/* unsigned int */
		case 'x':
		case 'X':
		case 'o':
		case 'u':
			bp += sprintf_interger(bp, &spec.info, pva,&va_offset);
			break;
		/* double and float */
		case 'f':
		case 'F':
		case 'e':
		case 'E':
		case 'g':
		case 'G':
			bp += sprintf_double(bp, &spec.info, pva, &va_offset);
			break;
		default:
            break;
		}		
	}
	while ( spec.end_fmt < spec.start_fmt ) *bp++ = *spec.end_fmt++;
	*bp = 0;
	return bp - buffer;
}

int sprintf(char* buffer, char* fmt, ...)
{
	va pva = getvahead(fmt);
	return _sprintf(buffer, fmt, pva);	
}
