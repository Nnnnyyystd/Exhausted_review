#include <stdio.h>

int version=1;

main1()
{
	int a,b,result;
	a = 1;
	b = 2;
	result = sum(a,b);
	printf("result=%d\n",result);
	printf("&main1   = %p\n", (void*)&main1);
    printf("&sum     = %p\n", (void*)&sum);
    printf("&printf  = %p\n", (void*)&printf);
    printf("&version = %p\n", (void*)&version);
    printf("&a       = %p, &b = %p, &result = %p\n", (void*)&a, (void*)&b, (void*)&result);


}

int sum(var1, var2)
{
	int count;
	version = 2;
	count = var1 + var2;
	return(count);
}
