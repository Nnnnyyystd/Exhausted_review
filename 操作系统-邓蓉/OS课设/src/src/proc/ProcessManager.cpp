#include "ProcessManager.h"
#include "Machine.h"
#include "User.h"
#include "Kernel.h"
#include "Video.h"
#include "Utility.h"
#include "PEParser.h"
#include "Regs.h"
#include "MemoryDescriptor.h"

unsigned int ProcessManager::m_NextUniquePid = 0;

ProcessManager::ProcessManager()
{
	CurPri = 0;
	RunRun = 0;
	RunIn = 0;
	RunOut = 0;
	ExeCnt = 0;
	SwtchNum = 0;
}

ProcessManager::~ProcessManager()
{
}

void ProcessManager::Initialize()
{
	// nothing to do here
}

// ------------- NOTE 3: 进程创建页目录并引用内核页表 -------------
PageDirectory *ProcessManager::AllocPageDirectory()
{
	// ! 内存中相对表和页目录都是线性地址
	KernelPageManager &kernelPageManager = Kernel::Instance().GetKernelPageManager();
	return (PageDirectory *)(kernelPageManager.AllocMemory(sizeof(PageDirectory)) + Machine::KERNEL_SPACE_START_ADDRESS);
}

void ProcessManager::InitProcPageDirectory(Process *proc, PageTable * privateUsrPageTable)
{
	PageDirectory *pPageDirectory = proc->pPageDirectory;
	if (pPageDirectory)
	{
		// 引用公用的0#用户页表
		pPageDirectory->m_Entrys[0].m_UserSupervisor = 1;
		pPageDirectory->m_Entrys[0].m_Present = 1;
		pPageDirectory->m_Entrys[0].m_ReadWriter = 1;
		pPageDirectory->m_Entrys[0].m_PageTableBaseAddress = Machine::USER_PAGE_TABLE_BASE_ADDRESS >> 12;
		// 引用核心页表
		unsigned int kPageTableIdx = Machine::KERNEL_SPACE_START_ADDRESS / PageTable::SIZE_PER_PAGETABLE_MAP;
		pPageDirectory->m_Entrys[kPageTableIdx].m_UserSupervisor = 0;
		pPageDirectory->m_Entrys[kPageTableIdx].m_Present = 1;
		pPageDirectory->m_Entrys[kPageTableIdx].m_ReadWriter = 1;
		pPageDirectory->m_Entrys[kPageTableIdx].m_PageTableBaseAddress = Machine::KERNEL_PAGE_TABLE_BASE_ADDRESS >> 12;
		// 引用私有的1#用户页表
		unsigned long phyFrame = ((unsigned long)privateUsrPageTable - Machine::KERNEL_SPACE_START_ADDRESS) >> 12;
		pPageDirectory->m_Entrys[1].m_PageTableBaseAddress = phyFrame;
		pPageDirectory->m_Entrys[1].m_UserSupervisor = 1;
		pPageDirectory->m_Entrys[1].m_Present = 1;
		pPageDirectory->m_Entrys[1].m_ReadWriter = 1;
	}
}
// ------------- END NOTE 3 -------------

void ProcessManager::SetupProcessZero()
{
	// 初始化Process#0的Process和User结构
	Process *pProcZero = &(this->process[0]);
	pProcZero->p_stat = Process::SRUN;
	pProcZero->p_flag = Process::SLOAD | Process::SSYS;
	pProcZero->p_nice = 0;
	pProcZero->p_time = 0;
	pProcZero->p_pid = NextUniquePid();
	// 除ppda区与核心栈外，进程没有用户态部分
	pProcZero->p_size = 0x1000;
	pProcZero->p_addr = PROCESS_ZERO_PPDA_ADDRESS;
	pProcZero->p_textp = NULL;

	User &u = Kernel::Instance().GetUser();
	u.u_procp = pProcZero;
	u.u_MemoryDescriptor.m_TextStartAddress = 0;
	u.u_MemoryDescriptor.m_TextSize = 0;
	u.u_MemoryDescriptor.m_DataStartAddress = 0;
	u.u_MemoryDescriptor.m_DataSize = 0;
	u.u_MemoryDescriptor.m_StackSize = 0;
	u.u_MemoryDescriptor.m_UserPageTableArray = NULL;
	// u.u_MemoryDescriptor.Initialize();
	// ------------- NOTE 3: 写0#进程页目录 -------------
	pProcZero->pPageDirectory = (PageDirectory *)(Machine::PAGE_DIRECTORY_BASE_ADDRESS + Machine::KERNEL_SPACE_START_ADDRESS);
	// ------------- END NOTE 3 -------------
}

unsigned int ProcessManager::NextUniquePid()
{
	return ProcessManager::m_NextUniquePid++;
}

