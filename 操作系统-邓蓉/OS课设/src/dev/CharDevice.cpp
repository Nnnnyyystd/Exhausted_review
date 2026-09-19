#include "CharDevice.h"
#include "Utility.h"
#include "Kernel.h"

/*==============================class CharDevice===============================*/
CharDevice::CharDevice()
{
	this->m_TTy = NULL;
}

CharDevice::~CharDevice()
{
	//nothing to do here
}

void CharDevice::Open(short dev, int mode)
{
	Utility::Panic("ERROR! Base Class: CharDevice::Open()!");
}

void CharDevice::Close(short dev, int mode)
{
	Utility::Panic("ERROR! Base Class: CharDevice::Close()!");
}

void CharDevice::Read(short dev)
{
	Utility::Panic("ERROR! Base Class: CharDevice::Read()!");
}

void CharDevice::Write(short dev)
{
	Utility::Panic("ERROR! Base Class: CharDevice::Write()!");
}

void CharDevice::SgTTy(short dev, TTy* pTTy)
{
	Utility::Panic("ERROR! Base Class: CharDevice::SgTTy()!");
}


/*==============================class ConsoleDevice===============================*/
/* 
 * 这里定义派生类ConsoleDevice的对象实例。
 * 该实例对象中override了字符设备基类中
 * Open(), Close(), Read(), Write()等虚函数。
 */
ConsoleDevice g_ConsoleDevice;
extern TTy g_TTy;

ConsoleDevice::ConsoleDevice()
{
	//nothing to do here
}

ConsoleDevice::~ConsoleDevice()
{
	//nothing to do here
}

void ConsoleDevice::Open(short dev, int mode)
{
	short minor = Utility::GetMinor(dev);
	User& u = Kernel::Instance().GetUser();

	if ( minor != 0 )	/* 选择的不是console */
	{
		return;
	}

	if ( NULL == this->m_TTy )
	{
		this->m_TTy = &g_TTy;
	}

	/* 该进程第一次打开这个设备 */
	if ( NULL == u.u_procp->p_ttyp )
	{
		u.u_procp->p_ttyp = this->m_TTy;	
	}

	/* 设置设备初始模式 */
	if ( (this->m_TTy->t_state & TTy::ISOPEN) == 0 )
	{
		this->m_TTy->t_state = TTy::ISOPEN | TTy::CARR_ON;
		this->m_TTy->t_flags = TTy::ECHO;
		this->m_TTy->t_erase = TTy::CERASE;
		this->m_TTy->t_kill = TTy::CKILL;
	}
}

void ConsoleDevice::Close(short dev, int mode)
{
	//nothing to do here
}

void ConsoleDevice::Read(short dev)
{
	short minor = Utility::GetMinor(dev);

	if ( 0 == minor )
	{
		this->m_TTy->TTRead();	/* 判断是否选择了console */
	}
}

void ConsoleDevice::Write(short dev)
{
	short minor = Utility::GetMinor(dev);

	if ( 0 == minor )
	{
		this->m_TTy->TTWrite();	/* 判断是否选择了console */
	}
}

void ConsoleDevice::SgTTy(short dev, TTy *pTTy)
{
}
