#include <stdio.h>
#include <sys.h>

/*
void SIGBRK_Handler()
{
	printf("child reaped\n");
}


int main1(int argc, char* argv[])
{

	signal(SIGINT, SIGBRK_Handler);
	int ppid = getpid();
	
	int i;
	for(i = 0; i < 3; i++){
		if(fork() == 0){
			printf("Hello from child %d\n", getpid());
			int ans = kill(ppid, SIGINT);
			exit(0);
		}
	}
	
	while(1);
	
	return 0;
}
*/

/*
int main1(int argc, char* argv[])
{

	while(1)
	{
		sleep(5);
		printf("Hello!\n");

	}

	return 0;
}
*/


#include <stdio.h>
#include <sys.h>

char* message = "Stuff this in your pipe and smoke it";

int main1(int argc, char* argv[])
{

	int pid = fork();


		if( pid == 0 )
		{

			while(1)
			{
				printf("Child %s\n",message);

			}
		}
		else
		{
			while(1)
			{
				printf("Father %s\n",message);
				//sleep(2);
			}
		}

	return 1;
}


