#ifndef KERNEL_INCLUDE_H
#define KERNEL_INCLUDE_H

/* 
此处列出了Testxx.cpp中需要Include内核中
声明的类或全局对象，提供给test下面的测试代码
Include内核的接口。
*/

#include "..\include\Kernel.h"
#include "..\include\Video.h"
#include "..\include\TimeInterrupt.h"
#include "..\include\CRT.h"

extern "C" void Delay();

#endif

