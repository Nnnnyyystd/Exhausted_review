#include <stdio.h>
#include <sys.h>

#define READ 0
#define WRITE 1

char* message = "Stuff this in your pipe and smoke it";

int main1(int argc, char* argv[])
{
	int fd[2];
	int nbytes;
	char recvBuffer[512];
	
	/* create a unnamed pipe and share it between current process and its descendants */
	pipe(fd);
	
	int pid = fork();
	

		if( pid == 0 )	/* child process */
		{
			close (fd[READ]);
			while(1)
			{
				write (fd[WRITE], message, strlen(message)+1);
				sleep(2);
			}
			//close (fd[WRITE]);
		}
		else	/* parent process */
		{
			close (fd[WRITE]);
			while(1)
			{
				nbytes = read( fd[READ], recvBuffer, 512);
				if( nbytes >= 0 ) //Successfully read pipe data
					printf ("Read %d bytes from Pipe: %s\n", nbytes, recvBuffer);
				else
					break;	//get out of while loop
				//close (fd[READ]);
			}
		}


	return 1;
}
