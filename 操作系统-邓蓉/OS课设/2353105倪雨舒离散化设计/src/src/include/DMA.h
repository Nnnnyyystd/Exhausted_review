#ifndef DMA_H
#define DMA_H

/* 
 * Physical Region Descriptor(PRD) 物理内存区描述符
 * 用于描述物理内存与外设之间进行DMA方式数据传输时，
 * 源(或目标)物理内存区域起始地址、 长度信息的数据结构。
 */
class PhysicalRegionDescriptor
{
	/* Members */
private:
	unsigned long m_BaseAddressZeroBit : 1;	/*  内存区域 "物理起始地址"(注：不是线性地址)的第0位，该位必须为0，意味着内存缓冲区必须2字节对齐 */
	unsigned long m_MemoryRegionPhysicalBaseAddress : 31;	/* 内存区域 "物理起始地址"的第[31 : 1]位 */
	unsigned short m_ByteCountZeroBit : 1;			/* DMA传输字节数Byte Count的第0位，恒为0， 意味着每次传输的字节数不可以是奇数个字节长度 */
	unsigned short m_ByteCount : 15;				/* DMA传输字节数Byte Count的第[15 : 1]位 */
	unsigned short m_Reserved : 15;					/* 保留区域 */
	unsigned short m_EOT : 1;						/* Bit(31)：End of Table位。该位为1表示当前物理内存区描述符(PRD)是PRD表中的最后一项。 */
	
public:
	void SetBaseAddress(unsigned long phyBaseAddr);
	void SetByteCount(unsigned short bytes);
	void SetEndOfTable(bool EOT /* End of Table */ );
	
}__attribute__((packed));


/* 
 * 物理内存区描述符(PRD)表，一个或多个PRD可以构成描述符表，
 * 每个表项描述一块用于DMA传输的物理内存区域，由此可以描述
 * 物理内存上不连续的多个区域用于同一次DMA传输。
 * 
 * 表中最后一项物理内存区描述符的Bit(31)为1表示PRD Table的结束，
 * 每启动一次DMA传输时，DMA控制芯片从PRD Table的第0项开始，依次
 * 读/写表中每一个PRD描述的内存区域，直至DMA芯片检测到表中第n个PRD
 * 的Bit(31)为1，则认为PRD Table结束，才算完成DMA传输。
 * 
 * 注: PRD Table中相邻两个描述符，它们所描述的物理内存区可以是不连续
 * 的，但是这两个描述符自身必须在内存上是连续的，因此PRDTable中
 * 使用数组的形式来实现描述符表。
 */
class PRDTable
{
public:
	/* static const */
	static const int NSIZE = 10;		/* PRD Table中描述符的最大允许数目 */

	/* Members */
public:
	/* 设置index相应的物理内存区描述符(PRD) */
	void SetPhysicalRegionDescriptor(int index, PhysicalRegionDescriptor& prd, bool EOT /* End of Table */);
	
	/* 获取PRD Table的物理起始地址 (注：返回的是物理地址，而不是线性地址) */
	unsigned long GetPRDTableBaseAddress();
	
private:
	/* 
	 * PRD Table的起始地址必须4字节对齐，这是由于DMA控制芯片内部的
	 * 物理区域描述符表寄存器(PRDTR)，该寄存器存放PRD Table的Base Address中
	 * 的[31 : 2]位,而忽略掉[1 : 0]两位，因此需要4字节对齐。
	 */
	PhysicalRegionDescriptor m_Descriptors[NSIZE] __attribute__((aligned (4)));
};


/* 
 * DMA类封装了DMA控制芯片的内部寄存器的端口号，
 * 包括命令寄存器，状态寄存器，以及物理区域
 * 描述符表寄存器(PRDTR)的端口号。
 * 
 * 同时还对这些寄存器中特定比特位定义相应的常量。
 */
class DMA
{
public:
	/* static member */
	static unsigned short COMMAND_PORT;		/* 命令寄存器的端口号 */
	static unsigned short STATUS_PORT;		/* 状态寄存器的端口号 */
	static unsigned short PRDTR_PORT;		/* PRD Table基地址寄存器的端口号 */

	/* 命令寄存器 (端口号：COMMAND_PORT) 比特位定义 */
	enum DMAStart	/* 命令寄存器Bit(0)，Start/Stop位 */
	{
		START	=	0x01,		/* Bit(0) = 1；启动一次DMA */
		STOP	=	0x00		/* Bit(0) = 0；停止正在执行的DMA；
								在执行完前一次DMA之后也需要用软件指令将Bit(0)置为0 */
	};

	enum DMAType	/* 命令寄存器Bit(3), Read/Write位；告诉DMA控制芯片进行DMA传输的方向 */
	{
		READ	=	0x08,		/* DMA Read，Bit(3) = 1；表示读硬盘，写入内存 */
		WRITE	=	0x00		/* DMA Write，Bit(3) = 0；表示写硬盘，读内存 */
	};
	/* 命令寄存器 (端口号：COMMAND_PORT) 8比特中只有Bit(0)和Bit(3)为有效位。 */


	/* 状态寄存器 (端口号：STATUS_PORT) 比特位定义： 只用到8个比特位中的Bit(0 - 2) */
	static const unsigned char ACTIVE = 0x01;	/* Acitve位Bit(0)在DMA操作执行期间硬件置1，
												DMA完成后由硬件自动置0。*/
												
	static const unsigned char ERROR =	0x02;	/* Error位Bit(1)：DMA执行过程中如果出错，硬件将该位置1，
												软件指令通过写入一个’1‘将该比特位复位到0。
												通过写’1‘来清零，有点奇怪～～ 但是绝对有效！*/
												
	static const unsigned char Interrupt = 0x04; /* Interrupt位Bit(2)：当DMA传输完成，
												并且外设已向CPU发出中断请求(不论CPU是否关中断)之后，
												由硬件置为1，表示已经发出中断请求。 
												软件指令通过写入一个’1‘将该比特位复位到0。*/

public:
	static void Init();			/* 初始化DMA芯片，确定DMA控制芯片内部寄存器所占据的I/O端口号 */

	static void Reset();		/* 重设DMA控制芯片，清除前一次DMA传输的结果状态 */
	
	static bool IsError();		/* 检查前一次DMA执行过程中是否出错 */

	/* 根据参数规定的DMA类型、PRD Table的起始物理地址，启动DMA操作 */
	static void Start(enum DMAType type, unsigned long baseAddress);
};

#endif
