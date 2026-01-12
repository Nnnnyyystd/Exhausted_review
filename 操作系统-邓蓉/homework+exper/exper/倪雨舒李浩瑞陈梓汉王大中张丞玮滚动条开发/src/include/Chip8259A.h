#ifndef CHIP8259A_H
#define CHIP8259A_H

/*
 * 定义了对8259A可编中断控制芯片(PIC)的操作。
 * 
 * 8259A芯片帮助CPU代为管理外设提出的中断请求,
 * 选择优先级最高的中断,转达给CPU来响应中断。
 */

class Chip8259A
{
public:
	/* 初始化系统中的主、从两片8259A中断控制芯片 */
	static void Init();
	
	/* 开启中断函数。
	 *    
	 * 功能：通过设置8259A的中断屏蔽寄存器，将相应
	 * 中断屏蔽位清0，允许来自特定外设的中断。
	 * 
	 * 输入参数：特定外设的IRQ号。如下面定义的: IRQ_TIMER = 0;等
	 */
	static void IrqEnable(unsigned int irq);
	
	/* 屏蔽中断函数。 和IrqEnable(unsigned int irq)执行反向功能。
	 * 
	 * 功能：通过设置8259A的中断屏蔽寄存器，将相应
	 * 中断屏蔽位置1，屏蔽来自特定外设的中断。
	 * 
	 * 输入参数：特定外设的IRQ号。如下面定义的: IRQ_TIMER = 0; 等
	 */
	static void IrqDisable(unsigned int irq);
	
public:
	/* 系统中有2片8259A芯片，每一片在IO地址空间中占用2个端口地址 */
	/* 主片(Master)的IO端口地址 */
	static const unsigned short MASTER_IO_PORT_1 = 0x20;
	static const unsigned short MASTER_IO_PORT_2 = 0x21;
	
	/* 从片(Slave)的IO端口地址 */
	static const unsigned short SLAVE_IO_PORT_1 = 0xA0;
	static const unsigned short SLAVE_IO_PORT_2 = 0xA1;
	
	/* 主片各引脚对应起始中断号 */
	static const unsigned char MASTER_IRQ_START = 0x20;
	/* 从片各引脚对应起始中断号,中断号范围起始0x28 */
	static const unsigned char SLAVE_IRQ_START = MASTER_IRQ_START + 8;	
	
	/* 
	 * 主片(IR0~IR7)连接到的外设对应的中断请求引脚 (这里只定义内核中
	 * 用到的外设)后面可以有选择性地 启用/禁用来自于该引脚的中断。
	 */
	static const unsigned int IRQ_TIMER = 0;	/* 时钟中断(IRQ0)发送到IR0引脚 */
	static const unsigned int IRQ_KBD	= 1;	/* 键盘中断(IRQ1)发送到IR1引脚 */
	static const unsigned int IRQ_SLAVE = 2;	/* 级联模式下,从片发出的中断(Slave的INT引脚),发送到主片的IR2 */
	
	/* 从片(IR0~IR7)连接到的外设对应的中断请求引脚,这里只用到的外设是硬盘 */
	static const unsigned int IRQ_IDE	= 14;	/* 硬盘中断(IRQ14)发送到从片IR6引脚 */
	
	/* 另外需要定义的一些常量 */
	static const unsigned char MASK_ALL = 0xFF;	/* 屏蔽状态字, 屏蔽所有引脚(IR0~IR7)上的中断请求 */
	static const unsigned char EOI = 0x20;		/* End Of Interrupt */
};

#endif
