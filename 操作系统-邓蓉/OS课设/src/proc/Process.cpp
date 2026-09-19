#include "Process.h"
#include "ProcessManager.h"
#include "Kernel.h"
#include "Utility.h"
#include "Machine.h"
#include "Video.h"


Process::Process()
{
	/* 标识所有p_stat为SNULL，标识该进程项可以使用 */
	this->p_stat = SNULL;
	/* 避免0#进程在Wait()时，许多空闲process项以0#进程为父进程 */
	this->p_ppid = -1;
}

Process::~Process()
{
}


void Process::SetRun()
{
	ProcessManager& procMgr = Kernel::Instance().GetProcessManager();

	/* 清除睡眠原因，转为就绪状态 */
	this->p_wchan = 0;
	this->p_stat = Process::SRUN;
	if ( this->p_pri < procMgr.CurPri )
	{
		procMgr.RunRun++;
	}
	if ( 0 != procMgr.RunOut && (this->p_flag & Process::SLOAD) == 0 )
	{
		procMgr.RunOut = 0;
		procMgr.WakeUpAll((unsigned long)&procMgr.RunOut);
	}
}

void Process::SetPri()
{
	int priority;
	ProcessManager& procMgr = Kernel::Instance().GetProcessManager();

	priority = this->p_cpu / 16;
	priority += ProcessManager::PUSER + this->p_nice;

	if ( priority > 255 )
	{
		priority = 255;
	}
	if ( priority > procMgr.CurPri )
	{
		procMgr.RunRun++;
	}
	this->p_pri = priority;
}

bool Process::IsSleepOn(unsigned long chan)
{
	/* 检查当前进程睡眠原因是否为chan */
	if( this->p_wchan == chan 
		&& (this->p_stat == Process::SWAIT || this->p_stat == Process::SSLEEP) )
	{
		return true;
	}
	return false;
}

void Process::Sleep(unsigned long chan, int pri)
{
	User& u = Kernel::Instance().GetUser();
	ProcessManager& procMgr = Kernel::Instance().GetProcessManager();

	if ( pri > 0 )
	{
		/* 
		 * 进程在进入低优先权睡眠之前，以及被唤醒之后，如果接收到不可忽略
		 * 的信号，则停止执行Sleep()，通过aRetU()直接跳转回Trap1()函数
		 */
		if ( this->IsSig() )
		{
			/* return确保aRetU()跳回到SystemCall::Trap1()之后立刻执行ret返回指令 */
			aRetU(u.u_qsav);
			return;
		}
		/* 
		* 此处关中断进入临界区，保证进程在设置睡眠原因chan和
		* 改进程状态为SSLEEP之间不会发生切换。
		*/
		X86Assembly::CLI();
		this->p_wchan = chan;
		/* 根据睡眠优先级pri确定进程进入高、低优先权睡眠 */
		this->p_stat = Process::SWAIT;
		this->p_pri = pri;
		X86Assembly::STI();

		if ( procMgr.RunIn != 0 )
		{
			procMgr.RunIn = 0;
			procMgr.WakeUpAll((unsigned long)&procMgr.RunIn);
		}
		/* 当前进程放弃CPU，切换其它进程上台 */
		//Diagnose::Write("Process %d Start Sleep!\n", this->p_pid);
		Kernel::Instance().GetProcessManager().Swtch();
		//Diagnose::Write("Process %d End Sleep!\n", this->p_pid);
		/* 被唤醒之后再次检查信号 */
		if ( this->IsSig() )
		{
			/* return确保aRetU()跳回到SystemCall::Trap1()之后立刻执行ret返回指令 */
			aRetU(u.u_qsav);
			return;
		}
	}
	else
	{
		X86Assembly::CLI();
		this->p_wchan = chan;
		/* 根据睡眠优先级pri确定进程进入高、低优先权睡眠 */
		this->p_stat = Process::SSLEEP;
		this->p_pri = pri;
		X86Assembly::STI();

		/* 当前进程放弃CPU，切换其它进程上台 */
		//Diagnose::Write("Process %d Start Sleep!\n", this->p_pid);
		Kernel::Instance().GetProcessManager().Swtch();
		//Diagnose::Write("Process %d End Sleep!\n", this->p_pid);
	}
}

