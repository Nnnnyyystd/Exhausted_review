#include "Text.h"
#include "Kernel.h"

Text::Text()
{
	// nothing to do here
}

Text::~Text()
{
	// nothing to do here
}

void Text::XccDec()
{
	User &u = Kernel::Instance().GetUser();
	PageTable *pUserPageTable = u.u_MemoryDescriptor.m_UserPageTableArray;
	UserPageManager &userPageMgr = Kernel::Instance().GetUserPageManager();

	if (this->x_ccount == 0)
		return;

	--this->x_ccount;  	// 如果x_ccount递减至0，释放共享正文段占据的内存

	int index = u.u_MemoryDescriptor.m_DataStartAddress >> 12 - 1024;
	int count = (u.u_MemoryDescriptor.m_DataSize + PageManager::PAGE_SIZE - 1) / PageManager::PAGE_SIZE;
	int frame;
	while (count)
	{
		if(this->x_ccount==0)
		{
			frame = pUserPageTable->m_Entrys[index].m_PageBaseAddress;  // 释放物理页框
			userPageMgr.FreeMemory(PageManager::PAGE_SIZE, frame << 12);
		}
		pUserPageTable->m_Entrys[index].m_Present = 0;				// 清空PTE
		pUserPageTable->m_Entrys[index].m_ReadWriter = 0;
		pUserPageTable->m_Entrys[index].m_UserSupervisor = 1;
		pUserPageTable->m_Entrys[index].m_PageBaseAddress = 0;

		index++; 		count--;
	}
}

void Text::XFree()
{
	/* 释放内存中的副本 */
	this->XccDec();

	/* 释放盘交换区上的副本 */
	if (--this->x_count == 0)
	{
		Kernel::Instance().GetSwapperManager().FreeSwap(this->x_size, this->x_daddr);
		Kernel::Instance().GetFileManager().m_InodeTable->IPut(this->x_iptr);
		this->x_iptr = NULL;
	}
}
