#include "GDT.h"

void SegmentDescriptor::SetBaseAddress(unsigned int baseAddress)
{
	this->m_Low16BitsBaseAddress = baseAddress & 0xffff;
	this->m_Mid8BitsBaseAddress = (baseAddress >> 16) & 0xff;
	this->m_High8BitsBaseAddress = (baseAddress >> 24) & 0xff;
}

void SegmentDescriptor::SetSegmentLimit(unsigned int segmentLimit)
{	
	this->m_Low16BitsSegmentLimit = segmentLimit & 0xffff;
	this->m_High4BitsSegmentLimit = (segmentLimit >> 16) & 0xf;
}

SegmentDescriptor& GDT::GetSegmentDescriptor(int index)
{
	return m_Descriptors[index];
}

void GDT::SetSegmentDescriptor(int index, SegmentDescriptor &segmentDescriptor)
{
	this->m_Descriptors[index] = segmentDescriptor;
}

void GDT::FormGDTR(GDTR& gdtr)
{
	/* 在启用分页机制下，IDTR中存放的应当是线性地址，而非物理地址 */
	gdtr.m_BaseAddress = (unsigned int)this;
	gdtr.m_Limit = 2048 - 1;
}
