#include <stdio.h>
#include <sys.h>
#include <v6pp_pids.h>

/* 入口须与 -e _main1 匹配 */
void _main1(void)
{
    struct v6pp_pids p;
    if (get_pids(&p) == 0) {
        printf("get_pids(): pid=%d ppid=%d\n", p.pid, p.ppid);
    } else {
        printf("get_pids() failed\n");
    }
}
