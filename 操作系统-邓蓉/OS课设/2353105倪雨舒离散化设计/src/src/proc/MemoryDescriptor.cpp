#include "MemoryDescriptor.h"
#include "Kernel.h"
#include "PageManager.h"
#include "Machine.h"
#include "PageDirectory.h"
#include "Video.h"

void MemoryDescriptor::Initialize()
{
	/* m_UserPageTableArray需要把AllocMemory()返回的物理内存地址 + 0xC0000000 */
	KernelPageManager &kernelPageManager = Kernel::Instance().GetKernelPageManager();
	// ------------- NOTE 3: 进程每次重新申请一张用户页表用相对表指针记录 -------------
	// this->m_UserPageTableArray = (PageTable*)(kernelPageManager.AllocMemory(sizeof(PageTable) * USER_SPACE_PAGE_TABLE_CNT) + Machine::KERNEL_SPACE_START_ADDRESS);
	this->m_UserPageTableArray = (PageTable *)(kernelPageManager.AllocMemory(sizeof(PageTable)) + Machine::KERNEL_SPACE_START_ADDRESS);
	// ------------- END NOTE 3 -------------
}

void MemoryDescriptor::Release()
{
	KernelPageManager &kernelPageManager = Kernel::Instance().GetKernelPageManager();
	if (this->m_UserPageTableArray)
	{
		// ------------- NOTE 3: 只释放一张用户页表 -------------
		// kernelPageManager.FreeMemory(sizeof(PageTable) * USER_SPACE_PAGE_TABLE_CNT, (unsigned long)this->m_UserPageTableArray - Machine::KERNEL_SPACE_START_ADDRESS);
		kernelPageManager.FreeMemory(sizeof(PageTable), (unsigned long)this->m_UserPageTableArray - Machine::KERNEL_SPACE_START_ADDRESS);
		this->m_UserPageTableArray = NULL;
		// ------------- END NOTE 3 -------------
	}
}

//unsigned int MemoryDescriptor::MapEntry(unsigned long virtualAddress, unsigned int size, unsigned long phyPageIdx, bool isReadWrite)
//{
//	unsigned long address = virtualAddress - USER_SPACE_START_ADDRESS;
//	unsigned long startIdx = (address >> 12) - PageTable::ENTRY_CNT_PER_PAGETABLE;
//	unsigned long cnt = (size + (PageManager::PAGE_SIZE - 1)) / PageManager::PAGE_SIZE;
//
//	// 1 号页表的地址
//	PageTableEntry *entrys = (PageTableEntry *)this->m_UserPageTableArray;
//	for (unsigned int i = startIdx; i < startIdx + cnt; i++, phyPageIdx++)
//	{
//		entrys[i].m_Present = 0x1;
//		entrys[i].m_UserSupervisor = 0x1;
//		entrys[i].m_ReadWriter = isReadWrite;
//		entrys[i].m_PageBaseAddress = phyPageIdx;
//	}
//	return phyPageIdx;
//}
//
//void MemoryDescriptor::MapTextEntrys(unsigned long textStartAddress, unsigned long textSize, unsigned long textPageIdx)
//{
//	this->MapEntry(textStartAddress, textSize, textPageIdx, false);
//}
//void MemoryDescriptor::MapDataEntrys(unsigned long dataStartAddress, unsigned long dataSize, unsigned long dataPageIdx)
//{
//	this->MapEntry(dataStartAddress, dataSize, dataPageIdx, true);
//}
//
//void MemoryDescriptor::MapStackEntrys(unsigned long stackSize, unsigned long stackPageIdx)
//{
//	unsigned long stackStartAddress = (USER_SPACE_START_ADDRESS + USER_SPACE_SIZE - stackSize) & 0xFFFFF000;
//	this->MapEntry(stackStartAddress, stackSize, stackPageIdx, true);
//}

PageTable *MemoryDescriptor::GetUserPageTableArray()
{
	return this->m_UserPageTableArray;
}
unsigned long MemoryDescriptor::GetTextStartAddress()
{
	return this->m_TextStartAddress;
}
unsigned long MemoryDescriptor::GetTextSize()
{
	return this->m_TextSize;
}
unsigned long MemoryDescriptor::GetDataStartAddress()
{
	return this->m_DataStartAddress;
}
unsigned long MemoryDescriptor::GetDataSize()
{
	return this->m_DataSize;
}
unsigned long MemoryDescriptor::GetStackSize()
{
	return this->m_StackSize;
}

void MemoryDescriptor::ClearUserPageTable()
{
	User &u = Kernel::Instance().GetUser();
	PageTable *pUserPageTable = u.u_MemoryDescriptor.m_UserPageTableArray;

	unsigned int j;

	// ------------- NOTE 3: 只负责一张用户页表的清除 -------------
	for (j = 0; j < PageTable::ENTRY_CNT_PER_PAGETABLE; j++)
	{
		pUserPageTable->m_Entrys[j].m_Present = 0;
		pUserPageTable->m_Entrys[j].m_ReadWriter = 0;
		pUserPageTable->m_Entrys[j].m_UserSupervisor = 1;
		pUserPageTable->m_Entrys[j].m_PageBaseAddress = 0;
	}
	// ------------- END NOTE 3 -------------
}

// ------------- NOTE 3: MapToPageTable函数将1#用户页表页框号写到私有的页目录 -------------
void MemoryDescriptor::MapToPageTable()
{
	User &u = Kernel::Instance().GetUser();

	if (NULL == u.u_MemoryDescriptor.m_UserPageTableArray)
	{
		return;
	}

	FlushPageDirectory(u.u_procp->GetPageDirectoryPhyAddr());
}
// ------------- END NOTE 3 -------------
