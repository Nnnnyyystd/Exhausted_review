#include "..\TestUtility.h"
#include "TestNew.h"

#include "KernelAllocator.h"
#include "Allocator.h"
#include "New.h"

class DummyClass
{
public:
	DummyClass();
	~DummyClass();
	int buffer[100];
};

DummyClass::DummyClass()
{
	int i = 0;
	i++;
}

DummyClass::~DummyClass()
{
	int j = 0;
	j--;
}


bool TestNew()
{
	Allocator allocator;
	KernelAllocator kAllocator(&allocator);
	kAllocator.Initialize();
	set_kernel_allocator(&kAllocator);

	unsigned char buffer[1024];
	kAllocator.map[0].m_AddressIdx = (unsigned long)&buffer;
	kAllocator.map[0].m_Size = 1024;

	
	DummyClass* p = 0;
	//Case1 new
	p = new DummyClass();
	PrintResult(
		"Case1", 
		(unsigned long)p == ((unsigned long)&buffer + 4)
		&& kAllocator.map[0].m_AddressIdx == (unsigned long)&buffer + sizeof(DummyClass) + sizeof(int) 
		&& kAllocator.map[0].m_Size == 1024 - (sizeof(DummyClass) + sizeof(int))
		);

	//Case2 delete
	delete (void*)p;
	PrintResult(
		"Case2",
		kAllocator.map[0].m_AddressIdx == (unsigned long)&buffer 
		&& kAllocator.map[0].m_Size == 1024
		);
	return true;
}