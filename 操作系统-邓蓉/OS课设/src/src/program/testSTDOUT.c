#include <stdio.h>
#include <stdlib.h>

int main1(int argc, char* argv[])
{
	int i;
	
	for( i = 0; i < 10; i++)
	{

		printf("%d", i);

	}
	
	sleep(10);

	printf("\n");

	exit(0);
}
