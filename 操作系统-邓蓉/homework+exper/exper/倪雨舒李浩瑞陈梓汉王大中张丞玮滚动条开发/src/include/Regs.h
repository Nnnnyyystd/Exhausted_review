#ifndef REGS_H
#define REGS_H

const unsigned int KERNEL_MODE = 0x0;
const unsigned int USER_MODE = 0x3;

struct pt_regs{
	unsigned int pad1;
	unsigned int pad2;
	unsigned int xds;
	unsigned int xes;
	unsigned int ebx;
	unsigned int ecx;
	unsigned int edx;
	unsigned int esi;
	unsigned int edi;
	unsigned int ebp;
	unsigned int eax;
};

/* 
 * 此部分数据结构是中断发生时刻，由中断隐指令保护入栈的寄存器
 * 包括eip，cs，eflags； 如果发生特权级变化，导致堆栈切换的话，
 * 应指令保护入栈的还有切换前堆栈的SS:ESP；此结构体所有字段都
 * 占据4个字节。
 */
struct pt_context{
	unsigned int eip;
	unsigned int xcs;
	unsigned int eflags;
	unsigned int esp;
	unsigned int xss;
};

/* 由中断隐指令保护入栈的数据，比pt_context多增加error_code字段 */
struct pte_context{
	unsigned int error_code;	//隐指令压入的error code
	unsigned int eip;
	unsigned int xcs;
	unsigned int eflags;
	unsigned int esp;
	unsigned int xss;
};						

#define SaveContext() \
	__asm__ __volatile__("	cld; \
							pushl %%eax; \
							pushl %%ebp; \
							pushl %%edi; \
							pushl %%esi; \
							pushl %%edx; \
							pushl %%ecx; \
							pushl %%ebx; \
							pushl %%es;  \
							pushl %%ds;  \
							pushl %%fs;  \
							pushl %%gs;  \
							lea 0x4(%%ebp), %%edx; \
							pushl %%edx; \
							lea 0x4(%%esp), %%edx; \
							pushl %%edx"::);
							
							
/* 装载核心态数据段，堆栈段描述符，在SaveContext()之后切换进入核心态 */
#define SwitchToKernel() \
	__asm__ __volatile__("	mov $0x10,	%%dx;	\
							mov	%%dx,	%%ds;	\
							mov %%dx,	%%es"::);
							

/* 保存现场完成之后调用中断(陷入)处理函数。*/
#define CallHandler(Class, Handler) \
	__asm__ __volatile__("	call *%%eax" :: "a" (Class::Handler) );
	
	
#define RestoreContext() \
	__asm__ __volatile__("	addl $0x8, %%esp; \
							popl %%gs;	\
							popl %%fs;	\
							popl %%ds;	\
							popl %%es;	\
							popl %%ebx; \
							popl %%ecx; \
							popl %%edx; \
							popl %%esi; \
							popl %%edi; \
							popl %%ebp; \
							popl %%eax;"::);
							
#define Leave()	\
	__asm__ __volatile__("	leave"	:: );
							
							
#define InterruptReturn() \
	__asm__ __volatile__("	iret"	:: );
							
							
#define GetContext(pContext)	\
	__asm__ __volatile__ ("	movl %%ebp, %0;		\
							addl $0x4, %0 "		\
							: "+m" (pContext) );
							
#define GetContextErrcode(pContext)	\
	__asm__ __volatile__ ("	movl %%ebp, %0;		\
							addl $0x8, %0 "		\
							: "+m" (pContext) );
							
#define MoveToUserStack()	\
		__asm__ __volatile__("	movl $0x800000,%%eax;	\
								pushl $0x23;			\
								pushl %%eax;			\
								pushfl;					\
								pushl $0x1b;			\
								pushl $1f - 0xC0000000;	\
								iret;					\
								1:						\
								movl %%esp, %%ebp;		\
								movl $0x23, %%eax;		\
								movw %%ax, %%ds;		\
								movw %%ax, %%es;"::);

#endif