// ------------- NOTE COW: 修改父进程页表中的RW标识 -------------
void ProcessManager::ModifyPageTable(UserPageManager &userPageManager, PageTable* pgTable) {
	for (unsigned int j = 0; j < PageTable::ENTRY_CNT_PER_PAGETABLE; j++) {
		if (pgTable->m_Entrys[j].m_Present && pgTable->m_Entrys[j].m_ReadWriter) {
			pgTable->m_Entrys[j].m_ReadWriter = 0;
			userPageManager.Page[pgTable->m_Entrys[j].m_PageBaseAddress]++;
		}
	}
}
// ------------- END NOTE COW -------------

int ProcessManager::NewProc()
{
	// ------------- NOTE 1: 为子进程分配process结构  -------------
	Process *child = 0;
	for (int i = 0; i < ProcessManager::NPROC; i++)
	{
		if (process[i].p_stat == Process::SNULL)
		{
			child = &process[i];
			break;
		}
	}
	if (!child)
	{
		Utility::Panic("No Proc Entry!");
	}
	// ------------- END NOTE ------------------------------------

	// ------------- NOTE 2: 为子进程分配pid，并复制父进程Process结构  -------------
	User &u = Kernel::Instance().GetUser();
	Process *current = (Process *)u.u_procp;
	current->Clone(*child);
	// ------------- END NOTE ---------------------------------------------------

	PageTable *pgTable = u.u_MemoryDescriptor.m_UserPageTableArray;  // 父进程用户态页表指针

	// ------------- NOTE 3: 为子进程创建二级页表  -------------
	u.u_MemoryDescriptor.Initialize();  // 分配1#用户页表
	PageTable *desPgTable = u.u_MemoryDescriptor.m_UserPageTableArray;
	child->pPageDirectory = ProcessManager::AllocPageDirectory();   // 分配页目录
	ProcessManager::InitProcPageDirectory(child, desPgTable);   // 写页目录
	// ------------- END NOTE 3 -------------

	// ------------- NOTE 4: 为子进程PPDA区分配物理页框，复制父进程的PPDA区 -------------
	UserPageManager &userPageManager = Kernel::Instance().GetUserPageManager();
	unsigned long desAddress = userPageManager.AllocMemory(PageManager::PAGE_SIZE);

	SaveU(u.u_rsav);
	u.u_procp = child;

	if (desAddress == 0)
	{
		/* 暂不考虑内存不够，需要swap的情况 */
	}

	child->p_addr = desAddress;
	Utility::CopySeg(current->p_addr, desAddress);
	// ------------- END NOTE 4 -------------

	// ------------- NOTE 5: 复制父进程的1#用户页表 -------------
	if (pgTable) {
		// ------------- NOTE COW: 修改父进程页表中的RW标识并且增加引用计数 -------------
		ProcessManager::ModifyPageTable(userPageManager, pgTable);
		// ------------- END NOTE COW -------------
		Utility::MemCopy((unsigned long)pgTable, (unsigned long)desPgTable, sizeof(PageTable));
	}
	// ------------- END NOTE 5 --------------------------------
	
	u.u_procp = current;
	u.u_MemoryDescriptor.m_UserPageTableArray = pgTable;
	return 0;
}

/* 在进程切换的过程中，根本没有用到TSS */
int ProcessManager::Swtch()
{
	// Diagnose::Write("Start Swtch()\n");
	User &u = Kernel::Instance().GetUser();
	SaveU(u.u_rsav);

	/* 0#进程上台*/
	Process *procZero = &process[0];

	/*
	 * 将SwtchUStruct()和RetU()作为临界区，防止被中断打断。
	 * 如果在RetU()恢复esp之后，尚未恢复ebp时，中断进入会导致
	 * esp和ebp分别指向两个不同进程的核心栈中位置。 good comment！
	 *
	 * 为什么，由0#进程承担挑选就绪进程上台的操作？
	 * 单从进程切换的角度，完全可以由下台进程挑选就绪进程上台。 但是，考虑时钟中断。
	 * 一秒末的 例行处理，最好系统idle时，其次是在执行应用程序过程中；不可以放在内核执行过程中。
	 * 如何判断？
	 * 内核idle的标志：  0#进程在睡眠态执行idle()子程序。
	 * 看 TimeInterrupt.cpp的Line 82.
	 * 如是，必须由0#进程执行select()。
	 *
	 */
	X86Assembly::CLI();
	SwtchUStruct(procZero);
	/* 原来的宏调用是这样写的   RetU(u0)，u0参数没用到，会引起歧义，删除 */
	RetU();
	X86Assembly::STI();

	/* 挑选最适合上台的进程 */
	Process *selected = Select();
	// Diagnose::Write("Process id = %d Selected!\n", selected->p_pid);

	/* 恢复被保存进程的现场 */
	X86Assembly::CLI();
	SwtchUStruct(selected);

	RetU();
	X86Assembly::STI();

	User &newu = Kernel::Instance().GetUser();

	newu.u_MemoryDescriptor.MapToPageTable();

	/*
	 * If the new process paused because it was
	 * swapped out, set the stack level to the last call
	 * to savu(u_ssav).  This means that the return
	 * which is executed immediately after the call to aretu
	 * actually returns from the last routine which did
	 * the savu.
	 *
	 * You are not expected to understand this.
	 */
	if (newu.u_procp->p_flag & Process::SSWAP)
	{
		newu.u_procp->p_flag &= ~Process::SSWAP;
		aRetU(newu.u_ssav);
	}

	/*
	 * 被fork出的进程在上台之前会在被调度上台时返回1，
	 * 并同时返回到NewProc()执行的地址
	 */
	return 1;
}

