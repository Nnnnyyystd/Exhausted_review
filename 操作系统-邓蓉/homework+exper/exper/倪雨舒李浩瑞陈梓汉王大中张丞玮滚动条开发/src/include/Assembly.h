#ifndef	ASSEMBLY_H
#define	ASSEMBLY_H

/*
 * X86Assembly类定义了对x86平台中部分特权级指令的抽象。
 * 
 * 使用C++ Inline Assembly 封装lgdt, lidt, ltr, cli, 
 * sti等指令，用于解决C++语言无法通过编译产生的特权级
 * 指令，完成开/关中断、加载gdt、idt等任务。
*/

/* 刷新页表，在每次对页表进行修改后需要调用，重新缓存页表 */
#define FlushPageDirectory()	\
	__asm__ __volatile__(" movl %0, %%cr3" : : "r"(0x200000) );

class X86Assembly
{
	public:
		//允许中断
		static inline void STI()
		{
			__asm__ __volatile__("sti");
		}
		
		//屏蔽中断
		static inline void CLI()
		{
			__asm__ __volatile__("cli");
		}
		
		//lidt指令
		static inline void LIDT(unsigned short idtr[3])
		{
			__asm__ __volatile__("lidt %0"::"m" (*idtr));
			//特别提醒：lidt指令的操作数是6字节的Limit+BaseAddress, 而不是这6个字节的首地址
			//所以“(*idtr)”中的“ * ”不可以漏掉！！！
		}
		
		//lgdt指令
		static inline void LGDT(unsigned short gdtr[3])
		{
			__asm__ __volatile__("lgdt %0"::"m" (*gdtr));
		}

		//ltr指令
		static inline void LTR(unsigned short tssSelector)
		{
			__asm__ __volatile__("mov %0, %%ax\n\tltr %%ax"::"m"(tssSelector));
		}
};
#endif
