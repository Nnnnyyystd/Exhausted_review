#ifndef KERNEL_H
#define KERNEL_H

#include "PageManager.h"
#include "ProcessManager.h"
#include "KernelAllocator.h"
#include "User.h"
#include "BufferManager.h"
#include "DeviceManager.h"
#include "FileManager.h"
#include "FileSystem.h"
#include "SwapperManager.h"

/*
 * Kernel类用于封装所有内核相关的全局类实例对象，
 * 例如PageManager, ProcessManager等。
 * 
 * Kernel类在内存中为单体模式，保证内核中封装各内核
 * 模块的对象都只有一个副本。
 */
class Kernel
{
public:
	static const unsigned long USER_ADDRESS = 0x400000 - 0x1000 + 0xc0000000;	/* 0xC03FF000 */
	static const unsigned long USER_PAGE_INDEX = 1023;		/* USER_ADDRESS对应页表项在PageTable中的索引 */

public:
	Kernel();
	~Kernel();
	static Kernel& Instance();
	void Initialize();		/* 该函数完成初始化内核大部分数据结构的初始化 */

	KernelPageManager& GetKernelPageManager();
	UserPageManager& GetUserPageManager();
	ProcessManager& GetProcessManager();
	KernelAllocator& GetKernelAllocator();
	SwapperManager& GetSwapperManager();
	BufferManager& GetBufferManager();
	DeviceManager& GetDeviceManager();
	FileSystem& GetFileSystem();
	FileManager& GetFileManager();
	User& GetUser();		/* 获取当前进程的User结构 */

private:
	void InitMemory();
	void InitProcess();
	void InitBuffer();
	void InitFileSystem();

private:
	static Kernel instance;		/* Kernel单体类实例 */

	KernelPageManager* m_KernelPageManager;
	UserPageManager* m_UserPageManager;
	ProcessManager* m_ProcessManager;
	KernelAllocator* m_KernelAllocator;
	SwapperManager* m_SwapperManager;
	BufferManager* m_BufferManager;
	DeviceManager* m_DeviceManager;
	FileSystem* m_FileSystem;
	FileManager* m_FileManager;
};

#endif
