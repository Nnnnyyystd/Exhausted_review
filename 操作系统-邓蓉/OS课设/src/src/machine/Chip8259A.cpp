#include "Chip8259A.h"
#include "IOPort.h"

void Chip8259A::Init()
{
	/* Initialize 8259A */
	//编程ICW1确定主、从片的工作模式为级联模式、边沿触发
	IOPort::OutByte(MASTER_IO_PORT_1, 0x11);
	IOPort::OutByte(SLAVE_IO_PORT_1, 0x11);
	
	//编程ICW2确定主、从片对应的中断向量号
	IOPort::OutByte(MASTER_IO_PORT_2, MASTER_IRQ_START);
	IOPort::OutByte(SLAVE_IO_PORT_2,  SLAVE_IRQ_START);
	
	//编程ICW3确定主、从片级联的引脚情况
	/* TO MASTER:  主片的IR2引脚连接了从片,所以IR2对应比特位为1 。*/
	IOPort::OutByte(MASTER_IO_PORT_2, 1 << IRQ_SLAVE); /* 0x04 = 0000 0100b */
	/* TO SLAVE: 从片连接到主片的IR2引脚, 所以要使得D2~D0 等于 2。*/
	IOPort::OutByte(SLAVE_IO_PORT_2, IRQ_SLAVE);
	
	//编程ICW4确定主、从片的中断执行模式
	//普通全嵌套模式、正常EOI(End of Interrupt)、Intel X86模式
	IOPort::OutByte(MASTER_IO_PORT_2, 0x01);
	IOPort::OutByte(SLAVE_IO_PORT_2, 0x01);
	
	/* 初始化之后先屏蔽所有中断,写OCW1寄存器,让中断屏蔽位处于一个已知状态下。*/
	IOPort::OutByte(MASTER_IO_PORT_2, MASK_ALL);
	IOPort::OutByte(SLAVE_IO_PORT_2,  MASK_ALL);	
	return;
}


void Chip8259A::IrqEnable(unsigned int irq)
{
	/* 
	 * IRQ范围在0～15之内，irq是unsigned int类型，小于0不可能，
	 * 其实这里irq可能值应该只会是IRQ_TIMER，IRQ_KBD等已经定义好
	 * 的几个。其它IRQ号，如IRQ6软盘驱动器中断之类在内核中并未涉及。
	 */
	if(irq >= 16)
		{return; /*.....输出错误信息*/}
	
	unsigned char value;
	
	if(irq <= 7) /* 0 <= irq <= 7,  对MASTER chip的OCW1操作*/
	{
		// Get ICW1
		value = IOPort::InByte(MASTER_IO_PORT_2);
		value &= ~(1 << irq);
		//Write ICW1 back to 8259A
		IOPort::OutByte(MASTER_IO_PORT_2, value);
	}
	else /*对SLAVE chip的OCW1操作*/
	{
		value = IOPort::InByte(SLAVE_IO_PORT_2);
		value &= ~(1 << (irq - 8));
		IOPort::OutByte(SLAVE_IO_PORT_2, value);
	}
	
	return;
}

void Chip8259A::IrqDisable(unsigned int irq)
{
	if(irq >= 16)
		{return; /*.....输出错误信息*/}
	
	unsigned char value;
	
	if(irq <= 7)	/* 0 <= irq <= 7,  对MASTER chip的OCW1操作*/
	{
		value = IOPort::InByte(MASTER_IO_PORT_2);
		value |= (1 << irq);
		IOPort::OutByte(MASTER_IO_PORT_2, value);
	}
	else /*对SLAVE chip的OCW1操作*/
	{
		value = IOPort::InByte(SLAVE_IO_PORT_2);
		value |= (1 << (irq - 8));
		IOPort::OutByte(SLAVE_IO_PORT_2, value);
	}
	return;
}
