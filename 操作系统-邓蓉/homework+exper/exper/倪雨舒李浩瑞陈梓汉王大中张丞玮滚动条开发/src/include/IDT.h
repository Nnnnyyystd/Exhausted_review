#ifndef	IDT_H
#define	IDT_H

/* 
 * 定义与IDT(中断描述符表)相关的类。
 * 主要是下面这3个class: 
 * 	1. struct GateDescriptor
 *	2. struct IDTR
 * 	3. class IDT
 */

/* 定义了IDT中每一个门描述符的格式 */
struct GateDescriptor
{
	unsigned short	m_Low16BitsOffset;			/*OFFSET的低16位*/
	unsigned short	m_SegmentSelector;			/*段选择子*/
	unsigned char	m_Reserved : 5;				/*保留区域，长5个bit*/
	unsigned char	m_Zero : 3;					/*全零区域*/
	unsigned char	m_Type : 4;					/*描述符类型.  0xE为中断门  0xF为陷入门*/
	unsigned char	m_System : 1;				/*1：系统描述符  0：代码、数据段描述符*/
	unsigned char	m_DPL : 2;					/*描述符访问优先级*/
	unsigned char	m_SegmentPresent : 1;		/*存在标志位*/
	unsigned short	m_High16BitsOffset;			/*OFFSET的高16位*/
}__attribute__((packed));
/* 一点说明：
 * 	__attribute__((packed))是为了防止类的成员变量，由于字节对齐需要而产生多余间隙，
 * 	会导致sizeof(GateDescriptor)大于8 Byte。 如果那样的话，CPU根据[GDTR+ 中断号*8]来
 * 	获取IDT中的Descriptor就会字节错位。(不过这里不用packed属性, 全部比特位之和应该也是8byte)
 */


/* IDT表基地址与限长结构体定义，用于加载到IDTR寄存器 */
struct IDTR
{
	unsigned short	m_Limit;		/* IDT的限长 */
	unsigned int	m_BaseAddress;	/* IDT的起始地址(线性地址) */
}__attribute__((packed));


/* IDT中断描述符表的定义，由256个GateDescriptor组成，总长度为2048字节 */
class IDT
{
	/* static member functions */
public:
	static void DefaultInterruptHandler();		/* 默认中断处理程序 */
	static void DefaultExceptionHandler();		/* 默认异常处理程序 */

public:
	/* 根据中断号、中断/异常处理程序入口地址，设置IDT表中对应的中断门描述符 */
	void SetInterruptGate(int number, unsigned int handler);
	
	/* 根据中断号、中断/异常处理程序入口地址，设置IDT表中对应的陷入门描述符 */
	void SetTrapGate(int number, unsigned int handler);
	
	/* 根据IDT表的起始地址(线性地址)与长度设置GDTR结构体 */
	void FormIDTR(IDTR& idtr);

private:
	GateDescriptor m_Descriptor[256];	/* 256 * 8 Bytes */
};

#endif
