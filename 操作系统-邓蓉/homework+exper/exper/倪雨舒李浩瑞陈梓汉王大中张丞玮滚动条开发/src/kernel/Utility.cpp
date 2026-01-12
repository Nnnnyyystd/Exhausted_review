#include "Utility.h"
#include "Kernel.h"
#include "User.h"
#include "PageManager.h"
#include "Machine.h"
#include "MemoryDescriptor.h"
#include "Video.h"
#include "Assembly.h"

void Utility::MemCopy(unsigned long src, unsigned long des, unsigned int count)
{
	unsigned char* psrc = (unsigned char*)src;
	unsigned char* pdes = (unsigned char*)des;
	
	for ( unsigned int i = 0; i < count; i++ ) 
		pdes[i] = psrc[i];
}

int Utility::CaluPageNeed(unsigned int memoryneed, unsigned int pagesize)
{
	int pageRequired = memoryneed / pagesize;
	pageRequired += memoryneed % pagesize ? 1 : 0;

	return pageRequired;
}

void Utility::StringCopy(char* src, char* dst)
{
	while ( (*dst++ = *src++) != 0 ) ;
}

int Utility::StringLength(char* pString)
{
	int length = 0;
	char* pChar = pString;

	while( *pChar++ )
	{
		length++;
	}

	/* 返回字符串长度 */
	return length;
}

void Utility::CopySeg2(unsigned long src, unsigned long des)
{
	PageTableEntry* userPageTable = (PageTableEntry*)Machine::Instance().GetUserPageTableArray();
	

	/*
	 * 先保存原用户态第一页与第二页PageTableEntry，因为下面的操作
	 * 将会将src所在页映射到0#目录表项，des映射到1#表项，最后进行copy
	 */
	unsigned long oriEntry1 = userPageTable[0].m_PageBaseAddress;
	unsigned long oriEntry2 = userPageTable[1].m_PageBaseAddress;	

	userPageTable[0].m_PageBaseAddress = src / PageManager::PAGE_SIZE;
	userPageTable[1].m_PageBaseAddress = des / PageManager::PAGE_SIZE;

	unsigned char* addressSrc = (unsigned char*)(src % PageManager::PAGE_SIZE);	
	//第二页virtual addess从4096开始
	unsigned char* addressDes = (unsigned char*)(PageManager::PAGE_SIZE + des % PageManager::PAGE_SIZE);	
	//需要刷新页表缓存
	FlushPageDirectory();

	*addressDes = *addressSrc;
	
	//恢复原页表映射
	userPageTable[0].m_PageBaseAddress = oriEntry1;
	userPageTable[1].m_PageBaseAddress = oriEntry2;
	FlushPageDirectory();
}

void Utility::CopySeg(unsigned long src, unsigned long des)
{
	PageTableEntry* PageTable = Machine::Instance().GetKernelPageTable().m_Entrys;

	/*
	 * 先保存原用户态第一页与第二页PageTableEntry，因为下面的操作
	 * 将会将src所在页映射到0#目录表项，des映射到1#表项，最后进行copy
	 */
	unsigned long oriEntry1 = PageTable[borrowedPTE].m_PageBaseAddress;
	unsigned long oriEntry2 = PageTable[borrowedPTE + 1].m_PageBaseAddress;

	PageTable[256].m_PageBaseAddress = src / PageManager::PAGE_SIZE;
	PageTable[257].m_PageBaseAddress = des / PageManager::PAGE_SIZE;

	unsigned char* addressSrc = (unsigned char*)(0xC0000000 + borrowedPTE*PageManager::PAGE_SIZE + src % PageManager::PAGE_SIZE);

	unsigned char* addressDes = (unsigned char*)(0xC0000000 + (borrowedPTE + 1)*PageManager::PAGE_SIZE + des % PageManager::PAGE_SIZE);
	//需要刷新页表缓存
	FlushPageDirectory();

	*addressDes = *addressSrc;

	//恢复原页表映射
	PageTable[borrowedPTE].m_PageBaseAddress = oriEntry1;
	PageTable[(borrowedPTE + 1)].m_PageBaseAddress = oriEntry2;
	FlushPageDirectory();
}

short Utility::GetMajor(const short dev)
{
	short major;
	major = dev >> 8;
	return major;
}

short Utility::GetMinor(const short dev)
{
	short minor;
	minor = dev & 0x00FF;
	return minor;
}

short Utility::SetMajor(short dev, const short value)
{
	dev &= 0x00FF;	/*  清除dev中原先高8比特 */
	dev |= (value << 8);
	return dev;
}

short Utility::SetMinor(short dev, const short value)
{
	dev &= 0xFF00;	/*  清除dev中原先低8比特 */
	dev |= (value & 0x00FF);	/* 仅保留value中的低8位 */
	return dev;
}

void Utility::Panic(char* str)
{
	Diagnose::TraceOn();
	Diagnose::Write("%s\n", str);
	X86Assembly::CLI();
	for(;;);
}

void Utility::DWordCopy(int *src, int *dst, int count)
{
	while(count--)
	{
		*dst++ = *src++;
	}
	return;
}

int Utility::Min(int a, int b)
{
	if(a < b)
		return a;
	return b;
}

int Utility::Max(int a, int b)
{
	if(a > b)
		return a;
	return b;
}

int Utility::BCDToBinary( int value )
{
	return ( (value >> 4) * 10 + (value & 0xF) );
}

void Utility::IOMove(unsigned char* from, unsigned char* to, int count)
{
	while(count--)
	{
		*to++ = *from++;
	}
	return;
}

unsigned int Utility::MakeKernelTime( struct SystemTime* pTime )
{
	unsigned int timeInSeconds = 0;
	unsigned int days;
	int currentYear = 2000 + pTime->Year;	/* Year中只有年份后2位 */

	/* compute hours, minutes, seconds */
	timeInSeconds += pTime->Second;
	timeInSeconds += pTime->Minute * Utility::SECONDS_IN_MINUTE;
	timeInSeconds += pTime->Hour * Utility::SECONDS_IN_HOUR;

	/* compute days in current year */
	days = pTime->DayOfMonth - 1;
	days += Utility::DaysBeforeMonth[pTime->Month];
	if (Utility::IsLeapYear(currentYear) && pTime->Month >= 3 /* After February */)
		days++;

	/* compute days in previous years */
	for (int year = 1970; year < currentYear; year++)
	{
		days += Utility::DaysInYear(year);
	}
	timeInSeconds += days * Utility::SECONDS_IN_DAY;
	
	return timeInSeconds;
}

/* 某个月份前经过的天数，第0项不使用，未纳入计算闰年2月份29天 */
const unsigned int Utility::DaysBeforeMonth[13] = {0xFFFFFFFF/* Unused */, 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

bool Utility::IsLeapYear( int year )
{
	return (year % 4) == 0 && ( (year % 100) != 0 || (year % 400) == 0 );
}

unsigned int Utility::DaysInYear( int year )
{
	return IsLeapYear(year) ? 366 : 365;
}
