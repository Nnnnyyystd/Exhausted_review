#include <stdio.h>
#include <stdlib.h>
#include <sys.h>

int main1(int argc, char* argv[])
{
	int fdSrc = -1;
	int fdDst = -1;
	
	fdSrc = open(argv[1], 0x1FF);
	if ( -1 == fdSrc )
		printf("Cannot Open File: %s\n", argv[1]);
	
	fdDst = creat(argv[2], 0x1FF);
	if ( -1 == fdDst )
		printf("Cannot Open File: %s\n", argv[2]);
		
	if ( -1 == fdSrc || -1 == fdDst )
		return -1;
	
	int rbytes = 0;
	int wbytes = 0;
	char buf[512];
	while ( rbytes = read(fdSrc, buf, 512) )
	{
		if ( rbytes < 0 )
		{
			printf("Read Source File Error...\n");
			return -1;
		}
		else
		{
			wbytes = write(fdDst, buf, rbytes);
			if (wbytes != rbytes)
			{
				printf("Write Target File Error...\n");
				return -1;
			}
		}
	}
	
	close(fdSrc);
	close(fdDst);
	
	return 1;
}
