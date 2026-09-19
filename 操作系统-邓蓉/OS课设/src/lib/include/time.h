#ifndef TIME_H
#define TIME_H

/* 保存系统时间信息的结构体 */
struct tm
{
	int Second;		/* Seconds: 0 ~ 59 */
	int Minute;		/* Minutes: 0 ~ 59 */
	int Hour;		/* Hours of Day: 0 ~ 23 */
	int DayOfMonth;	/* Day of Month: 1 ~ 31 */
	int Month;		/* Months since January: 1 ~ 12 */
	int Year;		/* Years since 1900 */
	int DayOfWeek;	/* Days since Sunday: 1 ~ 7 */
};

/* 记录进程使用的内核态和用户态下CPU时间的结构体 */
struct tms
{
	int utime;		/* 进程用户态CPU时间 */
	int stime;		/* 进程核心态CPU时间 */
	int cutime;		/* 子进程用户态时间总和 */
	int cstime;		/* 子进程核心态时间总和 */
};

/* 获取进程用户态、核心态CPU时间片数 */
extern int times(struct tms* ptms);

#define SECONDS_IN_MINUTE (60)
#define SECONDS_IN_HOUR (3600)
#define SECONDS_IN_DAY (86400)

#define isLeapYear(year) ((year % 4) == 0 && ( (year % 100) != 0 || (year % 400) == 0 ))

/* 某个月份前经过的天数，第0项不使用，未纳入计算闰年2月份29天 */
static unsigned int daysBeforeMonth[13] = {	0xFFFFFFFF/* Unused */, 
											0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
											
static unsigned int daysInMonth[13] = {	0xFFFFFFFF/* Unused */, 
											31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
											
static int weekdayNumber[7] = {4/*Thrusday*/, 5, 6, 7, 1, 2, 3};

static char* weekdayName[8] = { "NOT Used", "SUN", "MON", "TUE", 
								"WED", "THUR", "FRI", "SAT"};
								
static char* monthName[13] = {"NOT Used",	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
										"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

/* 获取系统时间，返回值从1970年1月1日0时至当前的秒数 */
unsigned int gtime();

/* 设置系统时间，参数seconds从1970年1月1日0时至当前的秒数 */
int stime(unsigned int seconds);

/* 根据tm结构体中的值计算出从1970年1月1日0时至当前的秒数 */
unsigned int mktime(struct tm* ptime);

/* 根据参数seconds(从1970年1月1日0时至当前的秒数)设置tm结构体的各项值 */
struct tm* localtime(unsigned int timeInSeconds);

/* 根据tm结构体中的值，返回ASCII格式的时间日期值 */
char* asctime(struct tm* ptime);

/* 返回一年的天数，闰年为366天 */
unsigned int daysInYear( int year );

#endif
