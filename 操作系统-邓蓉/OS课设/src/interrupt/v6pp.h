#ifndef V6PP_PUBLIC_H_
#define V6PP_PUBLIC_H_

/* 必须与内核 Utility.h 里的 v6pp_procinfo 完全一致 */
struct v6pp_procinfo
{
    unsigned long v_text_start;
    unsigned long v_text_size;
    unsigned long v_data_start;
    unsigned long v_data_size;
    unsigned long v_stack_size;

    unsigned long p_text_paddr;
    int           p_swap_daddr;
};

#endif
