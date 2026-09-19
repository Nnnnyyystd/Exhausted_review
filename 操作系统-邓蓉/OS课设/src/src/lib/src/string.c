/*#ifdef _UNITTEST
#include "string.h"
#else
#include <string.h>
#endif
*/
#include "string.h"

int strcmp(unsigned char* dst, unsigned char* src)
{
    int ret = 0 ;
    while( ! (ret = *dst - *src) && *dst)
            ++src, ++dst;
    if ( ret < 0 )  
		ret = -1 ;
    else if ( ret > 0 )
		ret = 1 ;
    return( ret );	
}

char* strcpy(char* dst, char* src)
{
    char * cp = dst;
    while( *cp++ = *src++ );
    return ( dst );
}

char* strcat(char* dst, char* src)
{
    char * cp = dst;
    while( *cp ) 
		++cp;           /* Find end of dst */
    while( *cp++ = *src++ );/* Copy src to end of dst */
    return( dst );
}

int strlen (char* str)
{
    int length = 0;
    while( *str++ ) ++length;
	return( length );
}

void* memset(void* dst, int c, unsigned int len)
{
	char* s = (char *)dst;
	
	while (len-- != 0)
	{
		*s++ = (char)c;
	}
	return dst;
}

void memmove(unsigned int des, unsigned int src, unsigned int count)
{
	__asm__("cld\n\t"
		"rep\n\t"
		"movsb\n\t"
		::"c" (count),
		"D" (des),"S" (src)
		);
}

void memcpy(unsigned int des, unsigned int src, unsigned int count)
{
	__asm__("cld\n\t"
		"rep\n\t"
		"movsb\n\t"
		::"c" (count),
		"D" (des),"S" (src)
		);
}


