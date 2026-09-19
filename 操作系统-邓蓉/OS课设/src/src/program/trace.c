#include <stdio.h>
#include <sys.h>

int main1(int argc, char* argv[])
{
	int ans = trace(10); /* lines for debug output */
	
	if( -1 == ans )
		printf("trace system call failed!\n");
	else if ( 0 == ans )
		printf("trace OFF\n");
	else /* ans > 0 */
		printf("trace ON\n");
		
	return 1;
}