void ProcessManager::Sched()    // 不考虑内存不足的情况，这个函数没有作用
{
	Process *pSelected;
	User &u = Kernel::Instance().GetUser();
	int seconds;
	unsigned int size;
	unsigned long desAddress;

	/*
	 * 选择在交换区驻留时间最长，处于就绪状态的进程换入
	 */
	goto loop;

sloop:
	this->RunIn++;
	u.u_procp->Sleep((unsigned long)&RunIn, ProcessManager::PSWP);

loop:
	X86Assembly::CLI();
	seconds = -1;
	for (int i = 0; i < ProcessManager::NPROC; i++)
	{
		if (this->process[i].p_stat == Process::SRUN && (this->process[i].p_flag & Process::SLOAD) == 0 && this->process[i].p_time > seconds)
		{
			pSelected = &(this->process[i]);
			seconds = pSelected->p_time;
		}
	}

	/* 如果没有符合条件的进程，0#进程睡眠等待有需要换入的进程 */
	if (-1 == seconds)
	{
		this->RunOut++;
		u.u_procp->Sleep((unsigned long)&RunOut, ProcessManager::PSWP);
		goto loop;
	}

	/* 如果有进程满足条件，需要换入，则检查是否有足够内存 */
	X86Assembly::STI();
	/* 计算进程换入需要的内存大小 */
	size = pSelected->p_size;
	/*
	 * 如果存在共享正文段，但是没有进程图像在内存中，引用该正文段的进程，
	 * 即共享正文段不再内存中，换入时需要读入正文段在交换区中的副本
	 */
	if (pSelected->p_textp != NULL && 0 == pSelected->p_textp->x_ccount)
	{
		size += pSelected->p_textp->x_size;
	}
	/* 如果内存分配成功，则进行实际换入操作 */
	// Note:3 可能要改，改成逐页申请
	desAddress = Kernel::Instance().GetUserPageManager().AllocMemory(size);
	if (NULL != desAddress)
	{
		goto found2;
	}

	/*
	 * 分配内存失败情况下，换出内存中进程，腾出空间。
	 * 换出原则：从易到难；依次将低优先权睡眠状态(SWAIT)-->
	 * 暂停状态(SSTOP)-->高优先权睡眠状态(SSLEEP)-->就绪状态(SRUN)进程换出。
	 */
	X86Assembly::CLI();
	for (int i = 0; i < ProcessManager::NPROC; i++)
	{
		if (this->process[i].p_flag & (Process::SSYS | Process::SLOCK | Process::SLOAD) == Process::SLOAD && (this->process[i].p_stat == Process::SWAIT || this->process[i].p_stat == Process::SSTOP))
		{
			goto found1;
		}
	}

	/*
	 * 在换出高优先权睡眠状态(SSLEEP)、就绪状态(SRUN)进程而腾出内存之前，
	 * 检查待换入进程在交换区驻留时间是否已达到3秒，低于则不予换入
	 */
	if (seconds < 3)
	{
		goto sloop;
	}

	seconds = -1;
	for (int i = 0; i < ProcessManager::NPROC; i++)
	{
		if (this->process[i].p_flag & (Process::SSYS | Process::SLOCK | Process::SLOAD) == Process::SLOAD && (this->process[i].p_stat == Process::SWAIT || this->process[i].p_stat == Process::SSTOP) && pSelected->p_time > seconds)
		{
			pSelected = &(this->process[i]);
			seconds = pSelected->p_time;
		}
	}

	/* 如果要换出SSLEEP、SRUN状态进程，先检查该进程驻留内存时间是否超过2秒，否则不予换出 */
	if (seconds < 2)
	{
		goto sloop;
	}

	/* 换出pSelected指向的被选中进程 */
found1:
	X86Assembly::STI();
	pSelected->p_flag &= ~Process::SLOAD;
	this->XSwap(pSelected, true, 0);
	/* 腾出内存空间后再次尝试换入进程 */
	goto loop;

	/* 已经分配好足够的内存，进行实际的换入操作 */
found2:
	BufferManager &bufMgr = Kernel::Instance().GetBufferManager();
	/*
	 * 如果存在共享正文段，但是没有进程图像在内存中，引用该正文段的进程，
	 * 即共享正文段不再内存中，换入时需要读入正文段在交换区中的副本
	 */
	if (pSelected->p_textp != NULL)
	{
		Text *pText = pSelected->p_textp;
		if (pText->x_ccount == 0)
		{
			/* 因为共享正文段，和进程ppda、数据段、堆栈段在交换区中是分开存放的，所以先换入共享正文段 */
			if (bufMgr.Swap(pText->x_daddr, desAddress, pText->x_size, Buf::B_READ) == false)
			{
				goto err;
			}
			/* 共享正文段在内存中的起始地址。错的，要改 */
			pText->x_caddr[0] = desAddress;
			desAddress += pText->x_size;
		}
		pText->x_ccount++;
	}
	/* 换入剩余部分图像：ppda、数据段、堆栈段 */
	if (bufMgr.Swap(pSelected->p_addr /* blkno */, desAddress, pSelected->p_size, Buf::B_READ) == false)
	{
		goto err;
	}
	Kernel::Instance().GetSwapperManager().FreeSwap(pSelected->p_size, pSelected->p_addr /* blkno */);
	pSelected->p_addr = desAddress;
	pSelected->p_flag |= Process::SLOAD;
	pSelected->p_time = 0;
	goto loop;

err:
	Utility::Panic("Swap Error");
}

