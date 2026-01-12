#ifndef CHIP8253_H
#define CHIP8253_H

/*
 * 定义对8253可编程定时芯片(PIT)的操作。
 *
 * 8253芯片用于产生固定间隔的时钟中断。
 */
class Chip8253
{
public:
	/* 对8253时钟芯片进行初始化，默认每秒产生60次时钟中断 */
	static void Init(int ticks = 60); 

private:
	/* 下面是一些关于端口地址、命令值和计数值的常量 */
	static const unsigned int INPUT_FREQ = 1193180;		/* 芯片输入频率为1.193180MHz */
	static const unsigned short CNT0_PORT = 0x40;		/* 计数器0的端口地址40H */
	static const unsigned short CTRLWRD_PORT = 0x43;	/* 控制字端口地址43H */
	static const unsigned char CTRLCMD_SEL0 = 0x00;		/* 选择计数器0 */
	static const unsigned char CTRLCMD_MODE3 = 0x06;	/* 计数模式: 输出固定间隔、无限循环的方波,作为时钟中断 */	
	static const unsigned char CTRLCMD_RW = 0x30;		/* 计数值读写模式: 写入,16比特,先低字节,后高字节 */
};

#endif
