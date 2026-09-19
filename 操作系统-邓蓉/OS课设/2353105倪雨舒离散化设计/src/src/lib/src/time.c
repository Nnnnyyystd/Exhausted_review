#include "time.h"

unsigned int gtime()
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(13) );
	if ( res >= 0 )
		return res;
	return -1;
}

int stime(unsigned int seconds)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(25),"b"(seconds) );
	if ( res >= 0 )
		return res;
	return -1;
}

unsigned int daysInYear( int year )
{
	return isLeapYear(year) ? 366 : 365;
}

unsigned int mktime(struct tm* ptime)
{
	unsigned int timeInSeconds = 0;
	unsigned int days;
	int currentYear = 2000 + ptime->Year;	/* Year中只有年份后2位 */
	
	/* compute hours, minutes, seconds */
	timeInSeconds += ptime->Second;
	timeInSeconds += ptime->Minute * SECONDS_IN_MINUTE;
	timeInSeconds += ptime->Hour * SECONDS_IN_HOUR;
	
	/* compute days in current year */
	days = ptime->DayOfMonth - 1;
	days += daysBeforeMonth[ptime->Month];
	if (isLeapYear(currentYear) && ptime->Month >= 3 /* After February */)
		days++;

	/* compute days in previous years */
	int year;
	for (year = 1970; year < currentYear; year++)
	{
		days += daysInYear(year);
	}
	timeInSeconds += days * SECONDS_IN_DAY;
	
	ptime->DayOfWeek = weekdayNumber[days % 7];
	
	return timeInSeconds;
}

static struct tm local_tm;
struct tm* localtime(unsigned int timeInSeconds)
{
	struct tm* ptime = &local_tm;
	memset(&local_tm, 0, sizeof(local_tm));
	
	/* compute days before today */
	unsigned int days = timeInSeconds / SECONDS_IN_DAY;
	ptime->DayOfWeek = weekdayNumber[(days % 7) + 1 /* 除非timeInSeconds恰好整除SECONDS_IN_DAY秒 */];
	int year = 1970;
	while(days >= 365)
	{
		if( days >= daysInYear(year) )
		{
			days -= daysInYear(year);
			year++;
		}
		else
		{
			break;
		}
	}
	ptime->Year = (year >= 2000) ? (year - 2000) : (year - 1900);
	
	/* Compute month in year & day of month */
	int i;
	int month = 0;	/* before January */
	int dayOfMonth;
	for( i = 1; i < 13; i++ )
	{
		dayOfMonth = ((i == 2 && isLeapYear(year)) ? daysInMonth[i]+1 : daysInMonth[i]);
		if( days > dayOfMonth )
		{
			days -= dayOfMonth;
			month = i + 1;
		}
		else
			break;
	}
	ptime->Month = month;
	ptime->DayOfMonth = days + 1;
	
	/* Compute hour, minute, second */
	unsigned int secondsInToday = timeInSeconds % SECONDS_IN_DAY;
	ptime->Hour = secondsInToday / SECONDS_IN_HOUR;
	ptime->Minute = (secondsInToday % SECONDS_IN_HOUR) / SECONDS_IN_MINUTE;
	ptime->Second = secondsInToday % SECONDS_IN_MINUTE;
	
	return ptime;
}

static char asctime_buf[40];
char* asctime(struct tm* ptime)
{
	memset(asctime_buf, 0, sizeof(asctime_buf));
	
	sprintf(asctime_buf, "%d-%s-%d %d:%d:%d(%s)", ptime->DayOfMonth, monthName[ptime->Month],
			2000 + ptime->Year, ptime->Hour, ptime->Minute, ptime->Second, weekdayName[ptime->DayOfWeek]);
			
	return asctime_buf;
}
