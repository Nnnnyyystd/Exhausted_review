#ifndef GDT_H
#define GDT_H

struct SegmentDescriptor
{
	unsigned short	m_Low16BitsSegmentLimit : 16;
	unsigned short	m_Low16BitsBaseAddress : 16;
	unsigned char	m_Mid8BitsBaseAddress : 8;
	unsigned char	m_Type : 4;
	unsigned char	m_System : 1;
	unsigned char	m_DPL : 2;
	unsigned char	m_SegmentPresent : 1;
	unsigned char	m_High4BitsSegmentLimit : 4;
	unsigned char	m_Available : 1;
	unsigned char	m_Reserved : 1;
	unsigned char	m_DefaultOperationSize : 1;
	unsigned char	m_Granularity : 1;
	unsigned char	m_High8BitsBaseAddress : 8;

public:
	void SetBaseAddress(unsigned int baseAddress);
	void SetSegmentLimit(unsigned int segmentLimit);

}__attribute__((packed));


struct GDTR
{
	unsigned short	m_Limit : 16;
	unsigned int	m_BaseAddress : 32;
}__attribute__((packed));


class GDT
{
public:
	/* 获取idx相应的段描述符的引用 */
	SegmentDescriptor& GetSegmentDescriptor(int index);
	
	/* 设置idx相应的段描述符 */
	void SetSegmentDescriptor(int index, SegmentDescriptor& segmentDescriptor);
	
	/* 根据GDT表的起始地址(线性地址)与长度设置GDTR结构体 */
	void FormGDTR(GDTR& gdtr);
	
private:
	SegmentDescriptor m_Descriptors[256];	/* 256 * 8 Bytes */
};

#endif
