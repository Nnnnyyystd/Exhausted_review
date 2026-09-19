#include "ATADriver.h"
#include "BufferManager.h"
#include "Utility.h"
#include "IOPort.h"
#include "Kernel.h"
#include "DMA.h"
#include "Chip8259A.h"

void ATADriver::ATAHandler(struct pt_regs *reg, struct pt_context *context)
{
	Buf* bp;
	Devtab* atab;
	short major = Utility::GetMajor(DeviceManager::ROOTDEV);

	BlockDevice& bdev = 
		Kernel::Instance().GetDeviceManager().GetBlockDevice(major);
	atab = bdev.d_tab;
	
	if( atab->d_active == 0 )
	{
		return;		/* 没有请求项 */
	}

	bp = atab->d_actf;		/* 获取本次中断对应的I/O请求Buf */
	atab->d_active = 0;		/* 表示设备已经空闲 */

	/* 检查I/O操作执行过程中磁盘控制器或者DMA控制器是否出错 */
	if( ATADriver::IsError() || DMA::IsError() )
	{
		if(++atab->d_errcnt <= 10)
		{
			bdev.Start();
			return;
		}
		bp->b_flags |= Buf::B_ERROR;
	}
	
	atab->d_errcnt = 0;		/* 错误计数器归零 */
	atab->d_actf = bp->av_forw;		/* 从I/O请求队列中取出已完成的I/O请求Buf */
	Kernel::Instance().GetBufferManager().IODone(bp);	/* I/O结束善后工作 */
	bdev.Start();	/* 启动I/O请求队列中下一个I/O请求 */
	/* 对主、从8259A中断控制芯片分别发送EOI命令。 */
	IOPort::OutByte(Chip8259A::MASTER_IO_PORT_1, Chip8259A::EOI);
	IOPort::OutByte(Chip8259A::SLAVE_IO_PORT_1, Chip8259A::EOI);
	return;
}

void ATADriver::DevStart(struct Buf* bp)
{
	if(bp == NULL)
	{
		Utility::Panic("Invalid Buf in DevStart()!");
	}

	/* 等待控制器较长时间未就绪，表示出错。 */
	if(ATADriver::IsControllerReady() == 0)
	{
		Utility::Panic("Disk Hang Up!");
	}

	short minor = Utility::GetMinor(bp->b_dev);

	/* 构造物理区域描述符表(PRD Table) */
	PhysicalRegionDescriptor prd;
	static PRDTable table;
	
	/* 传入参数为Buf缓冲区的实际物理地址 */
	/* 
	 * 由于进程图像起始地址p_addr所在用户态空间地址小于0xC0000000，
	 * 所以不能采用对待缓存同样的方法：一律线性地址减去0xC0000000算出实际物理地址:
	 * prd.SetBaseAddress((unsigned long)bp->b_addr - 0xC0000000); //Oops~
	 * 而且p_addr中存放的本身就已经是物理地址，所以采取如下方法，有则减之，无则保持。
	 */
	prd.SetBaseAddress((unsigned long)bp->b_addr & ~0xC0000000);
	prd.SetByteCount(bp->b_wcount);

	/* 
	 * 将构造好的prd描述符放到PRD Table的第0项位置，
	 * 并且标记为最后一项。(我们这里仅使用一个prd描述符即可。) 
	 */
	table.SetPhysicalRegionDescriptor(0, prd, true);

	DMA::Reset();		/* 复位DMA控制器 */

	/* 设置扇区数 */
	IOPort::OutByte(ATADriver::NSECTOR_PORT, bp->b_wcount / BufferManager::BUFFER_SIZE);
	/* 设置LBA28寻址模式中磁盘块号的0-7位 */
	IOPort::OutByte(ATADriver::BLKNO_PORT_1, bp->b_blkno & 0xFF);
	/* 设置LBA28寻址模式中磁盘块号的8-15位 */
	IOPort::OutByte(ATADriver::BLKNO_PORT_2, (bp->b_blkno >> 8) & 0xFF);
	/* 设置LBA28寻址模式中磁盘块号的16-23位 */
	IOPort::OutByte(ATADriver::BLKNO_PORT_3, (bp->b_blkno >> 16) & 0xFF);
	/* 设置ATA磁盘工作模式寄存器，以及LBA28中的24-27位 */
	IOPort::OutByte(ATADriver::MODE_PORT, ATADriver::MODE_IDE | ATADriver::MODE_LBA28 | (minor << 4) | ((bp->b_blkno >> 24) & 0x0F) );

	/* 如果是读操作 */
	if( (bp->b_flags & Buf::B_READ) == Buf::B_READ )
	{
		/* 告诉磁盘控制器的读、写类型，启动I/O */
		IOPort::OutByte(ATADriver::CMD_PORT, ATADriver::HD_DMA_READ);
		
		/* 告诉DMA控制器的读、写类型，传入PRD Table的物理起始地址，启动一次DMA */
		DMA::Start(DMA::READ, table.GetPRDTableBaseAddress());
	}
	else	/* 如果是写操作 */
	{
		IOPort::OutByte(ATADriver::CMD_PORT, ATADriver::HD_DMA_WRITE);
		
		DMA::Start(DMA::WRITE, table.GetPRDTableBaseAddress());
	}
	return;
}

int ATADriver::IsControllerReady()
{
	int ticks = 10000;
	
	while(--ticks)
	{
		unsigned char status = IOPort::InByte(ATADriver::STATUS_PORT);
		if( (status & (ATADriver::HD_DEVICE_BUSY | ATADriver::HD_DEVICE_READY)) == ATADriver::HD_DEVICE_READY )
		{
			return ticks;
		}
	}
	return 0;	/* 控制器长时间无响应 */
}

bool ATADriver::IsError()
{
	unsigned char status = IOPort::InByte(ATADriver::STATUS_PORT);
	if( (status & ATADriver::HD_ERROR) == ATADriver::HD_ERROR )
	{
		return true;	/* 出错 */
	}
	return false;	/* 没有出错 */
}
