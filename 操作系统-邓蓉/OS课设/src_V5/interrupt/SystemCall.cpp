#include "SystemCall.h"
#include "User.h"
#include "Kernel.h"
#include "Regs.h"
#include "TimeInterrupt.h"
#include "CRT.h"
#include "Video.h"

/* 系统调用入口表的定义
 * 参照UNIX V6中sysent.c中对系统调用入口表sysent的定义 @line 2910
 */
SystemCallTableEntry SystemCall::m_SystemEntranceTable[SYSTEM_CALL_NUM] =
	{
		{0, &Sys_NullSystemCall}, /* 0 = indir	*/
		{1, &Sys_Rexit},		  /* 1 = rexit	*/
		{0, &Sys_Fork},			  /* 2 = fork	*/
		{3, &Sys_Read},			  /* 3 = read	*/
		{3, &Sys_Write},		  /* 4 = write	*/
		{2, &Sys_Open},			  /* 5 = open	*/
		{1, &Sys_Close},		  /* 6 = close	*/
		{1, &Sys_Wait},			  /* 7 = wait	*/
		{2, &Sys_Creat},		  /* 8 = creat	*/
		{2, &Sys_Link},			  /* 9 = link	*/
		{1, &Sys_UnLink},		  /* 10 = unlink	*/
		{3, &Sys_Exec},			  /* 11 = Exec 	*/
		{1, &Sys_ChDir},		  /* 12 = chdir	*/
		{0, &Sys_GTime},		  /* 13 = time 	*/
		{3, &Sys_MkNod},		  /* 14 = mknod	*/
		{2, &Sys_ChMod},		  /* 15 = chmod	*/
		{3, &Sys_ChOwn},		  /* 16 = chown	*/
		{1, &Sys_SBreak},		  /* 17 = sbreak	*/
		{2, &Sys_Stat},			  /* 18 = stat 		*/
		{3, &Sys_Seek},			  /* 19 = seek	*/
		{0, &Sys_Getpid},		  /* 20 = getpid	*/
		{3, &Sys_Smount},		  /* 21 = mount	*/
		{1, &Sys_Sumount},		  /* 22 = umount	*/
		{1, &Sys_Setuid},		  /* 23 = setuid	*/
		{0, &Sys_Getuid},		  /* 24 = getuid	*/
		{1, &Sys_Stime},		  /* 25 = stime	*/
		{3, &Sys_Ptrace},		  /* 26 = ptrace	*/
		{0, &Sys_Nosys},		  /* 27 = nosys	*/
		{2, &Sys_FStat},		  /* 28 = fstat	*/
		{1, &Sys_Trace},		  /* 29 = trace	*/
		{0, &Sys_NullSystemCall}, /* 30 = smdate; inoperative */
		{2, &Sys_Stty},			  /* 31 = stty	*/
		{2, &Sys_Gtty},			  /* 32 = gtty	*/
		{0, &Sys_Nosys},		  /* 33 = nosys	*/
		{1, &Sys_Nice},			  /* 34 = nice	*/
		{1, &Sys_Sslep},		  /* 35 = sleep	*/
		{0, &Sys_Sync},			  /* 36 = sync	*/
		{2, &Sys_Kill},			  /* 37 = kill		*/
		{0, &Sys_Getswit},		  /* 38 = switch	*/
		{1, &Sys_Pwd},			  /* 39 = pwd	*/
		{0, &Sys_Nosys},		  /* 40 = nosys	*/
		{1, &Sys_Dup},			  /* 41 = dup		*/
		{1, &Sys_Pipe},			  /* 42 = pipe 	*/
		{1, &Sys_Times},		  /* 43 = times	*/
		{4, &Sys_Profil},		  /* 44 = prof	*/
		{0, &Sys_Nosys},		  /* 45 = nosys	*/
		{1, &Sys_Setgid},		  /* 46 = setgid	*/
		{0, &Sys_Getgid},		  /* 47 = getgid	*/
		{2, &Sys_Ssig},			  /* 48 = sig	*/
		{0, &Sys_Nosys},		  /* 49 = nosys	*/
		{0, &Sys_Nosys},		  /* 50 = nosys	*/
		{0, &Sys_Nosys},		  /* 51 = nosys	*/
		{0, &Sys_Nosys},		  /* 52 = nosys	*/
		{0, &Sys_Nosys},		  /* 53 = nosys	*/
		{0, &Sys_Nosys},		  /* 54 = nosys	*/
		{0, &Sys_Nosys},		  /* 55 = nosys	*/
		{0, &Sys_Nosys},		  /* 56 = nosys	*/
		{0, &Sys_Nosys},		  /* 57= nosys	*/
		{0, &Sys_Nosys},		  /* 58 = nosys	*/
		{0, &Sys_Nosys},		  /* 59 = nosys	*/
		{0, &Sys_Nosys},		  /* 60 = nosys	*/
		{0, &Sys_Nosys},		  /* 61 = nosys	*/
		{0, &Sys_Nosys},		  /* 62 = nosys	*/
		{0, &Sys_Nosys},		  /* 63 = nosys	*/
};

