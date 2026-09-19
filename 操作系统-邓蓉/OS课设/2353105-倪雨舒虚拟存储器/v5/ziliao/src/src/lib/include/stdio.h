#ifndef STDIO_H
#define STDIO_H

#define STDIN	0
#define STDOUT 	1

int sprintf(char* buffer, char* fmt,...);
void printf(char* fmt,...);
void gets(char *s);

/* Don't Del It!!! */
extern char *g_STDIO_str;
extern int g_STDIO_data;
extern char g_STDIO_bss[10];

#endif
