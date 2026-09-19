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
	static Allocator& GetInstance();
private:
	static Allocator m_Instance;
};

#endif

