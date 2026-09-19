#include "mm\TestAllocator.h"
#include "mm\TestPageManager.h"
#include "mm\TestNew.h"
#include "pe\TestPEParser.h"
#include <stdio.h>


int main()
{
	//TestAllocator();
	//TestPageManager();
	//TestPEParser();
	TestNew();

	printf("Press any key to exit...\n");
	getchar();
	return 0;
}