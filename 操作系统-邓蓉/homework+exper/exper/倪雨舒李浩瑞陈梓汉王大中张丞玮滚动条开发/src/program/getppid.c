#include <stdio.h>
#include <sys.h>

int main1()
{
    int pid  = getpid();
    int ppid = getppid(pid);

    printf("This is Process %d speaking...\n", pid);
    printf("My parent process ID is: %d\n", ppid);
    return 0;
}
