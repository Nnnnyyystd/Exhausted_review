#ifndef SYSTEM_CALL_H
#define SYSTEM_CALL_H

/*
 * SystemCallTableEntry结构是系统
 * 调用处理程序入口表的表项。
 *
 * 对应在UnixV6代码中的sysent结构
 * struct sysent		@line 2667
 * {
 *	int count;
 *	int (*call)();
 * }
 */
/*系统调用入口表表项的定义*/
struct SystemCallTableEntry
{
	unsigned int count; // 系统调用的参数个数
	int (*call)();		// 相应系统调用处理函数的指针
};

/*
 * UNIX V6中使用编译后trap指令码的低6bit作为index查找入口表，这依靠
 * trap指令能够针对不同系统调用产生不同指令码。而X86平台上的int指令
 * 无法做到产生不同的指令码，因而通过eax寄存器传入系统调用号作为index。
 *
 * eax中存放系统调用号，作为查找入口表中函数的index。
 * ebx开始存放用户态程序提供的系统调用第一个参数，ecx第二参数，以此类推
 * ebp存放最后的参数；最多可以有6个参数。其实UNIX V6的系统调用参数最多只有4个。
 *　
 * 随后会将传入参数转存到u.u_arg[5]中。
 */
class SystemCall
{
public:
	/*系统调用处理程序入口表的大小*/
	static const unsigned int SYSTEM_CALL_NUM = 64;

public:
	SystemCall();
	~SystemCall();

public:
	/* 偏移地址存放在IDT[0x80]陷入门中的系统调用入口函数
	 *
	 *	UNIX V6中的陷入"入口函数"的名字叫trap(@line 752)，但它是用汇编写的，
	 * 与C函数trap(dev, sp, r1, nps, r0, pc, ps)同名，负责保存现场，将dev，
	 * sp等参数压入核心栈，然后调用C语言写的trap(dev, sp, r1, nps, r0, pc, ps)
	 *
	 * "汇编trap" @line 0755负责的还包括：等陷入处理子程序返回以后，判断
	 * 是否需要swtch()，以及恢复现场的工作。
	 */
	static void SystemCallEntrance();

	/* 对应UNIX V6中的trap(dev, sp, r1, nps, r0, pc, ps)函数,
	 * 主要参照V6中系统调用的switch分支：case 6+USER: // sys call
	 * 其它的异常在X86平台上由INT 0-31的handler处理，不像V6那样
	 * 在trap(dev,...)中通过switch来区分不同的陷入(异常)。
	 */
	static void Trap(struct pt_regs *regs, struct pt_context *context);

	/* 对应UNIX V6中的trap1( int (*f)() )函数@line 2841
	 * 此函数由trap(dev,...)函数调用，trap(dev,...)函数
	 * 提供从入口表中获取的函数指针，作为参数传递给trap1( int (*f)());
	 */
	static void Trap1(int (*func)());

private:
	/* 下面的函数对应系统调用入口表中的处理程序入口地址,
	 * 他们负责系统调用在核心态下进行的具体处理逻辑。
	 *
	 * 这里函数统一声明为int func(void);而系统调用的返回值
	 * 并不是通过int返回，只是为了和int (*call)()类型匹配。
	 *
	 * UNIX V6中返回值放在u.u_ar0[R0]中，也就是通过r0寄存器
	 * 返回，而这里考虑使用EAX寄存器返回系统调用结果给用户态
	 * 程序。
	 */

	/*	0 = indir	count = 0	*/
	static int Sys_NullSystemCall(); /*在V6中用作间接系统调用，x86上不需要，不会被调用到的空函数 */

	/*	1 = rexit	count = 0	*/
	static int Sys_Rexit();

	/*	2 = fork	count = 0	*/
	static int Sys_Fork();

	/*	3 = read	count = 2	*/
	static int Sys_Read();

	/*	4 = write	count = 2	*/
	static int Sys_Write();

	/*	5 = open	count = 2	*/
	static int Sys_Open();

	/*	6 = close	count = 0	*/
	static int Sys_Close();

	/*	7 = wait	count = 0	*/
	static int Sys_Wait();

	/*	8 = creat	count = 2	*/
	static int Sys_Creat();

	/*	9 = link	count = 2	*/
	static int Sys_Link();

	/*	10 = unlink	count = 1	*/
	static int Sys_UnLink();

	/*	11 = exec	count = 2	*/
	static int Sys_Exec();

	/*	12 = chdir	count = 1	*/
	static int Sys_ChDir();

	/*	13 = gtime	count = 0	*/
	static int Sys_GTime();

	/*	14 = mknod	count = 3	*/
	static int Sys_MkNod();

	/*	15 = chmod	count = 2	*/
	static int Sys_ChMod();

	/*	16 = chown	count = 2	*/
	static int Sys_ChOwn();

	/*	17 = sbreak	count = 1	*/
	static int Sys_SBreak();

	/*	18 = stat	count = 2	*/
	static int Sys_Stat();

	/*	19 = seek	count = 2	*/
	static int Sys_Seek();

	/*	20 = getpid	count = 0	*/
	static int Sys_Getpid();

	/*	21 = mount	count = 3	*/
	static int Sys_Smount();

	/*	22 = umount  count = 1	*/
	static int Sys_Sumount();

	/*	23 = setuid	count = 0	*/
	static int Sys_Setuid();

	/*	24 = getuid	count = 0	*/
	static int Sys_Getuid();

	/*	25 = stime	count = 0	*/
	static int Sys_Stime();

	/*	26 = ptrace	count = 3	*/
	static int Sys_Ptrace();

	/*	27 = nosys	count = 0	*/
	static int Sys_Nosys(); /* 表示当前系统调用号保留未使用，用作将来扩展 */

	/*	28 = fstat	count = 1	*/
	static int Sys_FStat();

	/*	29 = trace	count = 1	*/
	static int Sys_Trace();

	/*	30 =  smdate; inoperative	count = 1	handler = nullsys	*/

	/*	31 = stty	count = 1	*/
	static int Sys_Stty();

	/*	32 = gtty	count = 1	*/
	static int Sys_Gtty();

	/*	33 = nosys	count = 0	*/

	/*	34 = nice	count = 0	*/
	static int Sys_Nice();

	/*	35 = sleep	count = 0	*/
	static int Sys_Sslep(); /* Don't Confused with sleep(chan, pri) */

	/*	36 = sync	count	= 0	*/
	static int Sys_Sync();

	/*	37 = kill	count = 1	*/
	static int Sys_Kill();

	/*	38 = switch	count = 0	*/
	static int Sys_Getswit();

	/*	39 = pwd	count = 1	*/
	static int Sys_Pwd();

	/*	40 = nosys	count = 0	*/

	/*	41 = dup	count = 0	*/
	static int Sys_Dup();

	/*	42 = pipe	count = 0	*/
	static int Sys_Pipe();

	/*	43 = times	count = 1	*/
	static int Sys_Times();

	/*	44 = prof	count = 4	*/
	static int Sys_Profil();

	/*	45 = nosys	count = 0	*/

	/*	46 = setgid	count = 0	*/
	static int Sys_Setgid();

	/*	47 = getgid	count = 0	*/
	static int Sys_Getgid();

	/*	48 = sig	count = 2	*/
	static int Sys_Ssig();

	/*	49 ~ 63 = nosys	count = 0	*/

private:
	/*系统调用入口表的声明*/
	static SystemCallTableEntry m_SystemEntranceTable[SYSTEM_CALL_NUM];
};

#endif
