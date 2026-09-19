#ifndef TEST_INCLUDE_H
#define TEST_INCLUDE_H

/*
此处列出所有用于测试的*.h头文件，
提供给内核源代码(如main.cpp)统一的Include接口，
而不必到处include test/Testxx.h。
*/

/* dev */
#include "dev\TestBufferManager.h"


/* fs */
#include "fs\TestFileSystem.h"


/* lib */
#include "lib\Lib.h"
#include "lib\TestLib.h"

/* mm */
#include "mm\TestSwapperManager.h"

#endif
