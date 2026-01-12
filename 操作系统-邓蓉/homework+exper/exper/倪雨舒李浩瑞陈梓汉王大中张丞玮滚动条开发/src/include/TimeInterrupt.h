#ifndef TIME_INTERRUPT_H
#define TIME_INTERRUPT_H

class Time
{
	/* 
	 * 注：由于Bochs的时钟中断不准确，一秒内时钟中断次数远大于60次，
	 * 为了尽可能减小系统时间的误差，常数SCHMAG和HZ的值同比扩大。
	 * 以下取值经过调试发现在Bochs下比较理想，请在了解影响范围后再进行改动！！。
	 */
public:
	static const int SCHMAG = 10 * 2;	/* 每秒钟减少的p_cpu魔数 */

	static const int HZ = 60 * 2;		/* 每秒钟时钟中断次数 */

	static int lbolt;				/* 累计接收到的时钟中断次数 */
	
	static unsigned int time;		/* 系统全局时间，自1970年1月1日至今的秒数 */

	static unsigned int tout;		/* 各延时睡眠进程中应被唤醒的时刻中最小值 */

	/* 时钟中断入口函数，其地址存放在IDT的磁盘中断对应中断门中 */
	static void TimeInterruptEntrance();

	/* 时钟中断处理函数，维护系统时钟日历，计算进程占用CPU时间，处理睡眠进程和延时队列等工作 */
	static void Clock(struct pt_regs* regs, struct pt_context* context);
};

#endif
