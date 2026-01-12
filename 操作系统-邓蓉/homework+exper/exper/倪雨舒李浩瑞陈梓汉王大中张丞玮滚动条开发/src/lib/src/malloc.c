#include <sys.h>          // 提供 sbrk 等系统调用（你环境中的头）
#include <malloc.h>       // 这里包含了标准 malloc/free 的声明（本文件会覆盖同名实现）
#include <stdio.h>

#define PAGE_SIZE 12288   // 每次向堆区扩展的粒度（这里是 3 * 4096）

char *malloc_begin = NULL;  // 堆区起始地址（首次调用时由 sbrk(0) 得到）
char *malloc_end   = NULL;  // 当前堆区“末端地址”（已向内核申请到的位置）

typedef struct flist {
   unsigned int size;      // 本块（含头部）的大小，按 8 字节对齐
   struct flist *nlink;    // 单向链表的下一个块
} flist;

struct flist *malloc_head = NULL;  // 空闲/已分配块的头指针（这里既当哨兵又当首块）

void* malloc(unsigned int size)
{
    if (malloc_begin == NULL)                // 第一次调用 malloc 时做初始化
    {
        malloc_begin = sbrk(0);              // 取当前“程序断点”作为堆起点
        malloc_end   = sbrk(PAGE_SIZE);      // 向内核向上扩 3 页，返回新的“断点”
        malloc_head  = (struct flist*)malloc_begin; // 头块放在堆起点
        malloc_head->size  = sizeof(struct flist);  // 头块大小先设为仅包含头
        malloc_head->nlink = NULL;                  // 目前链表只有这一块
    }

    if (size == 0)            // 申请 0 字节按约定返回 NULL
        return NULL;

    size += sizeof(struct flist);            // 真实占用 = 用户数据 + 头部
    size = ((size + 7) >> 3) << 3;           // 向上按 8 字节对齐
    struct flist* iter = malloc_head;        // 从头块开始遍历

    // ---------- 在已存在的相邻两块之间找是否有“足够大的缝隙” ----------
    struct flist *temp;
    while (iter->nlink != NULL)              // 只要后面还有块，就检查间隙
    {
        // 计算：后继块地址 - 当前块尾地址（= 当前块首址 + size）
        if ((int)(iter->nlink) - iter->size - (int)iter >= (int)size)
        {   // 若间隙 >= 需求，说明可以把新块安插到这个间隙里
            temp = (struct flist*)((char *)iter + iter->size); // 新块起址＝当前块尾
            temp->nlink = iter->nlink;       // 新块接到当前块与后继块之间
            iter->nlink = temp;
            temp->size  = size;              // 记录新块大小
            return (char *)temp + sizeof(struct flist);  // 返回用户区地址
        }
        iter = iter->nlink;                  // 否则继续看下一对间隙
    }

    // ---------- 遍历到最后一块：尝试用“尾部剩余空间” ----------
    int remain;
L1:
    remain = (int)(malloc_end - ( (char*)iter + iter->size )); // 末端到最后块尾的空余
    if (remain >= (int)size)
    {   // 尾部空余足够，直接在堆尾追加一个块
        temp = (struct flist*)((char *)iter + iter->size); // 新块起址
        temp->nlink = NULL;                 // 新块成为新的表尾
        iter->nlink = temp;
        temp->size  = size;
        return (char *)temp + sizeof(struct flist); // 返回用户区地址
    }

    // ---------- 尾部不够：向系统申请扩展堆空间 ----------
    int expand = (int)size - remain;        // 需要额外的最小字节数
    // 按 PAGE_SIZE 的整数倍扩展（向上取整）
    expand = ((expand + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;
    malloc_end = sbrk(expand);              // sbrk 返回扩展前的“旧断点”
    goto L1;                                // 扩展后再检查一次尾部是否够
}

int free(void* addr)  // 释放以 addr 为用户首地址的块
{
    char * real_addr = (char*)addr - 8; // 取块头地址：用户地址 - 头部大小(假设=8)
    struct flist* iter = malloc_head;   // 从头遍历链表
    struct flist* last;                 // 记录 iter 的前驱块

    if (addr == 0)                      // 传入空指针，按约定返回错误
        return -1;

    while (iter)                        // 遍历寻找“恰好等于该块头地址”的节点
    {
        if ((char*)iter == real_addr)   // 找到要释放的块
        {
            last->nlink = iter->nlink;  // 从链表中摘掉该块（简单删除，不做合并）

            if (last->nlink == NULL)    // 如果被删的是“最后一块”
            {
                char *pos = (char *)last + last->size; // 删除后新的“最后块尾”
                // 若堆末到最后块尾的空闲 > 2*PAGE_SIZE（这里=两页）
                if (malloc_end - pos > PAGE_SIZE * 2)
                {   // 把多余的整页归还给系统（按页对齐回收）
                    int giveback = ((malloc_end - pos) / PAGE_SIZE) * PAGE_SIZE;
                    malloc_end = sbrk(-giveback); // 通过负值收缩断点
                }
            }
            return 0;                  // 释放成功
        }
        last = iter;                   // 前驱前移
        iter = iter->nlink;            // 继续遍历
    }
    return -1;                         // 未找到对应块 => 失败
}
