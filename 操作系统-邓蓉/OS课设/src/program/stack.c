#include <stdio.h>
#include <stdlib.h>
#include <sys.h>

void test(int n)
{
	int a[100];
	int i;
	if (n > 0) {
		for (i = 0; i < 100; i++)
			a[i] = i+1;
		printf("%d\n",n);
		test(n-1);
	}
	else {
		printf("0\n");
	}
}

int main1(int argc, char* argv[])
{
	printf("%d\n",sizeof(int));

	test(100);

	printf("over\n");
	return 1;
}


