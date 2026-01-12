#include "PageManager.h"
#include "..\TestUtility.h"

bool TestPageManager()
{
	//Setup
	Allocator allocator;
	PageManager manager(&allocator);
	manager.Initialize();

	//TestCases
	unsigned long result = 0;
	unsigned long startidx = manager.map[0].m_AddressIdx;
	
	//Case1
	result = manager.AllocMemory(4096);
	PrintResult(
		"Case1", 
		result == PageManager::KERNEL_MEM_START_ADDR + PageManager::KERNEL_SIZE && 
		manager.map[0].m_AddressIdx - 1 == startidx
		);

	//Case2
	result = manager.FreeMemory(4096, result);
	PrintResult(
		"Case2", 
		manager.map[0].m_AddressIdx == startidx
		);

	//TearDown
	return true;
}