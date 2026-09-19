#ifndef PREEXECUTE_H
#define	PREEXECUTE_H

/* 
* 功能：将输入命令分成token, 并将位置送到args数组中
* 参数：input输入字符串
* 返回：分析出参数的个数
*/
int SpiltCommand( char* input );
/*
* 功能：得到下一个token的地址
* 参数：input输入字符串
* 返回：下一个keytoken的地址
*/
char* NextKeyToken(char* input);
/*
* 功能：删除输入字符串左边多余空格
* 参数：tp输入字符串
* 返回：结果字符串
*/
char* TrimLeft( char* tp );

#endif

