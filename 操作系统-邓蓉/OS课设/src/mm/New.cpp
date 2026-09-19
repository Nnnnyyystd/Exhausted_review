#include "New.h"
/*
 * 这个文件有问题
 * 1、返回的是空间的物理地址
 * 2、new后面的参数是需要的空间大小，没法申请可变长数组，参见PEParser.cpp中  line 157.
 */

KernelAllocator* g_pAllocator;

void set_kernel_allocator(KernelAllocator* pAllocator)
{
	g_pAllocator = pAllocator;
}

void* operator new (unsigned int size)
{

	unsigned long address = g_pAllocator->AllocMemory(size + sizeof(int));
	if ( address )
	{
		int* pSize = (int*)address;
		*pSize = size;
		return (void*)(address + sizeof(int));
	}
	else
	{
		return 0;
	}
}

void operator delete (void* p)
{
	unsigned long address = (unsigned long)p;
	if ( address )
	{
		int* pSize = (int*)(address - sizeof(int));
		g_pAllocator->FreeMemeory(*pSize + sizeof(int), (unsigned long)pSize);
	}
	return;
}