void Process::Expand(unsigned int newSize)
{
	UserPageManager& userPgMgr = Kernel::Instance().GetUserPageManager();
	ProcessManager& procMgr = Kernel::Instance().GetProcessManager();
	User& u = Kernel::Instance().GetUser();
	Process* pProcess = u.u_procp;

	unsigned int oldSize = pProcess->p_size;
	p_size = newSize;
	unsigned long oldAddress = pProcess->p_addr;
	unsigned long newAddress;

	/* 如果进程图像缩小，则释放多余的内存 */
	if ( oldSize >= newSize )
	{
		if(oldSize > newSize)
			userPgMgr.FreeMemory(oldSize - newSize, oldAddress + newSize);
		return;
	}

	/* 进程图像扩大，需要寻找一块大小newSize的连续内存区 */
	SaveU(u.u_rsav);
	newAddress = userPgMgr.AllocMemory(newSize);
	/* 分配内存失败，将进程暂时换出到交换区上 */
	if ( NULL == newAddress )
	{
		SaveU(u.u_ssav);
		procMgr.XSwap(pProcess, true, oldSize);
		pProcess->p_flag |= Process::SSWAP;
		procMgr.Swtch();
		/* no return */
	}
	/* 分配内存成功，将进程图像拷贝到新内存区，然后跳转到新内存区继续运行 */
	pProcess->p_addr = newAddress;
	for ( unsigned int i = 0; i < oldSize; i++ )
	{
		Utility::CopySeg(oldAddress + i, newAddress + i);
	}

	/* 释放原来占用的内存区 */
	userPgMgr.FreeMemory(oldSize, oldAddress);
	
	X86Assembly::CLI();
	SwtchUStruct(pProcess);
	RetU();
	X86Assembly::STI();

	u.u_MemoryDescriptor.MapToPageTable();
}

void Process::Exit()
{
	int i;
	User& u = Kernel::Instance().GetUser();
	ProcessManager& procMgr = Kernel::Instance().GetProcessManager();
	OpenFileTable& fileTable = *Kernel::Instance().GetFileManager().m_OpenFileTable;
	InodeTable& inodeTable = *Kernel::Instance().GetFileManager().m_InodeTable;

	Diagnose::Write("Process %d is exiting\n",u.u_procp->p_pid);
	/* Reset Tracing flag */
	u.u_procp->p_flag &= (~Process::STRC);

	/* 清除进程的信号处理函数，设置为1表示不对该信号作任何处理 */
	for ( i = 0; i < User::NSIG; i++ )
	{
		u.u_signal[i] = 1;
	}

	/* 关闭进程打开文件 */
	for ( i = 0; i < OpenFiles::NOFILES; i++ )
	{
		File* pFile = NULL;
		if ( (pFile = u.u_ofiles.GetF(i)) != NULL )
		{
			fileTable.CloseF(pFile);
			u.u_ofiles.SetF(i, NULL);
		}
	}
	/*  访问不存在的fd会产生error code，清除u.u_error避免影响后续程序执行流程 */
	u.u_error = User::NOERROR;

	/* 递减当前目录的引用计数 */
	inodeTable.IPut(u.u_cdir);

	/* 释放该进程对共享正文段的引用 */
	if ( u.u_procp->p_textp != NULL )
	{
		u.u_procp->p_textp->XFree();
		u.u_procp->p_textp = NULL;
	}

	/* 将u区写入交换区，等待父进程做善后处理 */
	SwapperManager& swapperMgr = Kernel::Instance().GetSwapperManager();
	BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
	/* u区的大小不会超过512字节，所以只写入ppda区的前512字节，已囊括u结构的全部信息 */
	int blkno = swapperMgr.AllocSwap(BufferManager::BUFFER_SIZE);
	if ( NULL == blkno )
	{
		Utility::Panic("Out of Swapper Space");
	}
	Buf* pBuf = bufMgr.GetBlk(DeviceManager::ROOTDEV, blkno);
	Utility::DWordCopy((int *)&u, (int *)pBuf->b_addr, BufferManager::BUFFER_SIZE / sizeof(int));
	bufMgr.Bwrite(pBuf);

	/* 释放内存资源 */
	u.u_MemoryDescriptor.Release();
	Process* current = u.u_procp;
	UserPageManager& userPageMgr = Kernel::Instance().GetUserPageManager();
	userPageMgr.FreeMemory(current->p_size, current->p_addr);
	current->p_addr = blkno;
	current->p_stat = Process::SZOMB;

	/* 唤醒父进程进行善后处理 */
	for ( i = 0; i < ProcessManager::NPROC; i++ )
	{
		if ( procMgr.process[i].p_pid == current->p_ppid )
		{
			procMgr.WakeUpAll((unsigned long)&procMgr.process[i]);
			break;
		}
	}
	/* 没找到父进程 */
	if ( ProcessManager::NPROC == i )
	{
		current->p_ppid = 1;
		procMgr.WakeUpAll((unsigned long)&procMgr.process[1]);
	}

	/* 将自己的子进程传给自己的父进程 */
	for ( i = 0; i < ProcessManager::NPROC; i++ )
	{
		if ( current->p_pid == procMgr.process[i].p_ppid )
		{
			Diagnose::Write("My:%d 's child %d passed to 1#process",current->p_pid,procMgr.process[i].p_pid);
			procMgr.process[i].p_ppid = 1;
			if ( procMgr.process[i].p_stat == Process::SSTOP )
			{
				procMgr.process[i].SetRun();
			}
		}
	}

	procMgr.Swtch();
}