SystemCall::SystemCall()
{
	// nothing to do here
}

SystemCall::~SystemCall()
{
	// nothing to do here
}

void SystemCall::SystemCallEntrance()
{
	SaveContext();

	SwitchToKernel();

	CallHandler(SystemCall, Trap);

	/* 获取由中断隐指令(即硬件实施)压入核心栈的pt_context。
	 * 这样就可以访问context.xcs中的OLD_CPL，判断先前态
	 * 是用户态还是核心态。
	 */
	struct pt_context *context;
	__asm__ __volatile__("	movl %%ebp, %0; addl $0x4, %0 " : "+m"(context));

	/* 这部分代码用高级语言实现恢复现场和中断返回 @line 0785 */
	/* V6中本身的逻辑是：
	 * if(中断前==用户态) {
	 * 		执行设备(陷入)处理子程序； 判断runrun， swtch()等等；	}
	 * else  {  //中断前==核心态
	 * 		改先前态为用户态； 执行设备(陷入)处理子程序；恢复现场，退出至前一层中断；}
	 *
	 * 由于体系结构的不同(x86的PSW不记录先前态)，x86上通过判断核心栈CS中OLD_CPL == 0x3；
	 * 判断先前是用户态或者核心态。
	 *
	 * 无论中断前==用户态or核心态，都一定执行处理子程序，那么就无需在调用Trap之前判断判断
	 * 判断中断前是用户态or核心态，因为两种情况下都要执行Trap()。
	 */
	if (context->xcs & USER_MODE) /*先前为用户态*/
	{
		while (true)
		{
			X86Assembly::CLI(); /* 处理机优先级升为7级 */

			if (Kernel::Instance().GetProcessManager().RunRun > 0)
			{
				X86Assembly::STI(); /* 处理机优先级降为0级 */
				Kernel::Instance().GetProcessManager().Swtch();
			}
			else
			{
				break; /* 如果runrun == 0，则退栈回到用户态继续用户程序的执行 */
			}
		}
	}
	RestoreContext(); // SysCallRestore();	/* 此后EAX中存放系统调用返回值，防止一切可能的修改 */

	Leave(); /* 手工销毁栈帧 */

	InterruptReturn(); /* 退出中断 */
}