void ProcessManager::Wait()
{
	int i;
	bool hasChild = false;
	User &u = Kernel::Instance().GetUser();
	SwapperManager &swapperMgr = Kernel::Instance().GetSwapperManager();
	BufferManager &bufMgr = Kernel::Instance().GetBufferManager();

	Diagnose::Write("Process %d finding dead son. They are ", u.u_procp->p_pid);
	while (true)
	{
		for (i = 0; i < NPROC; i++)
		{
			if (u.u_procp->p_pid == process[i].p_ppid)
			{
				Diagnose::Write("Process %d (Status:%d)  ", process[i].p_pid, process[i].p_stat);
				hasChild = true;
				/* 睡眠等待直至子进程结束 */
				if (Process::SZOMB == process[i].p_stat)
				{
					/* wait()系统调用返回子进程的pid */
					u.u_ar0[User::EAX] = process[i].p_pid;

					process[i].p_stat = Process::SNULL;
					process[i].p_pid = 0;
					process[i].p_ppid = -1;
					process[i].p_sig = 0;
					process[i].p_flag = 0;

					/* 读入swapper中子进程u结构副本 */
					Buf *pBuf = bufMgr.Bread(DeviceManager::ROOTDEV, process[i].p_addr);
					swapperMgr.FreeSwap(BufferManager::BUFFER_SIZE, process[i].p_addr);
					User *pUser = (User *)pBuf->b_addr;

					/* 把子进程的时间加到父进程上 */
					u.u_cstime += pUser->u_cstime + pUser->u_stime;
					u.u_cutime += pUser->u_cutime + pUser->u_utime;

					int *pInt = (int *)u.u_arg[0];
					/* 获取子进程exit(int status)的返回值 */
					*pInt = pUser->u_arg[0];

					/* 如果此处没有Brelse()系统会发生什么-_- */
					bufMgr.Brelse(pBuf);
					Diagnose::Write("end wait\n");
					return;
				}
			}
		}
		if (true == hasChild)
		{
			/* 睡眠等待直至子进程结束 */
			Diagnose::Write("wait until child process Exit! ");
			u.u_procp->Sleep((unsigned long)u.u_procp, ProcessManager::PWAIT);
			Diagnose::Write("end sleep\n");
			continue; /* 回到外层while(true)循环 */
		}
		else
		{
			/* 不存在需要等待结束的子进程，设置出错码，wait()返回 */
			u.u_error = User::ECHILD;
			break; /* Get out of while loop */
		}
	}
}

void ProcessManager::Fork()
{
	User &u = Kernel::Instance().GetUser();
	Process *child = NULL;

	/* 寻找空闲的process项，作为子进程的进程控制块 */
	for (int i = 0; i < ProcessManager::NPROC; i++)
	{
		if (this->process[i].p_stat == Process::SNULL)
		{
			child = &this->process[i];
			break;
		}
	}
	if (child == NULL)
	{
		/* 没有空闲process表项，返回 */
		u.u_error = User::EAGAIN;
		return;
	}

	if (this->NewProc()) /* 子进程返回1，父进程返回0 */
	{
		/* 子进程fork()系统调用返回0 */
		u.u_ar0[User::EAX] = 0;
		u.u_cstime = 0;
		u.u_stime = 0;
		u.u_cutime = 0;
		u.u_utime = 0;
	}
	else
	{
		/* 父进程进程fork()系统调用返回子进程PID */
		u.u_ar0[User::EAX] = child->p_pid;
	}

	return;
}

extern "C" void runtime();
extern "C" void ExecShell();

