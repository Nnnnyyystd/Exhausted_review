#ifndef KEYBOARD_INTERRUPT_H
#define KEYBOARD_INTERRUPT_H

class KeyboardInterrupt
{
public:
	/* 键盘中断入口函数，其地址存放在IDT的键盘中断对应中断门 */
	static void KeyboardInterruptEntrance();
};

#endif