void SystemCall::Trap(struct pt_regs *regs, struct pt_context *context)
{
	User &u = Kernel::Instance().GetUser();
	/* reference: u.u_ar0 = &r0 @line 2701 */

	/* 新加进的代码。判断有无接收到信号，如接收到信号则进行响应 */
	if (u.u_procp->IsSig())
	{
		u.u_procp->PSig(context);
		u.u_error = User::EINTR;
		regs->eax = -u.u_error;
		return;
	}

	u.u_ar0 = &regs->eax;

	if (regs->eax == 20)
		regs->eax = 20;

	/*
	 * 清空可能由于前一次系统调用失败而设置的错误码, u.u_error中如果有
	 * 出错码的话，即便后面的程序完全正确，内核也会进入错误路径 **!!!!**
	 */
	u.u_error = User::NOERROR;

	SystemCallTableEntry *callp = &m_SystemEntranceTable[regs->eax];

	// Diagnose::Write("eax = %d, callp: count = %d, address = %x\n", regs->eax, callp->count, callp->call);

	/* 根据callp->count将系统调用的传入参数从寄存器放入u.u_arg[5] */
	unsigned int *syscall_arg = (unsigned int *)&regs->ebx;
	for (unsigned int i = 0; i < callp->count; i++)
	{
		u.u_arg[i] = (int)(*syscall_arg++);
	}

	/* u.u_dirp一般用于指向系统调用的pathname参数 */
	u.u_dirp = (char *)u.u_arg[0];

	/*
	 * context指向核心栈上硬件保护现场部分，这样处理是因为Exec()系统调用
	 * 需要Fake一个退出环境，使之退出到ring3时，开始执行user code。目前所有
	 * 系统调用都是不会用到u.u_arg[4]的。
	 */
	u.u_arg[4] = (int)context;

	Trap1(callp->call); /* 系统调用处理子程序，如fork(), read()等等 */

	/*
	 * 如果系统调用期间受到信号打断，那么将不会执行Trap1()函数
	 * 中u.u_intflg = 0，而直接返回至Trap()函数当前位置
	 */
	if (u.u_intflg != 0)
	{
		u.u_error = User::EINTR;
	}

	/* 注: Unix V6++将系统调用出错结果返回给用户程序的方式和V6(通过PSW中的EBIT)有所区别!
	 * 如果系统调用期间出错，即u.u_error被设置，那么需要通过reg.eax返回-u.u_error，
	 * 从而和成功执行的系统调用返回>=0的值区别开来，继而出错的系统调用(即经由EAX寄存器
	 * 返回-u.u_error)将出错码存放在用户态全局变量errno中，对于用户程序统统返回-1表示出错。
	 */

	if (User::NOERROR != u.u_error)
	{
		regs->eax = -u.u_error;
		Diagnose::Write("regs->eax = %d , u.u_error = %d\n", regs->eax, u.u_error);
	}

	/* 判断有无接收到信号，如接收到信号则进行响应 */
	if (u.u_procp->IsSig())
	{
		u.u_procp->PSig(context);
	}

	/* Trap()末尾重算当前进程优先数 */
	u.u_procp->SetPri();
}

void SystemCall::Trap1(int (*func)())
{
	User &u = Kernel::Instance().GetUser();

	u.u_intflg = 1;
	/*	int pid = u.u_procp->p_pid;
		int text = u.u_MemoryDescriptor.m_TextSize;
		int data =  u.u_MemoryDescriptor.m_DataSize;*/
	SaveU(u.u_qsav);
	func();
	u.u_intflg = 0;
}

/*	27, 49 - 63 = nosys		count = 0	*/
int SystemCall::Sys_Nosys()
{
	/* 尚未分配的系统调用表项执行此空函数 */
	User &u = Kernel::Instance().GetUser();
	u.u_error = User::ENOSYS;

	return 0; /* GCC likes it ! */
}

/*	0 = indir	count = 0	*/
int SystemCall::Sys_NullSystemCall()
{
	/* This function should NEVER be called ! */

	return 0; /* GCC likes it ! */
}

/*	1 = rexit	count = 0	*/
int SystemCall::Sys_Rexit()
{
	User &u = Kernel::Instance().GetUser();
	u.u_procp->Exit();

	return 0; /* GCC likes it ! */
}

/*	2 = fork	count = 0	*/
int SystemCall::Sys_Fork()
{
	ProcessManager &procMgr = Kernel::Instance().GetProcessManager();
	procMgr.Fork();

	return 0; /* GCC likes it ! */
}

/*	3 = read	count = 2	*/
int SystemCall::Sys_Read()
{
	FileManager &fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.Read();

	return 0; /* GCC likes it ! */
}

/*	4 = write	count = 2	*/
int SystemCall::Sys_Write()
{
	FileManager &fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.Write();

	return 0; /* GCC likes it ! */
}

/*	5 = open	count = 2	*/
int SystemCall::Sys_Open()
{
	FileManager &fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.Open();

	return 0; /* GCC likes it ! */
}

