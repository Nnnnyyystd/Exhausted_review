#include <stdio.h>
#include <sys.h>

/* 简易 atoi */
int my_atoi(const char *s)
{
    int x = 0, sign = 1;
    if (*s == '-') {
        sign = -1;
        s++;
    }
    while (*s >= '0' && *s <= '9') {
        x = x * 10 + (*s - '0');
        s++;
    }
    return x * sign;
}

int main1(int argc, char* argv[])
{
    int sig, pid;

    if (argc != 3) {
        printf("usage: kill -signal pid\n");
        return -1;
    }

    /* 获取信号号，例如 -9 => 9 */
    if (argv[1][0] == '-')
        sig = my_atoi(argv[1] + 1);
    else
        sig = my_atoi(argv[1]);

    pid = my_atoi(argv[2]);

    printf("send signal %d to pid %d\n", sig, pid);
    printf("kill successfully\n");

    if (kill(pid, sig) < 0) {
        printf("kill failed\n");
        return -1;
    }

    return 0;
}
