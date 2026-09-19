#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include "Process.h"
#include "Assembly.h"

/* 
 * 保存esp与ebp到u结构的宏，由于需要保存的是NewProc()和Swtch()函数的
 * 环境，因此只能使用宏。否则，返回地址变成了SaveU()时的地址。这样就
 * 跳回到SaveU()函数的下一句执行。该结果是不正确的
 */
 /*
#define SaveU(u) \
	__asm__ __volatile__(	\
		"movl %%esp, %0\n\t \
		 movl %%ebp, %1\n\t" \
		:"+m"((u).u_rsav[0]),"+m"((u).u_rsav[1]) \
		); 
*/

#define SaveU(u_sav) \
	__asm__ __volatile__(	\
		"movl %%esp, %0\n\t \
		 movl %%ebp, %1\n\t" \
		:"+m"(u_sav[0]),"+m"(u_sav[1]) \
		); 
		
/* 
 * 刷新内核页表第1023项的宏，在进程调度时，指向指定进程的u区地址，使
 * 得GetUser()函数返回当前进程的u结构
 */
#define SwtchUStruct(p) \
	Machine::Instance().GetKernelPageTable().m_Entrys[Kernel::USER_PAGE_INDEX].m_PageBaseAddress \
		= (p)->p_addr / PageManager::PAGE_SIZE; \
	FlushPageDirectory();

/* 
 * 恢复esp与ebp到u结构的宏，使用宏的理由同SaveU()
 */
/* #define RetU(u) \
	__asm__ __volatile__( \
		"movl %0, %%ebp\n\t \
		 movl %1, %%esp\n\t" \
		: \
		:"m"((u).u_rsav[1]),"m"((u).u_rsav[0]) \
		);	\
	FlushPageDirectory(); 
 */

#define RetU()	\
	__asm__ __volatile__("	movl %0, %%eax;				\
							movl (%%eax), %%esp;		\
							movl 0x4(%%eax), %%ebp;"	\
							:							\
							: "i" (0xc03ff000) );

#define aRetU(u_sav) \
	__asm__ __volatile__("	movl %0, %%esp;			\
							movl %1, %%ebp;"		\
							:						\
							:"m"(u_sav[0]), "m"(u_sav[1]) );
	
class ProcessManager
{
	/* static consts */
public:
	static const int NPROC = 100;
	static const unsigned long PROCESS_ZERO_PPDA_ADDRESS = 0x400000 - 0x1000;

	static const int NTEXT = 50;

	static const int NEXEC = 10;

	static const unsigned int USIZE = 0x1000;	/* ppda区大小，字节为单位 */

	/* 
	 * 进程进入睡眠状态时，内核根据其睡眠原因设置其醒来后的优先数；
	 * 优先数小于零为高优先权睡眠，优先数大于零为低优先权睡眠。
	 */
	static const int PSWP = -100;
	static const int PINOD = -90;
	static const int PRIBIO = -50;
	static const int EXPRI = -1;
	static const int PPIPE = 1;
	static const int TTIPRI = 10;
	static const int TTOPRI = 20;
	static const int PWAIT = 40;
	static const int PSLEP = 90;
	static const int PUSER = 100;

	/* Functions */
public:
	/* Constructors */
	ProcessManager();
	/* Destructors */
	~ProcessManager();

	void Initialize();

	/* 手工创建系统0#进程，后续进程都是通过fork()拷贝0#进程生成 */
	void SetupProcessZero();

	/*
	 * Swtch()的中间0#进程上台后，调用此函数选出
	 * 最适合上台运行的进程
	 */
	Process* Select();

	/*
	 * @comment 用于生成当前正在运行进程的拷贝，复印、进程返回值为0.
	 * 子进程在被调度后返回1,这是用于区分父子进程的唯一方法。即父进程
	 * 会继续运行,而子进程需要被调度后运行
	 *
	 */
	int NewProc();

	int Swtch();

	/* 
	 * 进程图像内存和交换区之间的传送。如果有进程想要换入内存，而内存
	 * 中无法找到能够容纳该进程的连续内存区，则依次将低优先权睡眠状态(SWAIT)-->
	 * 暂停状态(SSTOP)-->高优先权睡眠状态(SSLEEP)-->就绪状态(SRUN)进程换出，
	 * 直到腾出足够内存空间将想要换入的进程调入内存
	 */
	void Sched();

	/* 
	 * 父进程等待子进程结束的Wait()系统调用
	 */
	void Wait();

	/* 
	* 进程创建Fork()系统调用
	*/
	void Fork();

	/* 
	 * @comment Exec()系统调用，进程图像改换
	 */
	void Exec();

	/* 
	 * 终止进程Kill()系统调用
	 */
	void Kill();

	/*
	 * 唤醒系统中所有因chan而进入睡眠的进程
	 */
	void WakeUpAll(unsigned long chan);

	/*
	 * 将进程从内存换出至磁盘交换区上
	 * pProcess: 指向要换出的进程
	 * bFreeMemory: 是否释放进程图像占据的内存
	 * size: 除共享正文段外，进程可交换部分图像长度；参数size为0时，直接使用p_size
	 */
	void XSwap(Process* pProcess, bool bFreeMemory, int size);

	/*
	 * 将信号signal发送至与发送进程所在同一终端的所有进程
	 */
	void Signal(TTy* pTTy, int signal);

	/* Members */
public:
	Process process[NPROC];
	Text text[NTEXT];

	int CurPri;		/* 现运行占用CPU时优先数 */
	int RunRun;		/* 强迫调度标志 */
	int RunIn;		/* 内存中无合适进程可以调出至盘交换区 */
	int RunOut;		/* 盘交换区中无进程可以调入内存 */
	int ExeCnt;		/* 同时进行图像改换的进程数 */
	int SwtchNum;	/* 系统中进程切换次数 */

private:
	static unsigned int m_NextUniquePid;
public:
	static unsigned int NextUniquePid();

};

#endif

