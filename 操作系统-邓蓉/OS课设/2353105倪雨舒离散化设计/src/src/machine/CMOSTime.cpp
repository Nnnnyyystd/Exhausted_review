#include "CMOSTime.h"
#include "IOPort.h"
#include "Assembly.h"

void CMOSTime::ReadCMOSTime( struct SystemTime* pTime )
{
	int value;
	
	/* 等待直到CMOS的RTC就绪，才开始读取时间和日期 */
	while( (value = ReadCMOSByte(CMOSTime::RTC_STATUS_REGISTER_A)) & CMOSTime::RTC_UPDATE_IN_PROGRESS )
		;

	pTime->Second = ReadCMOSByte(CMOSTime::RTC_SECONDS);
	pTime->Minute = ReadCMOSByte(CMOSTime::RTC_MINUTES);
	pTime->Hour = ReadCMOSByte(CMOSTime::RTC_HOURS);
	pTime->DayOfMonth = ReadCMOSByte(CMOSTime::RTC_DAY_OF_MONTH);
	pTime->Month = ReadCMOSByte(CMOSTime::RTC_MONTH);
	pTime->Year = ReadCMOSByte(CMOSTime::RTC_YEAR);
	pTime->DayOfWeek = ReadCMOSByte(CMOSTime::RTC_DAY_OF_WEEK);

	/* Convert BCD values in SystemTime to Binary values */
	pTime->Second = Utility::BCDToBinary(pTime->Second);
	pTime->Minute = Utility::BCDToBinary(pTime->Minute);
	pTime->Hour = Utility::BCDToBinary(pTime->Hour);
	pTime->DayOfMonth = Utility::BCDToBinary(pTime->DayOfMonth);
	pTime->Month = Utility::BCDToBinary(pTime->Month);
	pTime->Year = Utility::BCDToBinary(pTime->Year);
	pTime->DayOfWeek = Utility::BCDToBinary(pTime->DayOfWeek);

	return;
}

int CMOSTime::ReadCMOSByte( unsigned char CMOSOffset )
{
	int value;
	
	X86Assembly::CLI();

	IOPort::OutByte(CMOSTime::CMOS_ADDR_PORT, CMOSOffset);
	value = IOPort::InByte(CMOSTime::CMOS_DATA_PORT);

	X86Assembly::STI();

	return value;
}
