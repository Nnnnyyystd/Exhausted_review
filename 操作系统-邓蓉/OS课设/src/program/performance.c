#include <stdio.h>
#include <sys.h>

int main1(int argc, char *argv[])
{
	int temp,pre,post; 
	struct tms tms_info;
	memset(&tms_info, 0, sizeof(tms_info));
	if(argc < 2)
	{
		printf("Require more argument!\n");
		return 0;
	}
	pre = getswtch();	
    if ( fork() )
    {
        wait(&temp);
		post = getswtch();
		times(&tms_info);
		printf("Performance analysis:\n");
		printf("System time:%d\n", tms_info.stime);
		printf("User time:%d\n", tms_info.utime);
		printf("Child System time: %d\n", tms_info.cstime);
		printf("Child User Time: %d\n", tms_info.cutime);
		printf("Process switch number:%d\n", post-pre);
    }
    else
    {
		execv(argv[1], &argv[1]);
    }
    return 0;
}