/* 终于敢称为 V6 的 exec实现。缺憾：不支持 ISUID 比特 */
void ProcessManager::Exec()
{
	Inode *pInode;
	Text *pText;
	User &u = Kernel::Instance().GetUser();
	FileManager &fileMgr = Kernel::Instance().GetFileManager();
	UserPageManager &userPgMgr = Kernel::Instance().GetUserPageManager();
	KernelPageManager &kernelPgMgr = Kernel::Instance().GetKernelPageManager();
	BufferManager &bufMgr = Kernel::Instance().GetBufferManager();

	Diagnose::Write("Process %d execing\n", u.u_procp->p_pid);
	pInode = fileMgr.NameI(FileManager::NextChar, FileManager::OPEN);
	if (NULL == pInode) // 搜索目录失败
	{
		return;
	}

	/* 如果同时进行图像改换的进程数超出限制，则先进入睡眠 */
	while (this->ExeCnt >= NEXEC)
	{
		u.u_procp->Sleep((unsigned long)&ExeCnt, ProcessManager::EXPRI);
	}
	this->ExeCnt++;

	/* 进程必需拥有可执行文件的执行权限，且被执行的只能是一般文件。 */
	if (fileMgr.Access(pInode, Inode::IEXEC) || (pInode->i_mode & Inode::IFMT) != 0)
	{
		fileMgr.m_InodeTable->IPut(pInode);
		if (this->ExeCnt >= NEXEC)
		{
			WakeUpAll((unsigned long)&ExeCnt);
		}
		this->ExeCnt--;
		return;
	}

	PEParser parser;

	if (parser.HeaderLoad(pInode) == false)
	{
		fileMgr.m_InodeTable->IPut(pInode);
		return;
	}

//	/* 获取分析PE头结构得到正文段的起始地址、长度 */
//	u.u_MemoryDescriptor.m_TextStartAddress = parser.TextAddress;
//	u.u_MemoryDescriptor.m_TextSize = parser.TextSize;
//
//	/* 数据段的起始地址、长度 */
//	u.u_MemoryDescriptor.m_DataStartAddress = parser.DataAddress;
//	u.u_MemoryDescriptor.m_DataSize = parser.DataSize;
//
//	/* 堆栈段初始化长度 */
//	u.u_MemoryDescriptor.m_StackSize = parser.StackSize;

	if (parser.TextSize + parser.DataSize + parser.StackSize + PageManager::PAGE_SIZE > MemoryDescriptor::USER_SPACE_SIZE - parser.TextAddress)
	{
		fileMgr.m_InodeTable->IPut(pInode);
		u.u_error = User::ENOMEM;
		return;
	}

	X86Assembly::CLI();    //////////原子操作，接纳新程序的命令行参数//////////////////////////////////////////////////////////////////////////////////////////////
	int pages = (parser.StackSize + PageManager::PAGE_SIZE - 1) >> 12;
	int allocLength = pages << 12;   // 用户栈按页对齐

	unsigned long trueStack = (userPgMgr.AllocMemory(allocLength))>>12;  // 为用户栈分配物理页框，没有考虑内存不足的情况
	PageTableEntry *tempEntrys = (PageTableEntry *)Machine::Instance().GetUserPageTableArray();  // 用0#用户页表建立虚实地址映射关系，从1#PTE开始
	for (int i = 1; i <= pages; i++,trueStack++)
	{
		tempEntrys[i].m_UserSupervisor = 0x1;
		tempEntrys[i].m_Present = 0x1;
		tempEntrys[i].m_ReadWriter = true;
		tempEntrys[i].m_PageBaseAddress = trueStack;
	}
	trueStack -= pages;
	u.u_MemoryDescriptor.MapToPageTable();

	int argc = u.u_arg[1];  // 接纳命令行参数
	char **argv = (char **)u.u_arg[2];

	/* esp定位到栈底 */
	unsigned int esp = MemoryDescriptor::USER_SPACE_SIZE;
	unsigned long desAddress = 4096 + allocLength;

	int length;

	/* 复制argv[]指针数组指向的命令行参数字符串 */
	for (int i = 0; i < argc; i++)
	{
		length = 0;
		/* 计算参数字符串长度，length不含'\0' */
		while (NULL != argv[i][length])
		{
			length++;
		}
		desAddress = desAddress - (length + 1);
		/* 拷贝时将'\0'一起拷贝过去 */
		Utility::MemCopy((unsigned long)argv[i], desAddress, length + 1);
		/* 将参数字符串在新进程图像用户栈中的起始位置存入argv[i]，用户栈位于进程逻辑地址空间0x800000的底部 */
		esp = esp - (length + 1);
		argv[i] = (char *)esp;
	}

	/* 后续存放的是int型数值，这里以16字节边界对齐 */
	desAddress = desAddress & 0xFFFFFFF0;
	esp = esp & 0xFFFFFFF0;

	/* 复制argc和argv[] */
	int endValue = 0;
	desAddress -= sizeof(endValue);
	esp -= sizeof(endValue);
	/* 向用户栈中写入endValue作为argv[]的结束 */
	Utility::MemCopy((unsigned long)&endValue, desAddress, sizeof(endValue));

	desAddress -= argc * sizeof(int);
	esp -= argc * sizeof(int);
	/* 写入argv[]的内容 */
	Utility::MemCopy((unsigned long)argv, desAddress, argc * sizeof(int));

	/* 令endValue指向当前栈中argv[]的起始地址，即argv[]入栈完毕后当前栈顶地址 */
	endValue = esp;
	desAddress -= sizeof(int);
	esp -= sizeof(int);
	Utility::MemCopy((unsigned long)&endValue, desAddress, sizeof(int));

	/* 最后入栈argc */
	desAddress -= sizeof(int);
	esp -= sizeof(int);
	Utility::MemCopy((unsigned long)&argc, desAddress, sizeof(int)); /* Done! */

	for (int i = 1; i <= pages; i++)  // 解除映射关系
	{
		tempEntrys[i].m_UserSupervisor = 0;
		tempEntrys[i].m_Present = 0;
		tempEntrys[i].m_ReadWriter = false;
		tempEntrys[i].m_PageBaseAddress = 1;
	}
	u.u_MemoryDescriptor.MapToPageTable();
	X86Assembly::STI();     ///////////////////////////////////////////////////////////////////////////////////////////////////////////

	/* 释放原进程图像的共享正文段，数据段，堆栈段 */
	if (u.u_procp->p_textp != NULL)
	{
		u.u_procp->p_textp->XFree();
		u.u_procp->p_textp = NULL;
	}

	Process *current = u.u_procp;

	PageTable *pUserPageTable = u.u_MemoryDescriptor.m_UserPageTableArray;
	MemoryDescriptor &md = u.u_MemoryDescriptor;

	// 释放数据段
	int index = md.m_DataStartAddress >> 12 - 1024;
	int count = (md.m_DataSize + PageManager::PAGE_SIZE - 1) / PageManager::PAGE_SIZE;
	unsigned long frame;

	while (count)
	{
		frame = pUserPageTable->m_Entrys[index].m_PageBaseAddress;  // 释放物理页框
		userPgMgr.FreeMemory(PageManager::PAGE_SIZE, frame << 12);
		// userPgMgr.Page[frame]--;

		pUserPageTable->m_Entrys[index].m_Present = 0;				// 清空PTE
		pUserPageTable->m_Entrys[index].m_ReadWriter = 0;
		pUserPageTable->m_Entrys[index].m_UserSupervisor = 1;
		pUserPageTable->m_Entrys[index].m_PageBaseAddress = 0;

		index++; 		count--;
	}

	// 释放栈段
	index = (md.USER_SPACE_START_ADDRESS + md.USER_SPACE_SIZE - md.m_StackSize) >> 12 - 1024;
	count = (md.m_StackSize + PageManager::PAGE_SIZE - 1) / PageManager::PAGE_SIZE;
	while (count)
	{
		frame = pUserPageTable->m_Entrys[index].m_PageBaseAddress;  // 释放物理页框
		userPgMgr.FreeMemory(PageManager::PAGE_SIZE, frame << 12);
		// userPgMgr.Page[frame] --;

		pUserPageTable->m_Entrys[index].m_Present = 0;				// 清空PTE
		pUserPageTable->m_Entrys[index].m_ReadWriter = 0;
		pUserPageTable->m_Entrys[index].m_UserSupervisor = 1;
		pUserPageTable->m_Entrys[index].m_PageBaseAddress = 0;

		index++; 		count--;
	}

	/* 从PE头中获取新应用程序，正文段、数据段、堆栈段的起始地址、长度 */
	u.u_MemoryDescriptor.m_TextStartAddress = parser.TextAddress;
	u.u_MemoryDescriptor.m_TextSize = parser.TextSize;
	u.u_MemoryDescriptor.m_DataStartAddress = parser.DataAddress;
	u.u_MemoryDescriptor.m_DataSize = parser.DataSize;
	u.u_MemoryDescriptor.m_StackSize = parser.StackSize;

	pText = NULL;
	/* 分配一个空闲Text结构，或者和其它进程共享同一正文段 */
	for (int i = 0; i < ProcessManager::NTEXT; i++)
	{
		if (NULL == this->text[i].x_iptr) /* 记下找到的第一个空闲text结构 */
		{
			if (NULL == pText)
			{
				pText = &(this->text[i]);
			}
		}
		else if (pInode == this->text[i].x_iptr) /* 如果，这不是一个空闲text结构，看一下text结构指向的可执行文件是exec系统调用要执行的应用程序吗？ */
		{
			this->text[i].x_count++;
			this->text[i].x_ccount++;
			u.u_procp->p_textp = &(this->text[i]);
			pText = NULL; /* 与其它进程共享同一正文段，则pText重新清零，否则指向一空闲Text结构 */
			break;
		}
	}

	int sharedText = 0;

	PageTableEntry *entrys = (PageTableEntry *)md.m_UserPageTableArray;   // 1# 用户页表，hardcode Unix V6++
	pages = (u.u_MemoryDescriptor.m_TextSize + 4096 - 1) / 4096;    // 正文段页数
	int text_page_start = parser.TextAddress >> 12;  // 代码段第1页的逻辑页号
	text_page_start -= PageTable::ENTRY_CNT_PER_PAGETABLE;  // 在1# 用户页表中的下标，hardcode Unix V6++


	/* 无共享Text结构，为正文段分配text结构、物理页框、建立地址映射关系 */
	if (NULL != pText)
	{
		pInode->i_count++;

		pText->x_ccount = 1;
		pText->x_count = 1;
		pText->x_iptr = pInode;
		pText->x_size = u.u_MemoryDescriptor.m_TextSize;

		unsigned long temp;
		for (int i = 0; i < pages; i++)
		{
			temp = userPgMgr.AllocMemory(M_PAGE_SIZE);  // 没有考虑内存不足的情况
			pText->x_caddr[i] = (temp) >> 12;  // 分配给正文段的物理页框号

			entrys[text_page_start + i].m_UserSupervisor = 0x1;
			entrys[text_page_start + i].m_Present = 0x1;
			entrys[text_page_start + i].m_ReadWriter = false;
			entrys[text_page_start + i].m_PageBaseAddress = pText->x_caddr[i];
		}

		pText->x_daddr = Kernel::Instance().GetSwapperManager().AllocSwap(pText->x_size);   // 分配盘交换区空间

		u.u_procp->p_textp = pText;  // 建立Process结构和Text结构的勾连关系
	}
	else
	{
		pText = u.u_procp->p_textp;
		for (int i = 0; i < pages; i++)  // 写PTE
		{
			entrys[text_page_start + i].m_UserSupervisor = 0x1;
			entrys[text_page_start + i].m_Present = 0x1;
			entrys[text_page_start + i].m_ReadWriter = false;
			entrys[text_page_start + i].m_PageBaseAddress = pText->x_caddr[i];
		}
		sharedText = 1;
	}

	// 进程图像需要的物理空间尺寸：          PPDA + 数据 + 堆栈。没有考虑内存不足的情况。
	unsigned int newSize = ProcessManager::USIZE + u.u_MemoryDescriptor.m_DataSize + u.u_MemoryDescriptor.m_StackSize;
	u.u_procp->p_size = newSize;

	// 数据段
	pages = (u.u_MemoryDescriptor.m_DataSize + M_PAGE_SIZE - 1) / M_PAGE_SIZE;  // 数据段页数
	int data_page_start = (parser.DataAddress - md.USER_SPACE_START_ADDRESS) >> 12;
	data_page_start -= PageTable::ENTRY_CNT_PER_PAGETABLE;

	unsigned long temp;
	for (int i = 0; i < pages; i++)
	{
		temp = userPgMgr.AllocMemory(M_PAGE_SIZE);
		entrys[data_page_start + i].m_UserSupervisor = 0x1;
		entrys[data_page_start + i].m_Present = 0x1;
		entrys[data_page_start + i].m_ReadWriter = true;
		entrys[data_page_start + i].m_PageBaseAddress = temp >> 12;
	}

	// 用户栈
	pages = (u.u_MemoryDescriptor.m_StackSize + PageManager::PAGE_SIZE - 1) >> 12;
	unsigned long start = md.USER_SPACE_SIZE - allocLength;
	int stack_page = (start >> 12) - PageTable::ENTRY_CNT_PER_PAGETABLE;

	for (int i = 0; i < pages; i++,trueStack++)  // 写PTE
	{
		entrys[stack_page + i].m_UserSupervisor = 0x1;
		entrys[stack_page + i].m_Present = 0x1;
		entrys[stack_page + i].m_ReadWriter = true;
		entrys[stack_page + i].m_PageBaseAddress = trueStack;
	}

	md.MapToPageTable();

	/* 从exe文件中依次读入.text段、.data段、.rdata段、.bss段 */
	parser.Relocate(pInode, sharedText);

	/* .text段在swap分区上留复本 */
	if (sharedText == 0)
	{
		u.u_procp->p_flag |= Process::SLOCK;
		bufMgr.Swap(pText->x_daddr, pText->x_caddr[0], pText->x_size, Buf::B_WRITE);  // 错的，要改
		u.u_procp->p_flag &= ~Process::SLOCK;
	}

	/* 释放Inode，减少ExeCnt计数值 */
	fileMgr.m_InodeTable->IPut(pInode);
	if (this->ExeCnt >= NEXEC)
	{
		WakeUpAll((unsigned long)&ExeCnt);
	}
	this->ExeCnt--;

	/* 用默认的方式处理信号  */
	for (int i = 0; i < u.NSIG; i++)
	{
		u.u_signal[i] = 0;
	}

	/* 清0所有通用寄存器  */
	for (int i = User::EAX - 4; i < User::EAX - 4 * 7; i = i - 4)
	{
		u.u_ar0[i] = 0; /* 下标写成  User::EAX + i 可读性要强一些，但是运算速度慢了。就小抠，追求速度吧 */
	}

	/* 将exe程序的入口地址放入核心栈现场保护区中的EAX作为系统调用返回值，这个是runtime要用  */
	u.u_ar0[User::EAX] = parser.EntryPointAddress;

	/* 构造出Exec()系统调用的退出环境，使之退出到ring3时，开始执行user code */
	struct pt_context *pContext = (struct pt_context *)u.u_arg[4];
	pContext->eip = 0x00000000; /* 退出到ring3特权级下从线性地址0x00000000处runtime()开始执行 */
	// pContext->eip = parser.EntryPointAddress;
	pContext->xcs = Machine::USER_CODE_SEGMENT_SELECTOR;
	pContext->eflags = 0x200; /* 此项是否篡改无关紧要 */
	pContext->esp = esp;
	pContext->xss = Machine::USER_DATA_SEGMENT_SELECTOR;
}

