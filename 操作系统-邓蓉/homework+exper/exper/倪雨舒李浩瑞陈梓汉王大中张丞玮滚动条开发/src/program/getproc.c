#include <stdio.h>
#include <sys.h>
#include <v6pp.h>

/* 入口名要与链接参数 -e _main1 匹配 */
void _main1(void)
{
    struct v6pp_procinfo pi;
    if (get_proc(&pi) == 0) {
        printf("VIRT: text=[0x%lx,%lu], data=[0x%lx,%lu], stack=%lu\n",
               pi.v_text_start, pi.v_text_size,
               pi.v_data_start, pi.v_data_size,
               pi.v_stack_size);
        printf("PHYS: text_paddr=0x%lx, swap_daddr=%d\n",
               pi.p_text_paddr, pi.p_swap_daddr);
    } else {
        printf("get_proc failed\n");
    }
}
