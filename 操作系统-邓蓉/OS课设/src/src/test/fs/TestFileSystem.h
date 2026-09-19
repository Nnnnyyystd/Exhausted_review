#ifndef TEST_FILE_SYSTEM_H
#define TEST_FILE_SYSTEM_H

#include "..\KernelInclude.h"

void PrintInode(char* InodeName, Inode* pInode);

extern SuperBlock spb;

void LoadSuperBlock();

void MakeFS();

void InitTTyInode();

bool AllocAllBlock();

bool IAllocTest();

bool NameIandMakNodeTest();

bool NameITest();

bool SetupDirTree();

bool TestFileSystem();

#endif
