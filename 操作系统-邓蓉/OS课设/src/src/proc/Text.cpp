#include "Text.h"
#include "Kernel.h"

Text::Text()
{
	//nothing to do here
}

Text::~Text()
{
	//nothing to do here
}

void Text::XccDec()
{
	if ( this->x_ccount == 0 )
		return;

	/* 如果x_ccount递减至0，则释放该共享正文段占据的内存。*/
	if ( --this->x_ccount == 0 )
	{
		Kernel::Instance().GetUserPageManager().FreeMemory(this->x_size, this->x_caddr);
	}
}

void Text::XFree()
{
	this->XccDec();
	/* 
	 * 如果引用该共享正文段的进程数为0，进程都已终止
	 * 则没有必要在交换区上保存共享正文段的副本。
	 */
	if ( --this->x_count == 0 )
	{
		Kernel::Instance().GetSwapperManager().FreeSwap(this->x_size, this->x_daddr);
		Kernel::Instance().GetFileManager().m_InodeTable->IPut(this->x_iptr);
		this->x_iptr = NULL;
	}
}
