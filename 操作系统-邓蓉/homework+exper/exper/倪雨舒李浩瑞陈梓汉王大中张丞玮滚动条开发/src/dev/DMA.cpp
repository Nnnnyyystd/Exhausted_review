#include "DMA.h"
#include "IOPort.h"
#include "Utility.h"

/*============================== class PhysicalRegionDescriptor ===============================*/
void PhysicalRegionDescriptor::SetBaseAddress(unsigned long phyBaseAddr)
{
	/* 取物理地址的[31 : 1]位写入PRD相应字段 */
	this->m_MemoryRegionPhysicalBaseAddress = phyBaseAddr >> 1;
	/* 将物理地址字段的Bit(0)置零，起始地址必须双字节对齐 */
	this->m_BaseAddressZeroBit = 0;
}

void PhysicalRegionDescriptor::SetByteCount(unsigned short bytes)
{
	/* 将DMA传输字节数的第[15 : 1]位写入PRD相应字段 */
	this->m_ByteCount = bytes >> 1;
	/* DMA传输字节数必须是偶数 */
	this->m_ByteCountZeroBit = 0;
}

void PhysicalRegionDescriptor::SetEndOfTable(bool EOT)
{
	/* 设置EOT位 */
	this->m_EOT = EOT;
}


/*============================== class PRDTable ===============================*/
void PRDTable::SetPhysicalRegionDescriptor(int index, PhysicalRegionDescriptor &prd, bool EOT)
{
	this->m_Descriptors[index] = prd;
	this->m_Descriptors[index].SetEndOfTable(EOT);
}

unsigned long PRDTable::GetPRDTableBaseAddress()
{
	/* 需要返回的是物理地址，物理地址 = 线性地址 - 0xC0000000 */
	return (unsigned long)this->m_Descriptors - 0xC0000000;
}


/*============================== class DMA ===============================*/
/* static member */
unsigned short DMA::COMMAND_PORT = 0;
unsigned short DMA::STATUS_PORT = 0;
unsigned short DMA::PRDTR_PORT = 0;

void DMA::Init()
{
	/* 
	 * 枚举整个PCI设备配置空间，查找挂载于PCI总线上的IDE设备。
	 * 从而获取并初始化用于访问该设备DMA控制器的端口号。
	 */

	/* 系统中最多允许PCI总线256根,
	 * 每根PCI总线上最多允许32个PCI设备，每个PCI物理设备最多包括8个功能号(逻辑设备)。*/
	int BUS = 256, DEV = 32, FUNC = 8;

	/* PCI设备配置空间地址寄存器、数据寄存器端口。邓蓉的理解，每个PCI设备都有 */
	short cfg_addr_port = 0xCF8, cfg_data_port = 0xCFC;
	
	int i, j, k;
	int value;
	unsigned int config[64];	/* 每个功能号包括256字节的配置信息 */
	
	for(i = 0; i < BUS; i++)	/* 枚举PCI总线 */
	{
		for(j = 0; j < DEV; j++)	/* 枚举每根总线上的物理设备 */
		{
			for(k = 0; k < FUNC; k++)	/* 枚举每根总线上的功能号(逻辑设备) */
			{
				bool find = 0;
				for(int index = 0; index < 64; index++)
				{
					unsigned int ans;
					
					/* 构造要读取的配置空间地址，并输出到cfg_addr_port。每个PCI设备编址有固定的规则  */
					value = (0x80000000 | (i << 16) | (j << 11) | (k << 8) | (index << 2));
					IOPort::OutDWord(cfg_addr_port, value);
					
					/* 读入相应的配置空间内容 */
					ans = IOPort::InDWord(cfg_data_port);
					config[index] = ans;
					
					if(ans == 0xFFFFFFFF) 
						continue;
						
					/* 
					 * index == 2表示读入的是256字节配置空间中的0x08 - 0x0B字节，
					 * 判断第0x0B,0x0A两字节是否都为0x01。
					 * 第0x0B字节为设备基类型，该字节等于0x01表示当前设备为大容量存储设备。
					 * 第0x0A字节为设备子类型，该字节等于0x01表示当前设备为IDE接口。
					 */
					if( (ans & 0x01010000) == 0x01010000 && index == 2)
					{
						find = 1;
					}
				}
				
				if( find == 1 )
				{
					/* 到这里已经读取了配置空间的全部256字节，取出第0x20 - 0x23字节 */
					if(config[8] & 0x1)		/* Bit(0)为1表示端口地址为分离的I/O地址空间 */
					{
						unsigned short port_addr = config[8] & 0xFFFE;	/* 除去Bit(0)即为端口基地址 */
						DMA::COMMAND_PORT = port_addr;
						DMA::STATUS_PORT = port_addr + 2;
						DMA::PRDTR_PORT = port_addr + 4;
					}
					else	/* 否则为内存映射I/O地址空间 */
					{
						Utility::Panic("Error: Unsupported Memory Mapped I/O");
					}
					return;
				}
			}
		}
	}
	return;
}

void DMA::Reset()
{
	/* 停止以前的DMA传输 */
	IOPort::OutByte(DMA::COMMAND_PORT, DMA::STOP);
	/* 清除DMA状态寄存器中的Error和Interrupt位 */
	IOPort::OutByte(DMA::STATUS_PORT, DMA::Interrupt | DMA::ERROR);
	return;
}

bool DMA::IsError()
{
	unsigned char value = IOPort::InByte(DMA::STATUS_PORT);
	if( (value & DMA::ERROR) == DMA::ERROR )
	{
		return true;	/* 出错 */
	}
	return false;	/* 没有出错 */
}

void DMA::Start(enum DMAType type, unsigned long baseAddress)
{
	/* 将PRD Table的物理起始地址写入PRDTR寄存器 */
	IOPort::OutDWord(DMA::PRDTR_PORT, baseAddress);
	/* 根据读、写类型启动一次DMA */
	IOPort::OutByte(DMA::COMMAND_PORT, type | DMA::START);
	return;
}
