#include "IDT.h"
#include "Utility.h"

void IDT::SetInterruptGate(int number, unsigned int handler)
{
	this->m_Descriptor[number].m_Low16BitsOffset	= handler;
	this->m_Descriptor[number].m_High16BitsOffset	= handler>>16;
	this->m_Descriptor[number].m_SegmentSelector	= 0x8;
	this->m_Descriptor[number].m_Reserved	= 0;
	this->m_Descriptor[number].m_Zero		= 0;
	this->m_Descriptor[number].m_System		= 0;
	this->m_Descriptor[number].m_Type		= 0xE;	//中断门，清IF位
	this->m_Descriptor[number].m_DPL		= 0x3;
	this->m_Descriptor[number].m_SegmentPresent	= 1;
}

void IDT::SetTrapGate(int number, unsigned int handler)
{
	this->m_Descriptor[number].m_Low16BitsOffset	= handler;
	this->m_Descriptor[number].m_High16BitsOffset	= handler>>16;
	this->m_Descriptor[number].m_SegmentSelector	= 0x8;
	this->m_Descriptor[number].m_Reserved	= 0;
	this->m_Descriptor[number].m_Zero		= 0;
	this->m_Descriptor[number].m_System		= 0;
	this->m_Descriptor[number].m_Type		= 0xF;	//陷入门，不清IF位
	this->m_Descriptor[number].m_DPL		= 0x3;
	this->m_Descriptor[number].m_SegmentPresent	= 1;
}

void IDT::DefaultInterruptHandler()
{
	/* 输出错误信息，进入死循环 */
	Utility::Panic("Default Interrupt Handler!");
}

void IDT::DefaultExceptionHandler()
{
	Utility::Panic("Default Exception Handler!");
}

void IDT::FormIDTR(IDTR& idtr)
{
	/* 在启用分页机制下，IDTR中存放的应当是线性地址，而非物理地址 */
	idtr.m_BaseAddress = (unsigned int)this;
	idtr.m_Limit = 2048 - 1;
}
