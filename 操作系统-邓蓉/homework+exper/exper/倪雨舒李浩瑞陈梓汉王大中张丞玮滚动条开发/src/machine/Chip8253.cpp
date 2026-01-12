#include "Chip8253.h"
#include "IOPort.h"

/* 对8253时钟芯片进行初始化，默认每秒产生60次时钟中断 */
void Chip8253::Init(int ticks /*= 60*/)
{
	if(ticks <= 0)
	{
		ticks = 60;	/* 每秒时钟中断次数不允许为负数 */
	}

	//向控制字端口43H，写入命令值
	IOPort::OutByte(CTRLWRD_PORT, CTRLCMD_SEL0 | CTRLCMD_MODE3 | CTRLCMD_RW );
	//向计数器0写入计数值，先写低8位
	IOPort::OutByte(CNT0_PORT, (INPUT_FREQ / ticks) % 256 );
	//然后写计数值的高8位，端口地址不变
	IOPort::OutByte(CNT0_PORT, (INPUT_FREQ / ticks) / 256 );	
	
	/*
	 * 至此，时钟芯片开始以Ticks次/秒发出时钟中断，但是8259A还没有被初始化好，
	 * 并不能接收时钟中断，转达给CPU。同时CPU的IF标志位进入保护模式前被清零了，
	 * 目前还没有sti。
	 */
	return;
}
