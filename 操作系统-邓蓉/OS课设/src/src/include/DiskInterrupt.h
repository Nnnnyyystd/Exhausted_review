#ifndef DISK_INTERRUPT_H
#define DISK_INTERRUPT_H

class DiskInterrupt
{
public:
	/* 磁盘中断入口函数，其地址存放在IDT的磁盘中断对应中断门中 */
	static void DiskInterruptEntrance();
};

#endif
