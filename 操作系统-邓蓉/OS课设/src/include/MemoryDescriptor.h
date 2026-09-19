#ifndef MEMORY_DESCRIPTOR_H
#define MEMORY_DESCRIPTOR_H

#include "PageTable.h"

class MemoryDescriptor
{
public:
	/* 用户空间大小 8M 0x0 - 0x800000 2 PageTable */
	static const unsigned int USER_SPACE_SIZE	= 0x800000; 
	static const unsigned int USER_SPACE_PAGE_TABLE_CNT = 0x2;
	static const unsigned long USER_SPACE_START_ADDRESS		= 0x0;



public:
	MemoryDescriptor();
	~MemoryDescriptor();

public:
	/* 申请并初始化PageDirectory，在做Map操作前使用 */
	void Initialize();
	/* 在释放进程时，需要调用该操作释放被占用的页表 */
	void Release();

	/* 以下函数用户完成对user结构中页表Entry的填充，该页表在进程切换时填充现有的页表 */
	void MapTextEntrys(unsigned long textStartAddress, unsigned long textSize, unsigned long textPageIdxInPhyMemory);
	void MapDataEntrys(unsigned long dataStartAddress, unsigned long dataSize, unsigned long dataPageIdxInPhyMemory);
	void MapStackEntrys(unsigned long stackSize, unsigned long stackPageIdxInPhyMemory);

	/* @comment 原unixv6中sureg()函数.原函数用于将进程u区中的uisa和uisd两数组中的内存页映射数据映射到UISA与UISD
	 * 寄存器中.由于体系结构的关系，使用MapToPageTable()函数将MemoryDescriptor中的页表copy到操作系统正使用的
	 * PageTable中，然后使用FlushPageDirectory()函数完成页表映射，新上台进程的用户区数据映射完成 */
	void MapToPageTable();
	void DisplayPageTable();

	/* 
	 * @comment 原unix v6中estabur()函数，用于建立用户态地址空间的相对地址映射表，然后调用
	 * MapToPageTable()函数将相对地址映射表加载到用户态页表中。
	 */
	bool EstablishUserPageTable(unsigned long textVirtualAddress, unsigned long textSize, unsigned long dataVirtualAddress, unsigned long dataSize, unsigned long stackSize);
	void ClearUserPageTable();
	PageTable* GetUserPageTableArray();
	unsigned long GetTextStartAddress();
	unsigned long GetTextSize();
	unsigned long GetDataStartAddress();
	unsigned long GetDataSize();
	unsigned long GetStackSize();

private:
	/* @comment设置页表目录项
	 * @param
	 * unsigned long virtualAddress:	虚拟地址(以字节为单位) 
	 * unsigned int size:				需要映射的虚拟地址大小(以字节为单位) 
	 * unsigned long phyPageIdx:		其实物理页索引号(页为单位)		
	 * bool isReadWrite:				页属性，true为可读可写页
	 */
	unsigned int MemoryDescriptor::MapEntry(unsigned long virtualAddress, unsigned int size, unsigned long phyPageIdx, bool isReadWrite);
	
public:
	PageTable*		m_UserPageTableArray;
	/* 以下数据都是线性地址 */
	unsigned long	m_TextStartAddress;	/* 代码段起始地址 */
	unsigned long	m_TextSize;			/* 代码段长度 */

	unsigned long	m_DataStartAddress; /* 数据段起始地址 */
	unsigned long	m_DataSize;			/* 数据段长度 */

	unsigned long	m_StackSize;		/* 栈段长度 */
	//unsigned long	m_HeapSize;			/* 堆段长度 */
};

#endif

