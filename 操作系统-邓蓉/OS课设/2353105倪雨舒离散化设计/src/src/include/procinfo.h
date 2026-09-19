#ifndef PROCINFO_H_
#define PROCINFO_H_

/* 操作系统内核与用户程序共享的“进程信息”结构 */
struct proc_info {
    /* 虚拟空间（来自 User::u_MemoryDescriptor） */
    unsigned long v_text_start;  /* 代码段起始虚拟地址 */
    unsigned long v_text_size;   /* 代码段长度 */
    unsigned long v_data_start;  /* 数据段起始虚拟地址 */
    unsigned long v_data_size;   /* 数据段长度 */
    unsigned long v_stack_size;  /* 栈段长度 */

    /* 物理/交换（来自 PCB 文本段） */
    unsigned long p_text_paddr;  /* 文本段“物理”起始（x_caddr） */
    int           p_swap_daddr;  /* 文本段交换区起始块号（x_daddr） */
};

#endif