Process *ProcessManager::Select()
{
	/* 前一次选中上台进程 */
	static int lastSelect = 0;

	while (true)
	{
		int priority = 256;
		int best = -1; /* 本轮搜索找到的最合适上台进程 */

		this->RunRun = 0;

		/* 搜索优先级最高的可运行进程 */
		for (int count = 0; count < NPROC; count++)
		{
			/* 从上一次被选中进程的下一个开始回环扫描，而不是每次从0#进程开始，保证各进程机会均等 */
			int i = (lastSelect + 1 + count) % NPROC;
			if (Process::SRUN == process[i].p_stat && (process[i].p_flag & Process::SLOAD) != 0)
			{
				if (process[i].p_pri < priority)
				{
					best = i;
					priority = process[i].p_pri;
				}
			}
		}
		if (-1 == best)
		{
			__asm__ __volatile__("hlt");
			continue;
		}

		SwtchNum++;
		if (SwtchNum & 0x80000000)
		{
			SwtchNum = 0; /* 计数溢出变为负数后，重置为零 */
		}
		/* 如果选出优先级最高的可运行进程 */
		this->CurPri = priority;
		lastSelect = best;
		// Diagnose::Write("Process %d is running!",best);
		return &process[best];
	}
}

void ProcessManager::Kill()
{
	User &u = Kernel::Instance().GetUser();
	int pid = u.u_arg[0];
	int signal = u.u_arg[1];
	bool flag = false;

	for (int i = 0; i < ProcessManager::NPROC; i++)
	{
		/* 不允许发送信号给进程自身 */
		if (u.u_procp == &process[i])
		{
			continue;
		}
		/* 不是信号的接收方目标进程，继续搜寻 */
		if (pid != 0 && process[i].p_pid != pid)
		{
			continue;
		}
		/* pid为0，则将信号发送至与发送进程同一终端的所有进程，0#进程不包括在内 */
		if (pid == 0 && (process[i].p_ttyp != u.u_procp->p_ttyp || i == 0))
		{
			continue;
		}
		/* 除非是超级用户，否则要求发送、接收进程u.uid相同，即不可给其它用户进程发送信号 */
		if (u.u_uid != 0 && u.u_uid != process[i].p_uid)
		{
			continue;
		}
		flag = true;
		/* 信号发送给满足条件的目标进程 */
		process[i].PSignal(signal);
	}
	if (false == flag)
	{
		u.u_error = User::ESRCH;
	}
}