/*	6 = close	count = 0	*/
int SystemCall::Sys_Close()
{
	FileManager &fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.Close();

	return 0; /* GCC likes it ! */
}

/*	7 = wait	count = 0	*/
int SystemCall::Sys_Wait()
{
	ProcessManager &procMgr = Kernel::Instance().GetProcessManager();
	procMgr.Wait();

	return 0; /* GCC likes it ! */
}

/*	8 = creat	count = 2	*/
int SystemCall::Sys_Creat()
{
	FileManager &fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.Creat();

	return 0; /* GCC likes it ! */
}

/*	9 = link	count = 2	*/
int SystemCall::Sys_Link()
{
	FileManager &fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.Link();

	return 0; /* GCC likes it ! */
}

/*	10 = unlink	count = 1	*/
int SystemCall::Sys_UnLink()
{
	FileManager &fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.UnLink();

	return 0; /* GCC likes it ! */
}

/*	11 = exec	count = 2	*/
int SystemCall::Sys_Exec()
{
	ProcessManager &procMgr = Kernel::Instance().GetProcessManager();
	procMgr.Exec();

	return 0; /* GCC likes it ! */
}

/*	12 = chdir	count = 1	*/
int SystemCall::Sys_ChDir()
{
	FileManager &fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.ChDir();

	return 0; /* GCC likes it ! */
}

/*	13 = gtime	count = 0	*/
int SystemCall::Sys_GTime()
{
	User &u = Kernel::Instance().GetUser();
	u.u_ar0[User::EAX] = Time::time;

	return 0; /* GCC likes it ! */
}

/*	14 = mknod	count = 3	*/
int SystemCall::Sys_MkNod()
{
	FileManager &fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.MkNod();

	return 0; /* GCC likes it ! */
}

/*	15 = chmod	count = 2	*/
int SystemCall::Sys_ChMod()
{
	FileManager &fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.ChMod();

	return 0; /* GCC likes it ! */
}

/*	16 = chown	count = 2	*/
int SystemCall::Sys_ChOwn()
{
	FileManager &fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.ChOwn();

	return 0; /* GCC likes it ! */
}

/*	17 = sbreak	count = 1	*/
int SystemCall::Sys_SBreak()
{
	User &u = Kernel::Instance().GetUser();
	u.u_procp->SBreak();

	return 0; /* GCC likes it ! */
}

/*	18 = stat	count = 2	*/
int SystemCall::Sys_Stat()
{
	FileManager &fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.Stat();

	return 0; /* GCC likes it ! */
}

/*	19 = seek	count = 2	*/
int SystemCall::Sys_Seek()
{
	FileManager &fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.Seek();

	return 0; /* GCC likes it ! */
}

/*	20 = getpid	count = 0	*/
int SystemCall::Sys_Getpid()
{
	User &u = Kernel::Instance().GetUser();
	u.u_ar0[User::EAX] = u.u_procp->p_pid;

	return 0; /* GCC likes it ! */
}

/*	21 = mount	count = 3	*/
int SystemCall::Sys_Smount()
{
	return 0; /* GCC likes it ! */
}

/*	22 = umount  count = 1	*/
int SystemCall::Sys_Sumount()
{
	return 0; /* GCC likes it ! */
}

/*	23 = setuid	count = 0	*/
int SystemCall::Sys_Setuid()
{
	User &u = Kernel::Instance().GetUser();
	u.Setuid();

	return 0; /* GCC likes it ! */
}

/*	24 = getuid	count = 0	*/
int SystemCall::Sys_Getuid()
{
	User &u = Kernel::Instance().GetUser();
	u.Getuid();

	return 0; /* GCC likes it ! */
}

/*	25 = stime	count = 0	*/
int SystemCall::Sys_Stime()
{
	User &u = Kernel::Instance().GetUser();

	/* 仅超级用户才具有设置系统时间的权限 */
	if (u.SUser())
	{
		Time::time = u.u_ar0[User::EAX];
	}

	return 0; /* GCC likes it ! */
}

/*	26 = ptrace	count = 3	*/
int SystemCall::Sys_Ptrace()
{
	return 0; /* GCC likes it ! */
}

