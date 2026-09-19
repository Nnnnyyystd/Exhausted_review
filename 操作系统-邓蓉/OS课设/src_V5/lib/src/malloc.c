#include <sys.h>
#include <malloc.h>
#include <stdio.h>

#define PAGE_SIZE 12288

char *malloc_begin = NULL;
char *malloc_end = NULL;

typedef struct flist {
   unsigned int size;
   struct flist *nlink;
};

struct flist *malloc_head = NULL;

void* malloc(unsigned int size)
{
    if (malloc_begin == NULL)
    {   // 1、首次执行malloc，启用堆
        malloc_begin = sbrk(0);
        malloc_end = sbrk(PAGE_SIZE);  //3页
        malloc_head = malloc_begin;
        malloc_head->size = sizeof(struct flist);
        malloc_head->nlink = NULL;
    }
    if (size == 0)
    {
        return NULL;
    }
    size += sizeof(struct flist);
    size = ((size + 7) >> 3) << 3;
    struct flist* iter = malloc_head;

    // 2、分配动态内存（找一个足够大的空闲片）
    struct flist *temp;
    while( iter->nlink != NULL )
    {   // 搜索链表
        if ((int)(iter->nlink) - iter->size - (int)iter >= size)
        {
            temp = (char *)iter + (iter->size);
            temp->nlink = iter->nlink;
            iter->nlink = temp;
            temp->size = size;
            return (char *)temp + sizeof(struct flist);
        }
        iter = iter->nlink;
    }

    int remain;
L1: remain = malloc_end - iter->size - (int)iter;
    if ( remain >= size)
    {   // 最后一个内存片之后的空间，足够吗？
        temp = (char *)iter + (iter->size);
        temp->nlink = NULL;
        iter->nlink = temp;
        temp->size = size;
        return (char *)temp + sizeof(struct flist);
    }

    // 3、内存分配不成功，执行sbrk系统调用，扩展堆空间
    int expand = size - remain;
    expand = ((expand + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;  // 扩展3页的整数倍
    malloc_end = sbrk(expand);
    goto L1;  // 分配
}

int free(void* addr)  // 释放以addr为首地址的动态内存
{
    char * real_addr = addr - 8;   // 1、定位释放内存片首地址
    struct flist* iter = malloc_head;
    struct flist* last;   // 用来定位释放内存片之前的内存片
    if (addr == 0)
    {
        return -1;
    }

    while(iter)  // 遍历链表
    {
        if (iter == real_addr)   // 2、找到释放的内存片iter
        {
            last->nlink = iter->nlink;  // 把iter从链表里删除
            if (last->nlink == NULL)    // 3、如果被删除的是最后一个内存片
            {
                char *pos = (char *)last + last->size;
                if (malloc_end - pos > PAGE_SIZE * 2)
                {   // 若删除后，堆空间尾部出现尺寸大于6页的空闲片，sbrk缩小堆空间
                    malloc_end = sbrk(-((malloc_end - pos) / PAGE_SIZE * PAGE_SIZE));
                }
            }
            return 0;
        }
        last = iter;   // last和 iter同步后移
        iter = iter->nlink;
    }
    return -1;
}

