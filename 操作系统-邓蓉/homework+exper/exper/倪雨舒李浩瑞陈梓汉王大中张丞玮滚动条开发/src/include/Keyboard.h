#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "Regs.h"
#include "CRT.h"

class Keyboard
{
	/* Const Member */
public:
	/* 键盘I/O端口地址 */
	static const unsigned short DATA_PORT = 0x60;	/* 键盘数据寄存器端口号 */
	static const unsigned short STATUS_PORT = 0x64;	/* 键盘状态寄存器端口号 */

	/* 状态寄存器比特位定义 (端口号：0x64) */
	static const unsigned char DATA_BUFFER_BUSY = 0x1;	/* 键盘输出缓存是否满 */

	/* 扫描码常量定义 */
	static const unsigned char SCAN_ALT = 0x38;
	static const unsigned char SCAN_CTRL = 0x1d;
	static const unsigned char SCAN_LSHIFT = 0x2a;
	static const unsigned char SCAN_RSHIFT = 0X36;
	static const unsigned char SCAN_ESC = 0x01;
	static const unsigned char SCAN_NUMLOCK = 0x45;
	static const unsigned char SCAN_CAPSLOCK = 0x3a;
	static const unsigned char SCAN_SCRLOCK = 0x46;

	/* 以下为控制键按下的状态，mode中各比特位的定义 */
	static const int M_LCTRL = 0x01;
	static const int M_RCTRL = 0x02;
	static const int M_LALT = 0x04;
	static const int M_RALT = 0x08;
	static const int M_LSHIFT = 0x10;
	static const int M_RSHIFT = 0x20;
	static const int M_NUMLOCK = 0x40;
	static const int M_CAPSLOCK = 0x80;
	static const int M_SCRLOCK = 0x100;
	static const int M_DOWN_NUMLOCK = 0x200;
	static const int M_DOWN_CAPSLOCK = 0x400;
	static const int M_DOWN_SCRLOCK = 0x800;

	static const unsigned char SCAN_W = 0x11;
	static const unsigned char SCAN_S = 0x1F;
	static const unsigned char SCAN_PAGEUP = 0x49;
	static const unsigned char SCAN_PAGEDOWN = 0x51;
	static const unsigned char SCAN_LEFT = 0x4B;
	static const unsigned char SCAN_RIGHT = 0x4D;
	static const unsigned char SCAN_UP = 0x48;
	static const unsigned char SCAN_DOWN = 0x50;
	static const unsigned char SCAN_DELETE = 0x53;



	/* Functions */
public:
	/* 键盘中断设备处理子程序 */
	static void KeyboardHandler(struct pt_regs* reg, struct pt_context* context);

	/* 
	 * 处理扫描码子程序 scanCode(扫描码) expand(扩展码)
	 * expand表示是否是扩展的键，主要用于判断左右的ctrl和alt
	 */
	static void HandleScanCode(unsigned char scanCode, int expand);

	/* 将键盘扫描码转换为相应的ASCII码 */
	static char ScanCodeTranslate(unsigned char scanCode, int expand);
	
	/* Members */
public:
	/* 
	 * 键盘映射仅仅取了一个键盘键的子集，比如<fx>功能键没
	 * 有设计，这里所与标0的asc码表示在该系统中没有映射。
	 * keymap 是在shift键没有按下的情况下扫描码的映射情况
	 * 不过对于大于0x45的扫描码，也就是小键盘区与功能区的扫
	 * 描码，这里表示numlock键按下的情况
	 */
	static char Keymap[];

	/* shift_keymap则是shift按下的情况 */
	static char Shift_Keymap[];

	/* ctrl, alt, shift的状态，并置numlock,capslock,scrlock三键松开*/
	static int Mode;
};

#endif
