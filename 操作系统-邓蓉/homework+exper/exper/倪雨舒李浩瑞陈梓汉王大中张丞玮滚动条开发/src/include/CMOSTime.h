#ifndef CMOSTIME_H
#define CMOSTIME_H

#include "Utility.h"

class CMOSTime
{
public:
	/* static const member variables*/
	static const unsigned short CMOS_ADDR_PORT = 0x70;		/* CMOS地址端口号0x70，该端口用于选择要读取的CMOS字节偏移 */	
	static const unsigned short CMOS_DATA_PORT = 0x71;		/* CMOS数据端口号0x71，获取指定字节偏移的数据内容的端口 */
	
	/* CMOS存储器中各个字节偏移量的定义 其数据都为!!BCD格式!!存储，需要转换成二进制 */
	static const unsigned char RTC_SECONDS = 0x00;			/* 实时钟(Real Time Clock)当前秒值 */
	static const unsigned char RTC_ALARM_SECONDS = 0x01;	/* 实时钟(Real Time Clock)警报秒值，Not Used */
	static const unsigned char RTC_MINUTES = 0x02;			/* 实时钟(Real Time Clock)当前分钟值 */
	static const unsigned char RTC_ALARM_MINUTES = 0x03;	/* 实时钟(Real Time Clock)警报分钟值，Not Used */
	static const unsigned char RTC_HOURS = 0x04;			/* 实时钟(Real Time Clock)当前小时值 */
	static const unsigned char RTC_ALARM_HOURS = 0x05;		/* 实时钟(Real Time Clock)警报小时值，Not Used */
	static const unsigned char RTC_DAY_OF_WEEK = 0x06;		/* 实时钟(Real Time Clock)Days of Week since Sunday: 1 ~ 7 */
	static const unsigned char RTC_DAY_OF_MONTH = 0x07;		/* 实时钟(Real Time Clock)Days of Month: 1 ~ 31 */
	static const unsigned char RTC_MONTH = 0x08;			/* 实时钟(Real Time Clock)当前月份值：Months since January: 0 ~ 11 */
	static const unsigned char RTC_YEAR = 0x09;				/* 实时钟(Real Time Clock)当前年份值4位中的后2位：Years since 1900 */
	static const unsigned char RTC_STATUS_REGISTER_A = 0x0A;/* 实时钟(Real Time Clock)状态寄存器A */
	static const unsigned char RTC_STATUS_REGISTER_B = 0x0B;/* 实时钟(Real Time Clock)状态寄存器B，Not Used */
	static const unsigned char RTC_STATUS_REGISTER_C = 0x0C;/* 实时钟(Real Time Clock)状态寄存器C，Not Used */
	static const unsigned char RTC_STATUS_REGISTER_D = 0x0D;/* 实时钟(Real Time Clock)状态寄存器D，Not Used */
	
	static const unsigned char BASE_MEMORY_LOW = 0x15;			/* 基础内存(低字节) */
	static const unsigned char BASE_MEMORY_HIGH = 0x16;			/* 基础内存(高字节) */
	static const unsigned char EXTENDED_MEMORY_LOW = 0x17;		/* 扩展内存(低字节) */
	static const unsigned char EXTENDED_MEMORY_HIGH = 0x18;		/* 扩展内存(高字节) */
	static const unsigned char EXTENDED_MEMORY_ABOVE_1MB_LOW = 0x30;	/* 1MB以上扩展内存(低字节) */
	static const unsigned char EXTENDED_MEMORY_ABOVE_1MB_HIGH = 0x31;	/* 1MB以上扩展内存(高字节) */

	/* RTC状态寄存器A比特位定义： */
	static const unsigned char RTC_UPDATE_IN_PROGRESS = 0x80;	/* Bit(7)等于1表示Time update in progress，正在更新CMOS时间，
																此时不可读取，Bit(7)等于0表示可以读取CMOS时间和日期 */

public:
	/* 从COMS存储器中获取系统时间，填充SystemTime结构 */
	static void ReadCMOSTime(struct SystemTime* pTime);

	/* 读取指定偏移位置上的CMOS存储器数据内容 */
	static int ReadCMOSByte(unsigned char CMOSOffset);
};

#endif
