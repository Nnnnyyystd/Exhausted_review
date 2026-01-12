#ifndef VALIST_H
#define VALIST_H

typedef unsigned int va;
#define getvahead( last_vaName ) (&(last_vaName)) + 1
#define getva( type, va, va_offset ) *((type*)(va + va_offset))
#define CHAR_SIZE 4
#define SHORT_SIZE 4
#define INT_SIZE 4
#define LONG_SIZE 4
#define POINTER_SIZE 4
#define FLOAT_SIZE 4
#define DOUBLE_SIZE 8


#endif
