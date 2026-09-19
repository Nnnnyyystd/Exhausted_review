#ifndef STRING_H
#define STRING_H

int strcmp( unsigned char* src, unsigned char* dst);
char* strcpy( char* src, char* dst);
char* strcat( char* src, char* dst);
int strlen (char* str);

void* memset(void* dst, int c, unsigned int len);

#endif
