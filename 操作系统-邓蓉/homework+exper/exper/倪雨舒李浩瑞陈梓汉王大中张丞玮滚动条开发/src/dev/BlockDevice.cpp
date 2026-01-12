#include "BlockDevice.h"
#include "Kernel.h"
#include "ATADriver.h"

/*==============================class Devtab===================================*/
/* 定义块设备表devtab的实例。为系统中ATA硬盘设置一个块设备表。*/
Devtab g_Atab;

Devtab::Devtab()
{
	this->d_active = 0;
	this->d_errcnt = 0;
	this->b_forw = NULL;
	this->b_back = NULL;
	this->d_actf = NULL;
	this->d_actl = NULL;
}

Devtab::~Devtab()
{
	//nothing to do here
}

/*==============================class BlockDevice===============================*/
BlockDevice::BlockDevice()
{
	//nothing to do here
}

BlockDevice::BlockDevice(Devtab* pDevtab)
{	
	this->d_tab = pDevtab;
	if(this->d_tab != NULL)
	{
		this->d_tab->b_forw = (Buf *)this->d_tab;
		this->d_tab->b_back = (Buf *)this->d_tab;
	}
}

BlockDevice::~BlockDevice()
{
	//nothing to do here
}

int BlockDevice::Open(short dev, int mode)
{
	Utility::Panic("ERROR! Base Class: BlockDevice::Open()!");
	return 0;	/* GCC likes it ! */
}

int BlockDevice::Close(short dev, int mode)
{
	Utility::Panic("ERROR! Base Class: BlockDevice::Close()!");
	return 0;	/* GCC likes it ! */
}

int BlockDevice::Strategy(Buf *bp)
{
	Utility::Panic("ERROR! Base Class: BlockDevice::Strategy()!");
	return 0;	/* GCC likes it ! */
}

void BlockDevice::Start()
{
	Utility::Panic("ERROR! Base Class: BlockDevice::Start()!");
}

/*=============================class ATABlockDevice=============================*/
/* 定义派生类ATABlockDevice的实例。
 * 该实例对象中override块设备基类BlockDevice中Open(), Close(), Strategy()虚函数。
 */
ATABlockDevice g_ATADevice(&g_Atab);

/* 
 * !hard code!挂载于bochs下磁盘映像文件c.img的扇区数大小，应该在系统启动时获取。
 * 20 Cylinders; 16 Heads; 63 Sectors/Track，20 * 16 * 63 = 20160 Sectors。
 * c.img: 10,321,920 bytes = 10,008KB 
 */
int ATABlockDevice::NSECTOR = 0x7FFFFFFF;	/* Max Possible Sector Numbers */

ATABlockDevice::ATABlockDevice(Devtab* pDevtab)
	:BlockDevice(pDevtab)
{
}

ATABlockDevice::~ATABlockDevice()
{
	//nothing to do here
}

int ATABlockDevice::Open(short dev, int mode)
{
	return 0;	/* GCC likes it ! */
}

int ATABlockDevice::Close(short dev, int mode)
{
	return 0;	/* GCC likes it ! */
}

int ATABlockDevice::Strategy(Buf* bp)
{
	/* 检查I/O操作块是否超出了该硬盘的扇区数上限 */
	if(bp->b_blkno >= ATABlockDevice::NSECTOR)
	{
		/* 设置出错标志 */
		bp->b_flags |= Buf::B_ERROR;

		BufferManager& bm = Kernel::Instance().GetBufferManager();
		/* 
		 * 出错情况下不真正执行I/O操作，这里相当于模拟磁盘中断
		 * 处理程序中调用IODone()唤醒等待I/O操作结束的进程。
		 */
		bm.IODone(bp);
		return 0;	/* GCC likes it ! */
	}

	/* 将bp加入I/O请求队列的队尾，此时I/O队列已经退化到单链表形式，将bp->av_forw标志着链表结尾 */
	bp->av_forw = NULL;

	/* 以下操作将进入临界区，其中临界资源为块设备表g_Atab。
	 * 因为除了这里会对块设备表g_Atab的I/O请求队列进行操作，
	 * 磁盘中断处理程序也会对I/O请求队列其进行操作，两者是并行的。
	 * 实际上这里只需关闭硬盘的中断就可以了。
	 */
	X86Assembly::CLI();
	if(this->d_tab->d_actf == NULL)
	{
		this->d_tab->d_actf = bp;
	}
	else
	{
		this->d_tab->d_actl->av_forw = bp;
	}
	this->d_tab->d_actl = bp;

	/* 如果硬盘不忙就立即进行硬盘操作，否则将当前I/O请求送入
	 * 块设备表I/O请求队列之后直接返回，当前磁盘的I/O操作完成后
	 * 会向CPU发出磁盘中断，系统会在磁盘中断处理程序中执行块设备
	 * 表I/O请求队列中的下一个I/O请求。
	 */
	if(this->d_tab->d_active == 0)		/* 磁盘空闲 */
	{
		this->Start();
	}
	X86Assembly::STI();

	return 0;	/* GCC likes it ! */
}

void ATABlockDevice::Start()
{
	Buf* bp;

	if( (bp = this->d_tab->d_actf) == 0 )
		return;		/* 如果I/O请求队列为空，则立即返回 */

	this->d_tab->d_active++;	/* I/O请求队列不空，设置控制器忙标志 */

	/* 设置磁盘寄存器，启动I/O操作 */
	ATADriver::DevStart(bp);
}
