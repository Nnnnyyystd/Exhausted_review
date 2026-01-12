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

	unsigned char status = IOPort::InByte(Keyboard::STATUS_PORT);
	int limit = 10;
	static int pre_state = 0;

	while ( (status & Keyboard::DATA_BUFFER_BUSY) && limit-- )
	{
		/* 键盘缓冲区中还有剩余的扫描码 */
		unsigned char scancode = IOPort::InByte(Keyboard::DATA_PORT);

		/* 如果之前的状态为0，表示没有处于扩展码状态 */
		if ( 0 == pre_state )
		{
			if ( 0xE0 == scancode || 0xE1 == scancode )
			{
				/* 扫描码为0xe0或0xe1，表示是扩展码的开始 */
				/* 0xe1只有一种情况，就是pause键，按下序列为0xe1,0x1d,0x45，松开序列为0xe1,0x9d,0xc5 */
				pre_state = scancode;
			}
			else	/* 普通扫描码 */
			{
				pre_state = 0;
				Keyboard::HandleScanCode(scancode, 0);
			}
		}
		else if ( 0xE0 == pre_state )
		{
			/* 扩展码0xe0的第二个扫描码 */
			pre_state = 0;
			Keyboard::HandleScanCode(scancode, 0xe0);
		}
		else if ( 0xE1 == pre_state && ( 0x1d == scancode || 0x9d == scancode ) )
		{
			pre_state = 0x100;	/* 中间状态，表示pause已经收到前两个码 */
		}
		else if ( pre_state == 0x100 && 0x45 == scancode )	/* 只要知道pause什么时候按下 */
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

	
	bool diagHandled = false;

	switch ( scanCode )
	{
	case SCAN_ALT:
		if ( 0xE0 == expand )	
			Mode |= M_RALT;
		else
			Mode |= M_LALT;
		break;

	case SCAN_CTRL:
		if ( 0xE0 == expand )	
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

	/* 扫描码+0x80，清除Mode中对应的标志位 */
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

	/* 每次按键都翻转的标志位 */
	
	/* 
	 * M_DOWN_NUMLOCK 表示NUMLOCK键按下，还没有松开的
	 * 状态。因为NumLock键要翻转状态，只有numlock键没有按下
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

	/* 松开NumLock, CapsLock, ScrollLock键 */
	case SCAN_NUMLOCK + 0x80:
		Mode &= ~M_DOWN_NUMLOCK;
		break;

	case SCAN_CAPSLOCK + 0x80:
		Mode &= ~M_DOWN_CAPSLOCK;
		break;

	case SCAN_SCRLOCK + 0x80:
		Mode &= ~M_DOWN_SCRLOCK;
		break;

	case SCAN_PAGEUP:
		CRT::ScrollUp();
		//CRT::WriteChar('U');
		break;

	case SCAN_PAGEDOWN:
		CRT::ScrollDown();
		break;

	case SCAN_LEFT:
		ch = TTy::KEY_LEFT;
		break;
	case SCAN_RIGHT:
		ch = TTy::KEY_RIGHT;
		break;
	case SCAN_UP:
		ch = TTy::KEY_UP;
		break;
	case SCAN_DOWN:
		ch = TTy::KEY_DOWN;
		break;
	case SCAN_DELETE:
		if ( 0xE0 == expand )
			ch = TTy::CDEL;
		else
			ch = Keyboard::ScanCodeTranslate(scanCode, expand);
		break;

	/* ===== 新增：Ctrl+W / Ctrl+S 控制 Diagnose（下半屏）滚动 ===== */
	case SCAN_W:
		if ( (Mode & (M_LCTRL | M_RCTRL)) != 0 ) {
			Diagnose::ScrollUp();
			diagHandled = true;   // 早返回，避免后续把视图拉回最新
			break;
		} else {
			ch = Keyboard::ScanCodeTranslate(scanCode, expand);
		}
		break;

	case SCAN_S:
		if ( (Mode & (M_LCTRL | M_RCTRL)) != 0 ) {
			Diagnose::ScrollDown();
			diagHandled = true;   // 早返回，避免后续把视图拉回最新
			break;
		} else {
			ch = Keyboard::ScanCodeTranslate(scanCode, expand);
		}
		break;

	default:
		ch = Keyboard::ScanCodeTranslate(scanCode, expand);
		break;
	}

	// === 新增：Ctrl+W/S 已处理则直接返回，避免将输入路径把视图复位 ===
	if (diagHandled) {
		return;
	}

	if ( 0 != ch )
	{
		CRT::GoToLatestLine();
		Diagnose::GoToLatestLine();

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
	else if ( scanCode < 0x45 )	/* 小键盘区和功能键区 */
	{
		/* 根据扫描码映射为对应的ASCII码 */
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
			/* 如果小写字母，且已经Capslock了，那么转换为大写字母 */
			bReverse = ( (Mode & M_CAPSLOCK) ? 1 : 0 ) ^ ( (Mode & M_LSHIFT) || (Mode & M_RSHIFT) );

			if ( (Mode & M_LCTRL) || (Mode & M_RCTRL) )	/* 如果ctrl键按下，转换 */
			{
				if('c' ==  ch)  /* ctrl+c --> SIGINT 信号，透传给sched，shell之类会进行处理 */
				{
					ch = 0;

					/* FLush 终端 */
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
				    ch++;	/* 转换为0x1开始 */
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
