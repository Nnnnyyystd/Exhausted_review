#ifndef MACHINE_H
#define	MACHINE_H

#include "IDT.h"
#include "GDT.h"
#include "PageDirectory.h"
#include "TaskStateSegment.h"

/*
 * Machine类用于封装对底层硬件、保护模式下数据结构的抽象。
 * 包括对8254时钟芯片、8259A中断控制芯片的初始化，以及对
 * 保护模式下GDT, IDT等数据结构的操作。
 * 
 * Machine类使用Singleton模式实现，在系统内核整个生命周期
 * 中只有一个实例对象。
 */
class Machine
{
	/* static const member */
public:
	/* 内核代码段、内核数据段，用户代码段、用户数据段，TSS段的选择子 */
	static const unsigned int KERNEL_CODE_SEGMENT_SELECTOR = 0x08;
	static const unsigned int KERNEL_DATA_SEGMENT_SELECTOR = 0x10;
	static const unsigned int USER_CODE_SEGMENT_SELECTOR = (0x18 | 0x3);
	static const unsigned int USER_DATA_SEGMENT_SELECTOR = (0x20 | 0x3);		
	static const unsigned int TASK_STATE_SEGMENT_SELECTOR = 0x28;
	static const unsigned int TASK_STATE_SEGMENT_IDX = 0x5;	/* TSS段描述符在GDT中的位置 */

	/* 页目录、核心态页表、用户态页表在物理内存中的起始地址 */
	static const unsigned long PAGE_DIRECTORY_BASE_ADDRESS = 0x200000;
	static const unsigned long KERNEL_PAGE_TABLE_BASE_ADDRESS = 0x201000;
	static const unsigned long USER_PAGE_TABLE_BASE_ADDRESS = 0x202000;
	static const unsigned long USER_PAGE_TABLE_CNT = 2;
	
	/* 内核空间大小 4M 0xC0000000 - 0xC0400000 1 PageTable */
	static const unsigned int KERNEL_SPACE_SIZE = 0x400000;
	static const unsigned long KERNEL_SPACE_START_ADDRESS	= 0xC0000000;
	
public:
	static Machine& Instance();			/* 返回单态类的instance */
	void LoadIDT();						/* 把建立好的IDT表的基地址和长度加载进IDTR寄存器 */
	void LoadGDT();						/* 把建立好的GDT表的基地址和长度加载进IDTR寄存器 */
	void LoadTaskRegister();

	void InitIDT();
	void InitGDT();
	void InitPageDirectory();
	void InitUserPageTable();
	void InitTaskStateSegment();
	void EnablePageProtection();
	
	/* property functions */
	IDT& GetIDT();						/* 获取当前正在使用的IDT */
	GDT& GetGDT();						/* 获取当前正在使用的GDT */
	PageDirectory& GetPageDirectory();	/* 获取当前正在使用的页目录表 */
	PageTable& GetKernelPageTable();	/* 获取操作系统内核所使用的页表，用于map 0xc0000000以上地址 */
	PageTable* GetUserPageTableArray();	/* 获取用户进程页表，共两张，被映射在0x202000和0x203000上，
										    映射0x00000000 - 0x00800000用户态地址空间 */
	TaskStateSegment& GetTaskStateSegment();
	
private:
	static Machine instance;	/* Machine单体类实例 */
	
	IDT* m_IDT;
	GDT* m_GDT;
	PageDirectory* m_PageDirectory;	
	PageTable*	m_KernelPageTable;
	PageTable*	m_UserPageTable;
	TaskStateSegment* m_TaskStateSegment;
};

#endif
