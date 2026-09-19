#ifndef PROCESS_H
#define PROCESS_H

#include "Text.h"
#include "TTy.h"
#include "Regs.h"

/*
 * Process类与UNIX V6中进程控制块proc结构对应，这里只改变
 * 类名，不修改成员结构名字，以及对UNIX V6的proc结构中成员
 * 使用的数据类型进行适当更改，以符合现代程序的代码风格。
 */
class Process
{
public:
	enum ProcessState	/* 进程状态 */
	{
		SNULL	= 0,	/* 未初始化空状态 */
		SSLEEP	= 1,	/* 高优先权睡眠 */
		SWAIT	= 2,	/* 低优先权睡眠 */
		SRUN	= 3,	/* 运行、就绪状态 */
		SIDL	= 4,	/* 进程创建时的中间状态 */
		SZOMB	= 5,	/* 进程终止时的中间状态 */
		SSTOP	= 6		/* 进程正被跟踪 */
	};

	enum ProcessFlag	/* 进程标志位 */
	{
		SLOAD	= 0x1,	/* 进程图像在内存中 */
		SSYS	= 0x2,	/* 系统进程图像，不允许被换出 */
		SLOCK	= 0x4,	/* 含有该标志的进程图像暂不允许换出 */
		SSWAP	= 0x8,	/* 该进程被创建时图像就在交换区上 */
		STRC	= 0x10,	/* 父子进程跟踪标志，UNIX V6++未有效使用到 */
		STWED	= 0x20	/* 父子进程跟踪标志，UNIX V6++未有效使用到 */
	};
public:
	Process();
	~Process();
	void SetRun();								/* 唤醒当前进程，转入就绪状态 */
	
	void SetPri();								/* 根据占用CPU时间计算当前进程优先数 */
	
	bool IsSleepOn(unsigned long chan);			/* 检查当前进程睡眠原因是否为chan */
	
	void Sleep(unsigned long chan, int pri);	/* 使当前进程转入睡眠状态 */
	
	void Expand(unsigned int newSize);			/* 改变进程占用的内存大小 */
	
	void Exit();								/* Exit()系统调用处理过程 */
	
	void Clone(Process& proc);					/* 除p_pid之外，子进程拷贝父进程Process结构 */
	
	void SStack();                              /* 堆栈溢出时，自动扩展堆栈 */

	void SBreak();								/* brk()系统调用处理过程 */
	
	void PSignal(int signal);					/* 向当前进程发送信号。signal是要发送的信号数 */

	void PSig(struct pt_context* pContext);		/* 对当前进程接收到的信号进行处理 */

	void Nice();								/* 用户设置计算进程优先数的偏置值 */

	void Ssig();								/* 设置用户自定信号处理方式的系统调用处理函数 */

	int IsSig();								/* 判断当前进程是否接收到信号。
												 * 
												 * 进程在每次中断、异常和系统调用末尾退出核心态时，以及低优先权
												 * 睡眠首尾处总会检查p_sig。p_sig中接收到信号本身对进程并无影响，
												 * 仅在响应信号时进程会对改变原定执行流程。
												 * 
												 * 返回值：信号数，无信号则返回0。
												 */

public:
	/* 用于标识进程的标识 */
	short p_uid;		/* 用户ID */
	int p_pid;			/* 进程标识数，进程编号 */
	int p_ppid;			/* 父进程标识数 */

	/* 进程内存中图像信息位置 */
	unsigned long	p_addr; /* TBD user结构即ppda区在物理内存中的地址，用于替代页表中的某一项 */
	unsigned int	p_size; /* 除共享正文段的长度，以字节单位 */
	Text*	p_textp;		/* 指向该进程所运行的代码段的描述符 */

	/* 进程调度状态 */
	ProcessState	p_stat;	/* 进程当前状态 */
	int p_flag;			/* 进程标志位，可以将多个状态组合 */

	int p_pri;			/* 进程优先数 */
	int p_cpu;			/* cpu值，用于计算p_pri */
	int p_nice;			/* 进程优先数微调参数 */
	int p_time;			/* 进程在盘上(内存内)驻留时间 */

	unsigned long	p_wchan;	/* 进程睡眠原因，一般为内存地址，等待某个内核数据 */
	
	/* 信号与控制台终端 */
	int p_sig;			/* 进程信号 */
	TTy* p_ttyp;		/* 进程tty结构地址 */
	unsigned long p_sigmap;
};

#endif
