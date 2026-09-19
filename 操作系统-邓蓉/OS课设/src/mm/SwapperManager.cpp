#include "SwapperManager.h"

unsigned int SwapperManager::SWAPPER_ZONE_START_BLOCK = 18000;
unsigned int SwapperManager::SWAPPER_ZONE_SIZE = 2000;

SwapperManager::SwapperManager(Allocator *pAllocator)
{
	this->m_pAllocator = pAllocator;
}

SwapperManager::SwapperManager()
{
	//nothing to do here
}

SwapperManager::~SwapperManager()
{
	//nothing to do here
}

int SwapperManager::Initialize()
{
	for ( unsigned int i = 0; i < SWAPPER_MAP_ARRAY_SIZE; i++ )
	{
		this->map[i].m_AddressIdx = 0;
		this->map[i].m_Size = 0;
	}

	this->map[0].m_AddressIdx = SWAPPER_ZONE_START_BLOCK;
	this->map[0].m_Size = SWAPPER_ZONE_SIZE;

	return 0;
}

int SwapperManager::AllocSwap( unsigned long size )
{
	return this->m_pAllocator->Alloc(this->map, ( size + (BLOCK_SIZE - 1) ) / BLOCK_SIZE);
}

int SwapperManager::FreeSwap( unsigned long size, int startBlock )
{
	return this->m_pAllocator->Free(this->map, ( size + (BLOCK_SIZE - 1) ) / BLOCK_SIZE, startBlock);
}