/*	28 = fstat	count = 1	*/
int SystemCall::Sys_FStat()
{
	FileManager &fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.FStat();

	return 0; /* GCC likes it ! */
}

int SystemCall::Sys_Trace()
{
	User &u = Kernel::Instance().GetUser();

	if (Diagnose::ROWS == 0) /* if Diagnose not enabled */
	{
		Diagnose::ROWS = u.u_arg[0]; /* Diagnose类调试输出的总行数 */

		/* 定位当前输出坐标 */
		Diagnose::m_Row = Diagnose::SCREEN_ROWS - Diagnose::ROWS;
		Diagnose::m_Column = 0;

		CRT::ROWS = Diagnose::SCREEN_ROWS - Diagnose::ROWS;
	}
	else /* if enabled already */
	{
		Diagnose::ClearScreen();
		/* 停止Diagnose类调试输出 */
		Diagnose::ROWS = 0;
		/* 定位当前输出坐标 */
		Diagnose::m_Row = Diagnose::SCREEN_ROWS - Diagnose::ROWS;
		Diagnose::m_Column = 0;

		/* 字符设备输出使用整个屏幕所有行 */
		CRT::ROWS = Diagnose::SCREEN_ROWS;
	}
	u.u_ar0[User::EAX] = Diagnose::ROWS;

	return 0; /* GCC likes it ! */
}

/*	31 = stty	count = 1	*/
int SystemCall::Sys_Stty()
{
	File *pFile;
	Inode *pInode;
	User &u = Kernel::Instance().GetUser();
	int fd = u.u_arg[0];
	TTy *pTTy = (TTy *)u.u_arg[1];

	if ((pFile = u.u_ofiles.GetF(fd)) == NULL)
	{
		return 0;
	}
	pInode = pFile->f_inode;
	if ((pInode->i_mode & Inode::IFMT) != Inode::IFCHR)
	{
		u.u_error = User::ENOTTY;
		return 0;
	}
	short dev = pInode->i_addr[0];
	Kernel::Instance().GetDeviceManager().GetCharDevice(dev).SgTTy(dev, pTTy);

	return 0; /* GCC likes it ! */
}

/*	32 = gtty	count = 1	*/
int SystemCall::Sys_Gtty()
{
	File *pFile;
	Inode *pInode;
	User &u = Kernel::Instance().GetUser();
	int fd = u.u_arg[0];
	TTy *pTTy = (TTy *)u.u_arg[1];

	if ((pFile = u.u_ofiles.GetF(fd)) == NULL)
	{
		return 0;
	}
	pInode = pFile->f_inode;
	if ((pInode->i_mode & Inode::IFMT) != Inode::IFCHR)
	{
		u.u_error = User::ENOTTY;
		return 0;
	}
	short dev = pInode->i_addr[0];
	Kernel::Instance().GetDeviceManager().GetCharDevice(dev).SgTTy(dev, pTTy);

	return 0; /* GCC likes it ! */
}

/*	34 = nice	count = 0	*/
int SystemCall::Sys_Nice()
{
	User &u = Kernel::Instance().GetUser();
	u.u_procp->Nice();

	return 0; /* GCC likes it ! */
}

