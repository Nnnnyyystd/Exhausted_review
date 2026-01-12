#include "TestUtility.h"
#include "KernelInclude.h"

void PrintResult(char* casename, bool result)
{
	Diagnose::Write("%s...%s\n", casename, result ? "PASS" : "FAILED");
	if( false == result)
		while(true);
}
