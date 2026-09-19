#include "Keyboard.h"
#include "IOPort.h"
#include "Kernel.h"
#include "ProcessManager.h"
#include "Video.h"
#include "CharDevice.h"

char Keyboard::Keymap[] = {
	0,0x1b,'1','2','3','4','5','6',		/* 0x00-0x07 0, <esc>,1,2,3,4,5,6, */
	'7','8','9','0','-','=',0x8,0x9,	/* 0x08-0x0f 7,8,9,0,-,=,<backspace><tab>*/
	'q','w','e','r','t','y','u','i',	/* 0x10-0x17 qwertyui*/
	'o','p','[',']','\n',0,'a','s', 	/* 0x18-0x1f op[] <enter><ctrl>as */
	'd','f','g','h','j','k','l',';',	/* 0x20-0x27 dfghjkl; */
	'\'','`',0,'\\','z','x','c','v',	/* 0x28-0x2f '`<lshift>\zdcv */
	'b','n','m',',','.','/',0,'*', 		/* 0x30-0x37 bnm,./<rshitf><printscr> */
	0,' ',0,0,0,0,0,0, 					/* 0x38-0x3f <alt><space><caps><f1><f2><f3><f4><f5> */
	0,0,0,0,0,0,0,'7', 					/* 0x40-0x47 <f0><><><><><numlock><scrlock>7*/
	'8','9','-','4','5','6','+','1',	/* 0x48-x04f 89-456+1  */
	'2','3','0','.',0,0,0,0				/* 0x50-0x57 230.<><><><>  */
};

char Keyboard::Shift_Keymap[] = {
	0,0x1b,'!','@','#','$','%','^',		/* 0x00-0x07 0, <esc>,!,@,#,$,%,^, */
	'&','*','(',')','_','+',0x8,0x9,	/* 0x08-0x0f ~,<backspace><tab>*/
	'q','w','e','r','t','y','u','i',	/* 0x10-0x17 qwertyui*/
	'o','p','{','}','\n',0,'a','s', 	/* 0x18-0x1f op[] <enter><ctrl>as */
	'd','f','g','h','j','k','l',':',	/* 0x20-0x27 dfghjkl; */
	'\"','~',0,'|','z','x','c','v',		/* 0x28-0x2f '`<lshift>\zdcv */
	'b','n','m','<','>','?',0,'*', 		/* 0x30-0x37 bnm,./<rshitf><printscr> */
	0,' ',0,0,0,0,0,0, 					/* 0x38-0x3f <alt><space><caps><f1><f2><f3><f4><f5> */
	0,0,0,0,0,0,0,0, 					/* 0x40-0x47 <f0><><><><><numlock><scrlock>7*/
	0,0,'-',0,0,0,'+',0,				/* 0x48-x04f 89-456+1  */
	0,0,0,0x7f/*del*/,0,0,0,0			/* 0x50-0x57 230.<><><><>  */
};

int Keyboard::Mode = 0;



void Keyboard::KeyboardHandler( struct pt_regs* reg, struct pt_context* context )
{
	/***********************************************/
	//for test
	//Diagnose::Write("key pressed !   ");
	/***********************************************/

	unsigned char status = IOPort::InByte(Keyboard::STATUS_PORT);
	int limit = 10;
	static int pre_state = 0;

	while ( (status & Keyboard::DATA_BUFFER_BUSY) && limit-- )
	{
		/* 如果键盘缓存满就要读入剩下的扫描码 */
		unsigned char scancode = IOPort::InByte(Keyboard::DATA_PORT);

		/* 以下的判断过程有点像有限状态机，不过貌似有的地方没有化简 */
		if ( 0 == pre_state )
		{
			if ( 0xE0 == scancode || 0xE1 == scancode )
			{
				/* 将状态改为0xe0表示还有字符没有读入 */
				/* 产生0xe1只有一种状态，那就是pause键被按下，按键按下序列为0xe1,0x1d,0x45，按键断开序列为0xe1,0x9d,0xc5  */
				pre_state = scancode;
			}
			else	/* 非扩展键 */
			{
				pre_state = 0;
				Keyboard::HandleScanCode(scancode, 0);
			}
		}
		else if ( 0xE0 == pre_state )
		{
			/* 扩展键的第二个扫描码 */
			pre_state = 0;
			Keyboard::HandleScanCode(scancode, 0xe0);
		}
		else if ( 0xE1 == pre_state && ( 0x1d == scancode || 0x9d == scancode ) )
		{
			pre_state = 0x100;	/* 中间状态，表示pause已经有两个键吻合 */
		}
		else if ( pre_state == 0x100 && 0x45 == scancode )	/* 只需要知道pause键什么时候按下 */
		{
			pre_state = 0;
			Keyboard::HandleScanCode(scancode, 0xe1);
		}
		else
		{
			pre_state = 0;
		}
		status = IOPort::InByte(Keyboard::STATUS_PORT);
	}
}

