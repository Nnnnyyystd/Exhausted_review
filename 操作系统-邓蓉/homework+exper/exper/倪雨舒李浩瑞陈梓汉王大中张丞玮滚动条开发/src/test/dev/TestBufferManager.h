#ifndef TEST_BUFFERMANAGER_H
#define TEST_BUFFERMANAGER_H

#include "..\KernelInclude.h"

/* Utility Functions */
void PrintBuffer(Buf* pBuf);

void ModifyBuffer(Buf* pBuf, int offset);

int CheckSumBuffer(Buf* pBuf);


/* Test Fucntions */
bool BreadTest();

bool RepeatReadTest();

bool WriteTest();

bool TestBufferManager();

bool TestSwap();

#endif