void Process::Clone(Process& proc)
{
	User& u = Kernel::Instance().GetUser();

	/* 拷贝父进程Process结构中的大部分数据 */
	proc.p_size = this->p_size;
	proc.p_stat = Process::SRUN;
	proc.p_flag = Process::SLOAD;
	proc.p_uid = this->p_uid;
	proc.p_ttyp = this->p_ttyp;
	proc.p_nice = this->p_nice;
	proc.p_textp = this->p_textp;
	
	/* 建立父子关系 */
	proc.p_pid = ProcessManager::NextUniquePid();
	proc.p_ppid = this->p_pid;
	
	/* 初始化进程调度相关成员 */
	proc.p_pri = 0;		/* 确保child的优先数较小，与其它进程相比更有机会占用CPU */
	proc.p_time = 0;
	

	/* 打开文件控制块File结构引用计数+1 */
	for ( int i = 0; i < OpenFiles::NOFILES; i++ )
	{
		File* pFile;
		if ( (pFile = u.u_ofiles.GetF(i)) != NULL )
		{
			pFile->f_count++;
		}
	}
	/* 
	 * GetF()访问u.u_ofiles中的空闲项会产生出错码，
	 * 如不清除将导致进程创建(fork)系统调用失败。
	 */
	u.u_error = User::NOERROR;

	/* 增加对共享正文段的引用计数 */
	if ( proc.p_textp != 0 )
	{
		proc.p_textp->x_count++;
		proc.p_textp->x_ccount++;
	}

	/* 增加对当前工作目录的引用计数 */
	u.u_cdir->i_count++;
}

//用于堆栈溢出时，自动扩展堆栈
void Process::SStack()
{
	User& u = Kernel::Instance().GetUser();
	MemoryDescriptor& md = u.u_MemoryDescriptor;
	unsigned int change = 4096;
	//unsigned int change = 0;
	md.m_StackSize += change;
	unsigned int newSize = ProcessManager::USIZE + md.m_DataSize + md.m_StackSize;

	if ( false == u.u_MemoryDescriptor.EstablishUserPageTable(md.m_TextStartAddress,
						md.m_TextSize, md.m_DataStartAddress, md.m_DataSize, md.m_StackSize) )
	{
		u.u_error = User::ENOMEM;
		return;
	}

	this->Expand(newSize);
	int dst = u.u_procp->p_addr + newSize;
	unsigned int count = md.m_StackSize - change;
	while(count--)
	{
		dst--;
		Utility::CopySeg(dst - change, dst);
	}

	u.u_MemoryDescriptor.MapToPageTable();
}


void Process::SBreak()
{
	User& u = Kernel::Instance().GetUser();
	unsigned int newEnd = u.u_arg[0];
	MemoryDescriptor& md = u.u_MemoryDescriptor;
	unsigned int newSize = newEnd - md.m_DataStartAddress;

	if (newEnd == 0)
	{
		u.u_ar0[User::EAX] = md.m_DataStartAddress + md.m_DataSize;
		return;
	}

	if ( false == u.u_MemoryDescriptor.EstablishUserPageTable(md.m_TextStartAddress, 
						md.m_TextSize, md.m_DataStartAddress, newSize, md.m_StackSize) )
	{
		//系统调用出错时，不可以用这种方式返回。执行这条路径会导致 u.u_intflg == 1，u.u_error被错误修改为EINTR（4）；无论何故导致系统调用失败。
		//aRetU(u.u_qsav);
		return;
	}

	int change = newSize - md.m_DataSize;
	md.m_DataSize = newSize;
	newSize += ProcessManager::USIZE + md.m_StackSize;

	/* 数据段缩小 */
	if ( change < 0 )
	{
		int dst = u.u_procp->p_addr + newSize - md.m_StackSize;
		int count = md.m_StackSize;
		while(count--)
		{
			Utility::CopySeg(dst - change, dst);
			dst++;
		}
		this->Expand(newSize);
	}
	/* 数据段增大 */
	else if ( change > 0 )
	{
		this->Expand(newSize);
		int dst = u.u_procp->p_addr + newSize;
		int count = md.m_StackSize;
		while(count--)
		{
			dst--;
			Utility::CopySeg(dst - change, dst);
		}
	}
	u.u_ar0[User::EAX] = md.m_DataStartAddress + md.m_DataSize;
}

