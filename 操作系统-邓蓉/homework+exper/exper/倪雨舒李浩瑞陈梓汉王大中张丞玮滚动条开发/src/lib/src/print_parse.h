#ifndef PRINT_PARSE_H
#define PRINT_PARSE_H

/* 当前所要打印变量的convertion form 信息 */ 
struct print_info
{
    int prec;			/* Precision.  */
    int width;			/* Width.  */
    char spec;			/* Format letter.  */
    unsigned int is_long_double;/* L flag.  */
    unsigned int is_short;	/* h flag.  */
    unsigned int is_long;	/* l flag.  */
    unsigned int alt;		/* # flag.  */
    unsigned int space;		/* Space flag.  */
    unsigned int left;		/* - flag.  */
    unsigned int showsign;	/* + flag.  */
    unsigned int extra;		/* For special use.  */
    unsigned int is_char;	/* hh flag.  */
    char pad;			/* Padding character.  */
};

/* 当前正在被分析的fmt字符串的信息 */
struct print_spec
{
    char* fmt;
    char* start_fmt;    /* 指向当前分析好的字符串的首个字符 */ 
    char* end_fmt;     /* 指向当前分析好的字符串的后面一个字符 */ 
    struct print_info info; /* 当前正在分析的%后的convertion form的信息 */             
};

/*
* 寻找下一个spec的开始字符
* 参数： spec(当前正在分析的fmt字符串) 
* 返回值： 0 成功， -1 eof 
*/
int find_spec( struct print_spec* spec); /* 寻找下一个spec的开始字符 */

int parse_spec( struct print_spec* spec );

unsigned int read_int( char** ppstr );

#endif

