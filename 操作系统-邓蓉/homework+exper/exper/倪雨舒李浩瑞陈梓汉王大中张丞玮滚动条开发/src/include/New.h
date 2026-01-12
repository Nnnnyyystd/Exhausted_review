#ifndef NEW_H
#define NEW_H

#include "KernelAllocator.h"

void set_kernel_allocator(KernelAllocator* pAllocator);
void* operator new (unsigned int size);
void operator delete (void* p);

#endif

