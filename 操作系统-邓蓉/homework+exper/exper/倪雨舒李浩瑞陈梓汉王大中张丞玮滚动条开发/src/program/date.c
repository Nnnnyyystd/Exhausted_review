#include <stdio.h>
#include <time.h>

int main1(int argc, char* argv[])
{
	unsigned int timeInSeconds;
	struct tm* ptime;
	
	timeInSeconds = gtime();
	ptime = localtime(timeInSeconds);
	
	printf("%s\n", asctime(ptime));
	
	return 1;
}