void ProcessManager::WakeUpAll(unsigned long chan)
{
	/* 唤醒系统中所有因chan而进入睡眠的进程 */
	for (int i = 0; i < ProcessManager::NPROC; i++)
	{
		if (this->process[i].IsSleepOn(chan))
		{
			this->process[i].SetRun();
		}
	}
}

void ProcessManager::XSwap(Process *pProcess, bool bFreeMemory, int size)
{
	if (0 == size)
	{
		size = pProcess->p_size;
	}

	/* blkno记录分配到的交换区起始扇区号 */
	int blkno = Kernel::Instance().GetSwapperManager().AllocSwap(pProcess->p_size);
	if (0 == blkno)
	{
		Utility::Panic("Out of Swapper Space");
	}
	/* 递减进程图像在内存中，且引用该正文段的进程数 */
	if (pProcess->p_textp != NULL)
	{
		pProcess->p_textp->XccDec();
	}
	/* 上锁，防止同一进程图像被重复换出 */
	pProcess->p_flag |= Process::SLOCK;
	if (false == Kernel::Instance().GetBufferManager().Swap(blkno, pProcess->p_addr, size, Buf::B_WRITE))
	{
		Utility::Panic("Swap I/O Error");
	}
	if (bFreeMemory)
	{
		// ---- NOTE:3 ---- //
		// 目前没有做修改，但可能需要修改
		Kernel::Instance().GetUserPageManager().FreeMemory(size, pProcess->p_addr);
	}
	/* 把进程图像在交换区起始扇区号记录在p_addr中，SLOAD是0、进程是盘交换区上的进程了 */
	pProcess->p_addr = blkno;
	pProcess->p_flag &= ~(Process::SLOAD | Process::SLOCK);
	/* 最近一次被换入或换出以来，在内出或交换区驻留的时间长度清零 */
	pProcess->p_time = 0;

	if (this->RunOut)
	{
		this->RunOut = 0;
		Kernel::Instance().GetProcessManager().WakeUpAll((unsigned long)&RunOut);
	}
}

void ProcessManager::Signal(TTy *pTTy, int signal)
{
	for (int i = 0; i < ProcessManager::NPROC; i++)
	{
		if (this->process[i].p_ttyp == pTTy)
		{
			this->process[i].PSignal(signal);
		}
	}
}