void Process::PSignal( int signal )
{
	if ( signal >= User::NSIG )
	{
		return;
	}

	/* 如果已经接收到SIGKILL信号，则忽略后续信号 */
	if ( this->p_sig != User::SIGKILL )
	{
		this->p_sig = signal;
	}
	/* 若进程的优先数大于PUSER(100)，则将其设置为PUSER */
	if ( this->p_pri > ProcessManager::PUSER )
	{
		this->p_pri	= ProcessManager::PUSER;
	}
	/* 若进程的处于低优先权睡眠，则将其唤醒 */
	if ( this->p_stat == Process::SWAIT )
	{
		this->SetRun();
	}
}

int Process::IsSig()
{
	User& u = Kernel::Instance().GetUser();

	/* 未接受到信号 */
	if ( this->p_sig == 0 )
	{
		return 0;
	}
	/* u.u_signal[n]为偶数才表示对信号进程处理 */
	else if ( (u.u_signal[this->p_sig] & 1) == 0 )
	{
		return this->p_sig;
	}
	return 0;
}

/*
extern "C" void runtime();
extern "C" void SignalHandler();
*/

void Process::PSig(struct pt_context* pContext)
{
	User& u = Kernel::Instance().GetUser();
	int signal = this->p_sig;
	/* 清除已进入处理流程的信号 */
	this->p_sig = 0;

	if ( u.u_signal[signal] != 0 )
	{
		/* 清除进程在收到信号之前执行系统调用期间可能产生的ErrCode */
		u.u_error = User::NOERROR;

		unsigned int old_eip = pContext->eip;

		/* 核心态返回值为预定的用户函数SignalHandler()的首地址 */
		/*pContext->eip = ((unsigned long)SignalHandler - (unsigned long)runtime);
		pContext->esp -= 8;
		int* pInt = (int *)pContext->esp;
		*pInt = u.u_signal[signal];
		*(pInt + 1) = old_eip;*/
		pContext->eip = u.u_signal[signal];
		pContext->esp -= 4;
		int* pInt = (int *)pContext->esp;
		*pInt = old_eip;

		/* 
		 * 当前信号处理函数在响应完本次信号之后，需要重置为默认
		 * 的信号处理函数，设为0表示对信号的处理方式为终止本进程。
		 */
		u.u_signal[signal] = 0;
		return;
	}

	/* u.u_signal[n]为0，则对信号的处理方式是终止本进程 */
	u.u_procp->Exit();
}

void Process::Nice()
{
	User& u = Kernel::Instance().GetUser();
	int niceValue = u.u_arg[0];

	if (niceValue > 20)
	{
		niceValue = 20;
	}
	if (niceValue < 0 && !u.SUser())
	{
		/* 非系统超级用户不能为进程设置小于0的进程优先数计算偏置值 */
		niceValue = 0;
	}
	this->p_nice = niceValue;
}

void Process::Ssig()
{
	User& u = Kernel::Instance().GetUser();

	int signalIndex = u.u_arg[0];
	unsigned long func = u.u_arg[1];

	/* 这几个信号不许设置 */
	if ( signalIndex <= 0 || signalIndex >= User::NSIG || signalIndex == User::SIGKILL )
	{
		u.u_error = User::EINVAL;
		return;
	}
	/* 设置函数地址到信号处理函数数组 */
	u.u_ar0[User::EAX] = u.u_signal[signalIndex];
	u.u_signal[signalIndex] = func;
	/* 清当前信号 */
	if ( u.u_procp->p_sig == signalIndex )
	{
		u.u_procp->p_sig = 0;
	}
}





