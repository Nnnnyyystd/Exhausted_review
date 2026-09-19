#ifndef TEST_LIB_H
#define TEST_LIB_H

#define STDIN	0
#define STDOUT	1

bool FileRWTest();

bool SelectTest();

bool ForkTest();

bool ExitWaitTest();

void PrepareExeFile();

bool ExecTest();

bool CopyFileTest(char* srcFile, char* dstFile);

bool TTyQueueTest();

bool ConsoleTest();

bool WorkingDirTest();

#endif
