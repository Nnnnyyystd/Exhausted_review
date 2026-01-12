#include "PEParser.h"
#include "..\TestUtility.h"
#include "stdio.h"
#include "stdlib.h"

bool TestPEParser()
{
	//Setup
	FILE* fp = fopen("E:\\projects\\oos\\svn\\trunk\\src\\program\\a.exe", "rb");
	void* buffer = malloc(1024*256);
	int cnt = fread(buffer, sizeof(unsigned char), 1024*256, fp);
	
	printf("Filesize is %d\n", cnt);

	//TestCases;
	PEParser parser((unsigned long)buffer);
	parser.Parse();

	//Tear Down
	free(buffer);
	fclose(fp);

	return true;
}