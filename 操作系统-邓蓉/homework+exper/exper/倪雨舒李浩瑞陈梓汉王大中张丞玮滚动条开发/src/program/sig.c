#include <stdio.h>
#include <sys.h>

void SIGUSR2_Handler()
{
	printf("Child: \nThis is #SIGUSR2 signal handler!\n");
}

void divide_err_handler()
{
	printf("Divide Error handler...\n");
}

int main1(int argc, char* argv[])
{
	signal(SIGUSR2, SIGUSR2_Handler);
	int pid = fork();
	if( pid == 0 )
	{
		sleep(50);
		
		printf("Signal from parent received.\n");
		
		exit(0x88);
	}
	else
	{
		sleep(1);
		printf("Parent: Send #SIGUSR2 to child.\n");
		int ans = kill(pid, SIGUSR2);
		if (ans == -1)
			printf("#SIGBRK Sent failed.\n");
		
		int status;
		ans = wait(&status);
		printf("Parent : child process exit [%d],[%d]\n", status, ans);
	}
	
	return 1;
}
