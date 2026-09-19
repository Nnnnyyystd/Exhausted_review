#include <stdio.h>
#include <sys.h>

int main1(int argc, char* argv[])
{
    int pid = getpid();
    printf("loop process started, pid = %d\n", pid);

    while (1)
        sleep(1);

    return 0;
}
