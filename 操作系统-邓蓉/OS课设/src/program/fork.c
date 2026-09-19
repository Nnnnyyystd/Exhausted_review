#include <stdio.h>
#include <stdlib.h>

int main1(int argc, char* argv[])
{
	int pid = getpid();
	
	printf("My PID is %d\n",pid);

	exit(0);
}
