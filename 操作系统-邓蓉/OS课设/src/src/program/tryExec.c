// tryExec.c
#include <stdio.h>
#include <stdlib.h>
#include <sys.h>

/*
 * 目的：
 * 在虚拟机终端输入 tryExec 时，系统 shell 会 exec 启动这个程序。
 * 这个程序一运行就立刻 execv("myShell", ...)，
 * 把自己“变成”你写的 myShell。
 */

int main1(void)
{
    /* 注意：这里不用 NULL，直接用 0，兼容 V6++ 环境 */
    char *argv[] = { "myShell", 0 };

    /* 把当前进程图像替换成 myShell */
    execv(argv[0], argv);

    /* 能执行到这里说明 execv 失败了 */
    printf("execv myShell failed.\n");
    return 0;
}
