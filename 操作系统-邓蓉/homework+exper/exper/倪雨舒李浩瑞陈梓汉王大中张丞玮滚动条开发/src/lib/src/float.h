#ifndef FLOAT_H
#define FLOAT_H

#define FLOAT_MANTISSA_BITS 23
#define FLOAT_MANTISSA_START 0	
#define FLOAT_UP_OVERFLOW_EXPONENT 128
#define FLOAT_DOWN_OVERFLOW_EXPONENT -127

unsigned int float_getSign(float f);
/* 得到尾数部分 */
unsigned int float_getMantissa(float f);
/* 得到阶码部分 */
int float_getExponent(float f);
/* 得到整数部分 */
float float_getLeft(float f);
/* 得到小数部分 */
float float_getRight(float f);
int float_isZero(float f);
/* 上溢 */
int float_isUpOverFlow(float f);
/* 下溢 */
int float_isDownOverFlow(float f);

#endif

