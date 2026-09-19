#ifndef TEXT_H
#define TEXT_H

#include "INode.h"

/*
 * @comment 对应Unixv6中 struct text结构
 * 描述可执行代码正文段(code segment)的信息
 *
 *
 */
class Text
{
public:
	Text();
	~Text();

	/* 递减x_ccount的值，如果x_ccount递减至0，
	 * 表示内存中已经没有引用该共享正文段的进程，
	 * 则释放该共享正文段占据的内存。
	 */
	void XccDec();

	/*
	 * 进程释放其引用的共享正文段，通常发生在Exit()或Exec()时候。
	 */
	void XFree();

public:
	int				x_daddr;	/* 代码正文段在盘交换区上的地址 */
	unsigned long	x_caddr;	/* 代码正文段在物理内存中的起始地址，以字节为单位 */
	unsigned int	x_size;		/* 代码段长度，以字节为单位 */
	Inode*			x_iptr;		/* 内存inode地址 */
	unsigned short	x_count;	/* 共享正文段的进程数 */
	unsigned short	x_ccount;	/* 共享该正文段且图像在内存的进程数 */	
};

#endif

