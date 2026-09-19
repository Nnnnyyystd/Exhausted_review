#ifndef STDLIB_H
#define STDLIB_H

int errno;

char* itoa( unsigned long value, char* buffer, int radix );
char* ltoa( int value, char* buffer, int radix );
char* ultoa( int value, char* buffer, int radix );
char* _itoa( unsigned long value, int is_sign, char* buffer, int radix);
/* ftoa由于其精度问题不再使用*/
char* ftoa( float value, char* buffer, int precision );
char* lftoa( double value, char* buffer, int precision );
char* exlftoa( double value, char* buffer, int precision, char e );

#endif
