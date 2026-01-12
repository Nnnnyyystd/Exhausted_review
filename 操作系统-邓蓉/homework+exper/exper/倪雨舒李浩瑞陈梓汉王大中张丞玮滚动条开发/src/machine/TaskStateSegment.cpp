#include "TaskStateSegment.h"

void TaskStateSegmentDescriptor::SetBaseAddress(unsigned long baseAddress)
{
	this->m_Low16BitsBaseAddress = baseAddress & 0xffff;
	this->m_Mid8BitsBaseAddress = (baseAddress >> 16) & 0xff;
	this->m_High8BitsBaseAddress = (baseAddress >> 24) & 0xff;
}

void TaskStateSegmentDescriptor::SetSegmengLimit(unsigned int segmentLimit)
{
	this->m_Low16BitsSegmentLimit = segmentLimit & 0xffff;
	this->m_High4BitsSegmentLimit = (segmentLimit >> 16) & 0xf;
}