/*	35 = sleep	count = 0	*/
int SystemCall::Sys_Sslep()
{
	User &u = Kernel::Instance().GetUser();

	X86Assembly::CLI();

	unsigned int wakeTime = Time::time + u.u_arg[0]; /* sleep(second) */

	/*
	 * 对   if ( Time::tout <= Time::time || Time::tout > wakeTime )  中判断条件的解释：
	 * 1、系统先前设置的所有闹钟均已到期。  其后，第一个设置闹钟的进程看到的是条件 tout <= time成立，将自己的waketime写入tout变量。
	 * 2、系统中，存在闹钟未到期的进程。如果有进程设置闹钟，看到的是条件tout > time，进程比对tout变量和自己的waketime，令tout变量的值是所有进程waketime的最小值。
	 *
	 * 原先的注释：
	 * 此处不可以'wakeTime >= Time::time', 否则极端情况下前一次sleep(sec)刚结束，
	 * 紧接着第二次sleep(0)，会使wakeTime == Time::time == Time::tout，
	 * 而如果此时发生时钟中断恰为一秒末尾，Time::Clock()中Time::time++，
	 * 会导致Time::tout比Time::time小1，永远无法满足Time::time == Time::tout
	 * 的唤醒条件，调用sleep(0)的进程永远睡眠。         The end.
	 *
	 * 原先的注释不对。如果while循环的判断条件是'wakeTime >= Time::time'，执行sleep(0)的进程将把waketime和tout设为上个整数秒。整数秒时钟中断处理程序会time++，之后
	 * 1、如果不再有进程设置新闹钟，系统的闹钟服务就瘫痪了。这是因为， time==tout的条件永远无法满足，时钟中断处理程序不再会唤醒任何因设置了闹钟而入睡的进程。
	 * 2、如果有进程设置新闹钟newWaketime，执行sleep(0)操作的进程以及所有waketime<=newWaketime的进程的唤醒时刻将推迟到newWaketime。
	 *
	 * 现在的闹钟服务正确，执行sleep(0)的进程不会入睡更不会使tout值出现错误。
	 */
	while (wakeTime > Time::time)
	{
		if (Time::tout <= Time::time || Time::tout > wakeTime)
		{
			Time::tout = wakeTime;
		}
		u.u_procp->Sleep((unsigned long)&Time::tout, ProcessManager::PSLEP);
	}

	X86Assembly::STI();

	return 0; /* GCC likes it ! */
}

/*	36 = sync	count	= 0	*/
int SystemCall::Sys_Sync()
{
	Kernel::Instance().GetFileSystem().Update();

	return 0; /* GCC likes it ! */
}

/*	37 = kill	count = 1	*/
int SystemCall::Sys_Kill()
{
	ProcessManager &procMgr = Kernel::Instance().GetProcessManager();
	procMgr.Kill();

	return 0; /* GCC likes it ! */
}

/*	38 = switch	count = 0	*/
int SystemCall::Sys_Getswit()
{
	ProcessManager &procMgr = Kernel::Instance().GetProcessManager();
	User &u = Kernel::Instance().GetUser();

	u.u_ar0[User::EAX] = procMgr.SwtchNum;
	return 0; /* GCC likes it ! */
}

/*	39 = pwd	count = 1	*/
int SystemCall::Sys_Pwd()
{
	User &u = Kernel::Instance().GetUser();
	u.Pwd();

	return 0; /* GCC likes it ! */
}

/*	41 = dup	count = 0	*/
int SystemCall::Sys_Dup()
{
	FileManager &fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.Dup();

	return 0; /* GCC likes it ! */
}

/*	42 = pipe	count = 0	*/
int SystemCall::Sys_Pipe()
{
	FileManager &fileMgr = Kernel::Instance().GetFileManager();
	fileMgr.Pipe();

	return 0; /* GCC likes it ! */
}

/*	43 = times	count = 1	*/
int SystemCall::Sys_Times()
{
	User &u = Kernel::Instance().GetUser();

	struct tms *ptms = (struct tms *)u.u_arg[0];

	ptms->utime = u.u_utime;
	ptms->stime = u.u_stime;
	ptms->cutime = u.u_cutime;
	ptms->cstime = u.u_cstime;

	return 0; /* GCC likes it ! */
}

/*	44 = prof	count = 4	*/
int SystemCall::Sys_Profil()
{
	return 0; /* GCC likes it ! */
}

/*	46 = setgid	count = 0	*/
int SystemCall::Sys_Setgid()
{
	User &u = Kernel::Instance().GetUser();
	u.Setgid();

	return 0; /* GCC likes it ! */
}

/*	47 = getgid	count = 0	*/
int SystemCall::Sys_Getgid()
{
	User &u = Kernel::Instance().GetUser();
	u.Getgid();

	return 0; /* GCC likes it ! */
}

/*	48 = ssig	count = 2	*/
int SystemCall::Sys_Ssig()
{
	User &u = Kernel::Instance().GetUser();
	u.u_procp->Ssig();

	return 0; /* GCC likes it ! */
}
