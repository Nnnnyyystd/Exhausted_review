#ifndef KERNEL_ALLOCATOR
#define KERNEL_ALLOCATOR

#include "MapNode.h"
#include "Allocator.h"

class KernelAllocator
{
	/* static const member */
public:
	static const unsigned int MEMORY_MAP_ARRAY_SIZE =	0x200;		/* 最多可分配512个对象 */
	static const unsigned int KERNEL_HEAP_START_ADDR =	0x180000 + 0xC0000000;	/* 1.5M处开始为内核堆区，但是整个内核从0xC0000000才开始 */
	static const unsigned int KERNEL_HEAP_SIZE =		0x80000;	/* 512K内核堆大小 */

	/* Functions */
public:
	KernelAllocator(Allocator* allocator);
	~KernelAllocator();
	int Initialize();

	unsigned long AllocMemory(unsigned long size);
	unsigned long FreeMemeory(unsigned long size, unsigned long memoryStartAddress);

	/* Members */
public:
	MapNode map[KernelAllocator::MEMORY_MAP_ARRAY_SIZE];

private:
	Allocator* m_pAllocator;
};

#endif
