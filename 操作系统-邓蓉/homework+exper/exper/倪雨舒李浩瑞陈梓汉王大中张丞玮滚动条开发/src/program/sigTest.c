#include <stdio.h>
#include <sys.h>

/*test for ctrl-c*/
int main1(int argc, char* argv[])
{
		printf("Getting into sleep.\n");

		sleep(50);

		printf("Wakeup.\n");

		return 1;
}
