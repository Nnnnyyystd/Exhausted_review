#include "TestSwapperManager.h"

void PrintMapNode(SwapperManager& swapMgr, int index)
{
	Diagnose::Write("MapNode[%d]: AddrIdx = [%d], Size = [%d]\n", index, swapMgr.map[index].m_AddressIdx, swapMgr.map[index].m_Size);
}

void PrintAllMapNode(SwapperManager& swapMgr)
{
	for ( unsigned int i = 0; i < SwapperManager::SWAPPER_MAP_ARRAY_SIZE; i++ )
	{
		if ( swapMgr.map[i].m_Size != 0 )
		{
			Diagnose::Write("MapNode[%d]: AddrIdx = [%d], Size = [%d]\n", i, swapMgr.map[i].m_AddressIdx, swapMgr.map[i].m_Size);
			Delay();
		}
		else
		{
			break;
		}
	}
}

bool TestSwapperManager()
{
	SwapperManager& swapMgr = Kernel::Instance().GetSwapperManager();
	int ans;
	
	ans = swapMgr.AllocSwap(4000); /* bytes, need 4096 / 512 = 8 sectors  */
	
	swapMgr.AllocSwap(SwapperManager::BLOCK_SIZE * 2);

	swapMgr.FreeSwap(2000, ans + 4);

	ans = swapMgr.AllocSwap(SwapperManager::BLOCK_SIZE * 10);

	swapMgr.FreeSwap(SwapperManager::BLOCK_SIZE * 10, ans);

	//PrintAllMapNode(swapMgr);

	swapMgr.AllocSwap(513 /* byte */ );

	PrintAllMapNode(swapMgr);

	return true;
}
