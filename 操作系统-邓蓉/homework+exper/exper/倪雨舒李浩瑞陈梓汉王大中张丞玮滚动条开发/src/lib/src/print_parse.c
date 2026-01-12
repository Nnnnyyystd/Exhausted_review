#ifdef _UNITTEST
#include "ctype.h"
#include "print_parse.h"
#else
#include <print_parse.h>
#include <ctype.h>
#endif

int find_spec( struct print_spec* spec) /* 寻找下一个spec的开始字符 */
{
    char* pf = spec->end_fmt;
    while ( *pf != 0 && *pf != '%' )
        {pf++; }
    spec->start_fmt = pf;
    if ( *pf == 0 ) return -1;
    else return 0;
} 

int parse_spec( struct print_spec* spec )
{
    char* format = spec->start_fmt;
	/* init the spec->info */
	spec->info.alt = 0;
	spec->info.space = 0;
	spec->info.left = 0;
	spec->info.showsign = 0;
	/*spec->info.group = 0;*/
	spec->info.pad = ' ';
	
	
	/* skip the char '%' */
	format++;
	
 	/* now I only implement a sub set version 
  	of the iso c lib, but i think it's enough
   	for unix86 */
   	/* check the flag */
    while ( 
    	*format == ' ' || *format == '0' || *format == '+' ||
    	*format == '-' ||*format == '#' )
     {
     	switch ( (int)(*format++) )
     	{
     		case ' ':
     			spec->info.space = 1;
     			break;
 			case '0':
 				spec->info.pad = '0';
 				break;
			case '+':
				spec->info.showsign = 1;
				break;
			case '-':
				spec->info.left = 1;
				break;
			case '#':
				spec->info.alt = 1;
				break; 				
     	}
     } 
	 if ( spec->info.left ) spec->info.pad = ' ';
     spec->info.width = -1;
	 spec->info.prec = -1;
     /* yes, this is width field */
     if ( isdigit( *format ) )
     {
     	spec->info.width = read_int( &format );
     }		
	 if( *format == '.' )
	 {
		 format++;
		 if ( isdigit( *format ) )
		 {
			 spec->info.prec = read_int( &format );
		 }
	 }/* now finished get the width and prec */
	 /* to parse the type infomation */
	 spec->info.is_char = 0;	/*hh*/
	 spec->info.is_short = 0;	/*h*/
	 spec->info.is_long = 0;	/*l*/
	 spec->info.is_long_double = 0;	/*ll*/
	switch ( *format )
	{
	case 'l':
		if ( *(format+1) == 'l')
		{
			spec->info.is_long_double = 1;
			format++;
		}
		else spec->info.is_long = 1;
		format++;
		break;
	case 'h':
		if ( *(format+1) == 'h' ) 
		{
			spec->info.is_char = 1;
			format++;
		}
		else spec->info.is_short = 1;
		format++;
		break;
	}
	spec->info.spec = *format++;
	spec->end_fmt = format;

	return 0;
} 

unsigned int read_int( char** ppstr )
{
	unsigned int rval = *((*ppstr)++) - '0';
 	while ( isdigit( **ppstr ) )
	{
		rval *= 10;			
		rval += **ppstr - '0';
		(*ppstr)++;
	}
	return rval;
}
