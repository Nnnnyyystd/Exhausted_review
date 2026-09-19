#include "TimeInterrupt.h"
#include "Kernel.h"
#include "Regs.h"
#include "IOPort.h"
#include "Chip8259A.h"
#include "Video.h"

int Time::lbolt = 0;
unsigned int Time::time = 0;
unsigned int Time::tout = 0;

void Time::TimeInterruptEntrance()
{
	SaveContext();			/* 保存中断现场 */

	SwitchToKernel();		/* 进入核心态 */

	CallHandler(Time, Clock);		/* 调用时钟中断处理子程序 */

	/* 获取由中断隐指令(即硬件实施)压入核心栈的pt_context。
	* 这样就可以访问context.xcs中的OLD_CPL，判断先前态
	* 是用户态还是核心态。
	*/
	struct pt_context *context;
	__asm__ __volatile__ ("	movl %%ebp, %0; addl $0x4, %0 " : "+m" (context) );

	if( context->xcs & USER_MODE ) /*先前为用户态*/
	{
		while(true)
		{
			X86Assembly::CLI();	/* 处理机优先级升为7级 */
			
			if(Kernel::Instance().GetProcessManager().RunRun > 0)
			{
				X86Assembly::STI();	/* 处理机优先级降为0级 */
				Kernel::Instance().GetProcessManager().Swtch();
			}
			else
			{
				break;	/* 如果runrun == 0，则退栈回到用户态继续用户程序的执行 */
			}
		}
	}

	RestoreContext();		/* 恢复现场 */

	Leave();				/* 手工销毁栈帧 */

	InterruptReturn();		/* 退出中断 */
}

void Time::Clock( struct pt_regs* regs, struct pt_context* context )
{
	User& u = Kernel::Instance().GetUser();
	ProcessManager& procMgr = Kernel::Instance().GetProcessManager();

	/* 系统或用户时间计时，如果先前态为用户态，mode为非零 */
	if ( (context->xcs & USER_MODE) == USER_MODE )
	{
		u.u_utime++;
	}
	else
	{
		u.u_stime++;
	}

	Process* current = u.u_procp;
	/* 计算当前进程占用的CPU时间 */
	current->p_cpu = Utility::Min(++current->p_cpu, 1024);

	/* 到了一秒末尾，根据先前态决定是否对所有进程重算优先数 */
	if ( ++Time::lbolt < HZ )
    	{
		/* 对主8259A中断控制芯片发送EOI命令。 */
		    IOPort::OutByte(Chip8259A::MASTER_IO_PORT_1, Chip8259A::EOI);
        	return;
    	}
    else
	{
        /* 中断前为核心态，把耗时的计算留在下一次时钟中断再考虑 */
		/*if( (context->xcs & USER_MODE) == KERNEL_MODE && current->p_pid != 0)*/
    	if( current->p_stat == Process::SRUN && (context->xcs & USER_MODE) == KERNEL_MODE )
		{
			/* 对主8259A中断控制芯片发送EOI命令。 */
			    IOPort::OutByte(Chip8259A::MASTER_IO_PORT_1, Chip8259A::EOI);
        		return;
   		 }

		/* 以下为一秒末尾，进行耗时的计算过程 */
		Time::lbolt -= HZ;

		/* 系统全局时间+1，以秒为单位 */
		Time::time++;

		/* 允许中断进入，相当于降低处理机优先级 */
		X86Assembly::STI();
	    /* 对主8259A中断控制芯片发送EOI命令。 */
	    IOPort::OutByte(Chip8259A::MASTER_IO_PORT_1, Chip8259A::EOI);


		if ( Time::time == Time::tout )
		{
			/* 唤醒延时睡眠的进程 */
			procMgr.WakeUpAll((unsigned long)&Time::tout);
		}

		/* 重算所有进程的p_time, p_cpu,以及优先数p_pri */
		for( int i = 0; i < ProcessManager::NPROC; i++ )
		{
			Process* pProcess = &procMgr.process[i];
			if ( pProcess->p_stat != Process::SNULL )
			{
				pProcess->p_time = Utility::Min(++pProcess->p_time, 127);
				
				if ( pProcess->p_cpu > SCHMAG )
				{
					pProcess->p_cpu -= SCHMAG;
				}
				else
				{
					pProcess->p_cpu = 0;
				}
				if ( pProcess->p_pri > ProcessManager::PUSER )
				{
					pProcess->SetPri();
					//Diagnose::Write("PID = %d, p_cpu = %d, p_pri = %d\n", pProcess->p_pid, pProcess->p_cpu, pProcess->p_pri);
				}
			}
		}
		//Diagnose::Write("curpri = %d\n", procMgr.CurPri);
		//Diagnose::Write("System Time: %d\n", Time::time);
		
		if ( procMgr.RunIn != 0 )
		{
			procMgr.RunIn = 0;
			procMgr.WakeUpAll((unsigned long)&procMgr.RunIn);
		}

		/* 如果中断前为用户态，则考虑进行信号处理 */
		if ( (context->xcs & USER_MODE) == USER_MODE )
		{
			if ( current->IsSig() )
			{
				current->PSig(context);
			}
			/* 计算当前进程优先数 */
			current->SetPri();
		}
	}
}
