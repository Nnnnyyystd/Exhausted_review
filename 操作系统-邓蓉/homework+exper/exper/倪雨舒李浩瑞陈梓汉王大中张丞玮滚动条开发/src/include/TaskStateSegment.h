#ifndef TSS_H
#define TSS_H

struct TaskStateSegment
{
	unsigned int m_PreviousTaskLink;	//16bit
	unsigned int m_ESP0;
	unsigned int m_SS0;					//16bit
	unsigned int m_ESP1;
	unsigned int m_SS1;					//16bit
	unsigned int m_ESP2;
	unsigned int m_SS2;					//16bit
	unsigned int m_CR3;
	unsigned int m_EIP;
	unsigned int m_EFLAGS;
	unsigned int m_EAX;
	unsigned int m_ECX;
	unsigned int m_EDX;
	unsigned int m_EBX;
	unsigned int m_ESP;
	unsigned int m_EBP;
	unsigned int m_ESI;
	unsigned int m_EDI;
	unsigned int m_ES;					//16bit
	unsigned int m_CS;					//16bit
	unsigned int m_SS;					//16bit
	unsigned int m_DS;					//16bit
	unsigned int m_FS;					//16bit
	unsigned int m_GS;					//16bit
	unsigned int m_LDT;					//16bit		
	unsigned char m_DebugTrapFlag : 1;
	unsigned short m_Empty : 15;
	unsigned short m_IOMapAddressBase;
}__attribute__((packed));

struct TaskStateSegmentDescriptor
{
	unsigned short	m_Low16BitsSegmentLimit : 16;
	unsigned short	m_Low16BitsBaseAddress : 16;
	unsigned char	m_Mid8BitsBaseAddress : 8;
	unsigned char	m_Type : 4;
	unsigned char	m_ZeroBit : 1;
	unsigned char	m_DescriptorPrivilegeLevel : 2;
	unsigned char	m_Present : 1;
	unsigned char	m_High4BitsSegmentLimit : 4;
	unsigned char	m_Available : 1;
	unsigned char	m_ZeroBit2 : 2;
	unsigned char	m_Granularity : 1;
	unsigned char	m_High8BitsBaseAddress : 8;

public:
	void SetBaseAddress(unsigned long baseAddress);
	void SetSegmengLimit(unsigned int segmentLimit);
}__attribute__((packed));

#endif

