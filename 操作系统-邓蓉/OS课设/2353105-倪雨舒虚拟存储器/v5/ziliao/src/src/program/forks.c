#include <stdio.h>
#include <sys.h>

/*   这个程序是对的，但是shell的命令行提示符会穿插显示在该应用的输出之中。之后，shell工作正常，
 * 但系统就没有提示符了。改变可以如下。
int main1(int argc, char* argv[])
{
	int i;
	for( i = 0; i < 3; i++ )
	{
		fork();
	}

	int pid = getpid();
	sleep(pid%8 + 1);
	printf("PID : %d\n", pid);
	
	return 1;
}
*/

//命令行提示符会在应用执行完毕之后出现。可是上面的程序运行结束之前，屏幕上会出现命令行提示符。
//可完善的系统是不应该因为应用程序没写好，就丢了命令行提示符的。
//shell应该robust!  HOW TO DO?（等待进程组中的所有进程全部结束再唤醒shell）

int main1(int argc, char* argv[])
{
	int i;
	int kid,reCode;

	printf("PID : %d\n", getpid());
	for( i = 0; i < 3; i++ )
	{
		if(fork()==0)
			printf("PID : %d\n", getpid());
		else
			kid = wait(&reCode);
	}

	exit(0);
}
