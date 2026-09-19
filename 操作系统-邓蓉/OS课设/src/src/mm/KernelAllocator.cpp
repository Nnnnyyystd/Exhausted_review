#include "KernelAllocator.h"
#include "Allocator.h"

KernelAllocator::KernelAllocator(Allocator *allocator)
{
	this->m_pAllocator = allocator;
}

int KernelAllocator::Initialize()
{
	for ( unsigned int i = 0; i < MEMORY_MAP_ARRAY_SIZE; i++ )
	{
		this->map[i].m_AddressIdx = 0;
		this->map[i].m_Size = 0;
	}

	unsigned long startIdx = KERNEL_HEAP_START_ADDR;
	unsigned long size = KERNEL_HEAP_SIZE;

	map[0].m_AddressIdx = startIdx;
	map[0].m_Size = size;

	return 0;
}

unsigned long KernelAllocator::AllocMemory(unsigned long size)
{
	return this->m_pAllocator->Alloc(this->map, size);
}

unsigned long KernelAllocator::FreeMemeory(unsigned long size, unsigned long memoryStartAddress)
{
	return this->m_pAllocator->Free(this->map, size, memoryStartAddress);
}

KernelAllocator::~KernelAllocator()
{
}

