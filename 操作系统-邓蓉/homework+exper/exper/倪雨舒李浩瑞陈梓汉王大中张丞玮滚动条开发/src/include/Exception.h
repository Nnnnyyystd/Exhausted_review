#ifndef EXCEPTION_H
#define	EXCEPTION_H

#include "Regs.h"

/* 
 * Exception类封装I386平台上INT 0 - 31号范围异常的处理函数。
 * 
 * 对于每一种可能产生的异常，Exception类中提供对该异常的处理
 * 入口函数(Entrance)和异常处理函数(Handler)，入口函数作为入口
 * 地址记录在中断描述符表IDT中，异常处理函数执行具体异常处理
 * 逻辑。
 */

class Exception
{
public:
	Exception();
	~Exception();

	//除零错(INT 0)
	static void DivideErrorEntrance();
	static void DivideError(struct pt_regs* regs, struct pt_context* context);

	//调试异常(INT 1)
	static void DebugEntrance();
	static void Debug(struct pt_regs* regs, struct pt_context* context);

	//NMI非屏蔽中断(INT 2)
	static void NMIEntrance();
	static void NMI(struct pt_regs* regs, struct pt_context* context);

	//调试断点(INT 3)
	static void BreakpointEntrance();
	static void Breakpoint(struct pt_regs* regs, struct pt_context* context);

	//溢出(INT 4)
	static void OverflowEntrance();
	static void Overflow(struct pt_regs* regs, struct pt_context* context);

	//BOUND指令异常(INT 5)
	static void BoundEntrance();
	static void Bound(struct pt_regs* regs, struct pt_context* context);

	//无效操作码(INT 6)
	static void InvalidOpcodeEntrance();
	static void InvalidOpcode(struct pt_regs* regs, struct pt_context* context);

	//设备不可用(INT 7)
	static void DeviceNotAvailableEntrance();
	static void DeviceNotAvailable(struct pt_regs* regs, struct pt_context* context);

	//双重错误(INT 8)  *有出错码*
	static void DoubleFaultEntrance();
	static void DoubleFault(struct pt_regs* regs, struct pte_context* context);

	//协处理器段越界(INT 9)
	static void CoprocessorSegmentOverrunEntrance();
	static void CoprocessorSegmentOverrun(struct pt_regs* regs, struct pt_context* context);

	//无效TSS(INT 10)  *有出错码*
	static void InvalidTSSEntrance();
	static void InvalidTSS(struct pt_regs* regs, struct pte_context* context);

	//段不存在(INT 11)  *有出错码*
	static void SegmentNotPresentEntrance();
	static void SegmentNotPresent(struct pt_regs* regs, struct pte_context* context);

	//堆栈段错误(INT 12)  *有出错码*
	static void StackSegmentErrorEntrance();
	static void StackSegmentError(struct pt_regs* regs, struct pte_context* context);

	//一般保护性异常(INT 13)  *有出错码*
	static void GeneralProtectionEntrance();
	static void GeneralProtection(struct pt_regs* regs, struct pte_context* context);

	//缺页异常(INT 14)  *有出错码*
	static void PageFaultEntrance();
	static void PageFault(struct pt_regs* regs, struct pte_context* context);

	//Intel保留异常(INT 15)
	/* INT 15和INT 20 - 31同样采用默认的处理函数IDT::DefaultExceptionHandler()，无需在Excpetion类中定义 */

	//x87 FPU浮点错误(INT 16)
	static void CoprocessorErrorEntrance();
	static void CoprocessorError(struct pt_regs* regs, struct pt_context* context);

	//对齐校验(INT 17)  *有出错码*
	static void AlignmentCheckEntrance();
	static void AlignmentCheck(struct pt_regs* regs, struct pte_context* context);

	//机器检查(INT 18)
	static void MachineCheckEntrance();
	static void MachineCheck(struct pt_regs* regs, struct pt_context* context);

	//SIMD浮点异常(INT 19)
	static void SIMDExceptionEntrance();
	static void SIMDException(struct pt_regs* regs, struct pt_context* context);

	//Intel保留异常(INT 20 - INT 31)这部分为Intel保留未使用的异常向量
	/* 采用默认的处理函数IDT::DefaultExceptionHandler() */
};

#endif
