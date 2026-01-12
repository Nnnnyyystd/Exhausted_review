#include "KeyboardInterrupt.h"
#include "Kernel.h"
#include "Regs.h"
#include "Keyboard.h"
#include "IOPort.h"
#include "Chip8259A.h"

void KeyboardInterrupt::KeyboardInterruptEntrance()
{
	SaveContext();			/* 保存中断现场 */

	SwitchToKernel();		/* 进入核心态 */

	CallHandler(Keyboard, KeyboardHandler);		/* 调用键盘中断设备处理子程序 */

	/* 对主8259A中断控制芯片发送EOI命令。 */
	IOPort::OutByte(Chip8259A::MASTER_IO_PORT_1, Chip8259A::EOI);

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
