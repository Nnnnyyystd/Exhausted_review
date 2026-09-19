#ifndef ALLOCATOR_H
#define ALLOCATOR_H

#include "MapNode.h"

/* @comment 该类为内存分配算法类，针对使用MapNode
 * 数组标记的情况，可以用在PageManager和SwapDiskManager中
 * 其中函数在Unixv6中对应关系如下：
 * Alloc()	: malloc(mp, size)		@line 2538
 * Free()	: mfree(mp, size, aa)	@line 2556
 */
class Allocator
{
	/* Functions */
public:
	unsigned long Alloc(MapNode map[], unsigned long size);
	unsigned long Free(MapNode map[], unsigned long size, unsigned long addrIdx);

public:
	static Allocator &GetInstance();

private:
	static Allocator m_Instance;
};

// NOTE:1
/* @comment 该类为内存分配算法类，针对使用BitMap
 * 标记的情况，可以用在KernelPageManager and UserPageManager中
 */
class BitMapAllocator
{
	/* Functions */
public:
	unsigned long Alloc(BitMap &bitmap, unsigned long size);
	unsigned long Free(BitMap &bitmap, unsigned long size, unsigned long addrIdx);
	int is_free(BitMap &bitmap, int index);
	void set_bit(BitMap &bitmap, int index, int value);

public:
	static BitMapAllocator &GetInstance();

private:
	static BitMapAllocator m_Instance_bitmap;
};

#endif
