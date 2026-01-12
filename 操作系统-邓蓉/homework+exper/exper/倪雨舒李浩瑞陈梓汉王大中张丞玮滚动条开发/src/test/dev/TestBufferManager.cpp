#include "TestBufferManager.h"
#include "..\KernelInclude.h"

void PrintBuffer(Buf* pBuf)
{
	for(int i = 0; i < BufferManager::BUFFER_SIZE; i++)
	{
		Diagnose::Write("%x ", pBuf->b_addr[i]);
		if(i % 100 == 0) Delay();
	}
}

void ModifyBuffer(Buf* pBuf, int offset)
{
	for(int i = 0; i < BufferManager::BUFFER_SIZE; i++)
	{
		pBuf->b_addr[i] = 'a' + offset + i % 26;
	}
}

int CheckSumBuffer(Buf* pBuf)
{
	int sum = 0;
	for(int i = 0; i < BufferManager::BUFFER_SIZE; i++)
	{
		sum += pBuf->b_addr[i];
	}
	Diagnose::Write("CheckSum Buf addr of %x = %x\n", pBuf->b_addr, sum);
	return sum;
}

bool BreadTest()
{
	Diagnose::Write("Start Test Bread...\n");
	Buf* pBuf;

	BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
	pBuf = bufMgr.Bread(DeviceManager::ROOTDEV, 0);
	CheckSumBuffer(pBuf);

	/* 一定要Brelse() */
	bufMgr.Brelse(pBuf);

	pBuf = bufMgr.Bread(DeviceManager::ROOTDEV, 0);
	Diagnose::Write("Read same block second time!\n");
	CheckSumBuffer(pBuf);

	return true;
}

bool RepeatReadTest()
{
	Diagnose::Write("Repeated Read Test Start...\n");
	Buf* pBuf;
	unsigned long addr;
	/* 15个缓存循环利用，读取多个字符块，但别超过c.img的扇区数20,160 Sectors */
	int repeat = 3000;

	BufferManager& bufMgr = Kernel::Instance().GetBufferManager();

	pBuf = bufMgr.Bread(DeviceManager::ROOTDEV, 0);
	/* 一定要Brelse() */
	bufMgr.Brelse(pBuf);
	/* 记录1st buffer的地址 */
	addr = (unsigned long)pBuf->b_addr;

	int nbuffer = 0;
	for( int blkno = 0; blkno < repeat; blkno++)
	{
		pBuf = bufMgr.Bread(DeviceManager::ROOTDEV, blkno);
		//CheckSumBuffer(pBuf);
		nbuffer = ( (unsigned long)pBuf->b_addr - addr )/BufferManager::BUFFER_SIZE;
		Diagnose::Write("Using Buffer[%d]...\n", nbuffer);
		//Delay();
		bufMgr.Brelse(pBuf);
	}

	if( nbuffer == (repeat - 1) % BufferManager::NBUF)
	{
		return true;
	}
	else
	{
		Diagnose::Write("Test Failed!\n");
		while(1);
	}
}


bool WriteTest()
{
	/* read 0# sector, modify it and write back */
	Diagnose::Write("Repeated Read Test Start...\n");
	Buf* pBuf;

	BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
	
	pBuf = bufMgr.Bread(DeviceManager::ROOTDEV, 0);

	ModifyBuffer(pBuf, 0);
	int checksum = CheckSumBuffer(pBuf);
	Diagnose::Write("%s\n", pBuf->b_addr);

	bufMgr.Bwrite(pBuf);
	Diagnose::Write("Write Done!\n");	/* Go and check the c.img */

	return true;
}

bool TestBufferManager()
{
	/* read 1# sector,  modify it and write to 2#, read 2# sector for check */
	Diagnose::Write("Start Test Buffer Manager...\n");
	Buf* pBuf;
	
	BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
	pBuf = bufMgr.Bread(DeviceManager::ROOTDEV, 1);

	pBuf->b_blkno = 2;
	ModifyBuffer(pBuf, 1);
	int checksum = CheckSumBuffer(pBuf);

	bufMgr.Bawrite(pBuf);
	Diagnose::Write("Bwrite Done!\n");
	
	/* 
	Violate the buf, force the following Bread() to conduct a real I/O,  
	rather than fetch data from the exist buf!!
	*/
	pBuf->b_dev = -1;

	pBuf = bufMgr.Bread(DeviceManager::ROOTDEV, 2);
	if( CheckSumBuffer(pBuf) == checksum )
	{
		Delay();
		return true;
	}
	else
	{
		Diagnose::Write("Test Failed...\n");
		while (1);
	}
}

bool TestSwap()
{
	BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
	int count = SwapperManager::BLOCK_SIZE * 2;
	char swapBuf[1024];

	/* 将1#，2#这两个磁盘块读入swapBuf，相当于进程图像换入，因为还没有可用于测试的进程图像 */
	if( bufMgr.Swap(1, (unsigned long)swapBuf, count, Buf::B_READ) == false )
	{
		while(true);
	}

	/* 分配交换区空间，并将进程图像“换出”到交换区中，运行结果使用UltraEdit打开c.img查看 */
	int blkno = Kernel::Instance().GetSwapperManager().AllocSwap(count);
	
	if( bufMgr.Swap(blkno, (unsigned long)swapBuf, count, Buf::B_WRITE) == false )
	{
		while(true);
	}

	return true;
	/* 注意：目前用于测试Swap()函数的进程图像首地址swapBuf是位于核心态地址空间，即大于0xC0000000，
	 * 真正进程图像换入换出时，用户态地址空间从线性地址0开始，而ATADriver::DevStart()函数中，在
	 * 物理区域描述符表(PRD Table)填入的是实际物理地址，统一进行Buf - 0xC0000000的换算。这操作对于指向
	 * 进程图像起始地址的swbuf会是灾难性的，后面需要对其进行修改。
	 */
}

