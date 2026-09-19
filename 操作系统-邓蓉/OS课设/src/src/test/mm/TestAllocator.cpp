#include "Allocator.h"
#include "..\TestUtility.h"

bool TestAllocator()
{
	//Setup
	MapNode map[10]={0};
	map[0].m_Size = 100;
	map[0].m_AddressIdx = 0;

	Allocator* allocator = new Allocator();
	
	//TestCases

	unsigned long result = 0;

	//Case1
	result = allocator->Alloc(map, 5);
	PrintResult(
		"Case1", 
		result == 0 && map[0].m_AddressIdx == 5 && map[0].m_Size == 95
		);

	//Case2
	result = allocator->Alloc(map, 5);
	allocator->Free(map, 5, 0);
	PrintResult(
		"Case2", 
		result == 5 
		&& map[0].m_AddressIdx == 0 && map[0].m_Size == 5
		&& map[1].m_AddressIdx == 10 && map[1].m_Size ==90
		);

	//Case3
	result = allocator->Alloc(map, 5);
	allocator->Free(map, 1, 5);
	PrintResult(
		"Case3", 
		result == 0 
		&& map[0].m_AddressIdx == 5 && map[0].m_Size ==1
		&& map[1].m_AddressIdx == 10 && map[1].m_Size == 90
		);

	//Case4
	allocator->Free(map, 1, 9);
	PrintResult(
		"Case4", 
		map[0].m_AddressIdx == 5 && map[0].m_Size == 1
		&& map[1].m_AddressIdx == 9 && map[1].m_Size == 91
		);

	//Case5
	allocator->Free(map, 1, 7);
	PrintResult(
		"Case5", 
		map[0].m_AddressIdx == 5 && map[0].m_Size == 1
		&& map[1].m_AddressIdx == 7 && map[1].m_Size == 1
		&& map[2].m_AddressIdx == 9 && map[2].m_Size == 91
		);

	//Case6
	allocator->Free(map, 1, 6);
	PrintResult(
		"Case6", 
		map[0].m_AddressIdx == 5 && map[0].m_Size == 3
		&& map[1].m_AddressIdx == 9 && map[1].m_Size == 91
		);

	//Case7
	allocator->Free(map, 1, 8);
	PrintResult(
		"Case7", 
		map[0].m_AddressIdx == 5 && map[0].m_Size == 95
		);

	//Case8
	allocator->Free(map, 5, 0);
	PrintResult(
		"Case8", 
		map[0].m_AddressIdx == 0 && map[0].m_Size == 100
		);

	//Case9
	allocator->Alloc(map, 1);
	allocator->Alloc(map, 1);
	allocator->Free(map, 1, 0);
	result = allocator->Alloc(map, 2);
	PrintResult(
		"Case9", 
		map[0].m_AddressIdx == 0 && map[0].m_Size == 1
		&& map[1].m_AddressIdx == 4 && map[1].m_Size == 96
		);

	//TearDown
	delete allocator;

	return 0;
}



