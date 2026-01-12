#include <stdio.h>
#include <stdlib.h>

int main1(int argc, char* argv[])
{
	int i;
	char line[100];
	
	for( i = 1; i < argc; i++)
	{
		printf("%s ", argv[i]);
	}
	
	printf("\n");

	printf("main1: 0x%x\n",&main1);

	unsigned long a = (unsigned long)0xC03ff000;
	printf("a = 0x%x\n", (unsigned long)a);

	exit(0);
}