void Keyboard::HandleScanCode(unsigned char scanCode, int expand)
{
	int isOK = 0;
	char ch = 0;

	if ( 0xE1 == expand )
	{
		ch = Keyboard::ScanCodeTranslate(scanCode, expand);
	}

	switch ( scanCode )
	{
	case SCAN_ALT:
		if ( 0xE0 == expand )	/* 使用扩展键，表示是右边的 alt按下 */
			Mode |= M_RALT;
		else
			Mode |= M_LALT;
		break;

	case SCAN_CTRL:
		if ( 0xE0 == expand )	/* 使用扩展键，表示是右边的 ctrl按下 */
			Mode |= M_RCTRL;
		else
			Mode |= M_LCTRL;
		break;

	case SCAN_LSHIFT:
		Mode |= M_LSHIFT;
		break;

	case SCAN_RSHIFT:
		Mode |= M_RSHIFT;
		break;

	/* 处理按键被松开的情况，清空Mode中按键对应的标志位 */
	case SCAN_ALT + 0x80:
		if ( 0xE0 == expand )
			Mode &= ~M_RALT;
		else
			Mode &= ~M_LALT;
		break;

	case SCAN_CTRL + 0x80:
		if ( 0xE0 == expand )
			Mode &= ~M_RCTRL;
		else
			Mode &= ~M_LCTRL;
		break;

	case SCAN_LSHIFT + 0x80:
		Mode &= ~M_LSHIFT;
		break;

	case SCAN_RSHIFT + 0x80:
		Mode &= ~M_RSHIFT;
		break;

	/* 每次按下就反转改变状态位 */
	
	/* 
	 * M_DOWN_NUMLOCK宏表示NUMLOCK被按下，在没有按下的
	 * 状态下按下NumLock键需要反转状态，并置numlock键没有松开
	 */
	case SCAN_NUMLOCK:
		isOK = Mode & M_DOWN_NUMLOCK;
		if ( !isOK )
		{
			Mode ^= M_NUMLOCK;
			Mode |= M_DOWN_NUMLOCK;
		}
		break;

	case SCAN_CAPSLOCK:
		isOK = Mode & M_DOWN_CAPSLOCK;
		if ( !isOK )
		{
			Mode ^= M_CAPSLOCK;
			Mode |= M_DOWN_CAPSLOCK;
		}
		break;

	case SCAN_SCRLOCK:
		isOK = Mode & M_DOWN_SCRLOCK;
		if ( !isOK )
		{
			Mode ^= M_SCRLOCK;
			Mode |= M_DOWN_SCRLOCK;
		}
		break;

	/* 释放NumLock，CapsLock，ScrollLock键 */
	case SCAN_NUMLOCK + 0x80:
		Mode &= ~M_DOWN_NUMLOCK;
		break;

	case SCAN_CAPSLOCK + 0x80:
		Mode &= ~M_DOWN_CAPSLOCK;
		break;

	case SCAN_SCRLOCK + 0x80:
		Mode &= ~M_DOWN_SCRLOCK;
		break;

	default:
		ch = Keyboard::ScanCodeTranslate(scanCode, expand);
		break;
	}
	if ( 0 != ch )
	{
		TTy* pTTy = Kernel::Instance().GetDeviceManager().GetCharDevice(DeviceManager::TTYDEV).m_TTy;
		if ( NULL != pTTy )
		{
			pTTy->TTyInput(ch);
		}
	}
}

char Keyboard::
ScanCodeTranslate(unsigned char scanCode, int expand)
{
	char ch = 0;
	bool bReverse = false;

	if ( 0xE1 == expand )	/* Pause Key */
	{
		ch = 0x05;	/* Pause ASCII */
	}
	else if ( scanCode < 0x45 )	/* 非小键盘和控制键 */
	{
		/* 根据扫描码映射表找到对应按键的ASCII码 */
		if ( (Mode & M_LSHIFT) || (Mode & M_RSHIFT) )
		{
			ch = Shift_Keymap[scanCode];
		}
		else
		{
			ch = Keymap[scanCode];
		}

		if ( ch >= 'a' && ch <= 'z' )
		{
			/* 是小写字符而且已经Capslock了，那么转换成大写字符 */
			bReverse = ( (Mode & M_CAPSLOCK) ? 1 : 0 ) ^ ( (Mode & M_LSHIFT) || (Mode & M_RSHIFT) );

			if ( (Mode & M_LCTRL) || (Mode & M_RCTRL) )	/* 按下ctrl进行转意 */
			{
				if('c' ==  ch)  /* ctrl+c --> SIGINT信号，送给除 sched、shell 之外的所有进程 */
				{
					ch = 0;

					/* FLush终端 */
					TTy* pTTy = Kernel::Instance().GetDeviceManager().GetCharDevice(DeviceManager::TTYDEV).m_TTy;
					if ( NULL != pTTy )
					{
						pTTy->FlushTTy();
					}

					ProcessManager& procMgr = Kernel::Instance().GetProcessManager();
					for ( int killed = 0; killed < ProcessManager::NPROC ; killed ++ )
						if ( procMgr.process[killed].p_pid > 1)
							procMgr.process[killed].PSignal(User::SIGINT);
				}
				else
				{
					ch -= 'a';
				    ch++;	/* 转义从0x1 开始*/
				}
			}
			else if ( bReverse )
			{
				ch += 'A' - 'a';
			}
		}
	}
	else if ( scanCode < 0x58 )
	{
		bReverse = ( (Mode & M_NUMLOCK) ? 1 : 0 ) ^ ( (Mode & M_LSHIFT) || (Mode & M_RSHIFT) );

		if ( 0xE0 == expand )
		{
			ch = Shift_Keymap[scanCode];
		}
		else if ( bReverse )
		{
			ch = Keymap[scanCode];
		}
		else
		{
			ch = Shift_Keymap[scanCode];
		}
	}

	return ch;
}

