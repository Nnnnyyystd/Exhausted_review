#include "Kernel.h"
#include "Machine.h"
#include "New.h"
#include "Video.h"

Kernel Kernel::instance;

/* 
 * 内存管理相关的全局manager
 */
UserPageManager g_UserPageManager(&(Allocator::GetInstance()));
KernelPageManager g_KernelPageManager(&(Allocator::GetInstance()));
KernelAllocator g_KernelAllocator(&(Allocator::GetInstance()));

/*
 * 交换区相关全局manager
 */
SwapperManager g_SwapperManager(&(Allocator::GetInstance()));

/* 
 * 进程相关全局manager
 */
ProcessManager g_ProcessManager;

/*
 * 设备管理、高速缓存管理全局manager
 */
BufferManager g_BufferManager;
DeviceManager g_DeviceManager;

/*
 * 文件系统相关全局manager
 */
FileSystem g_FileSystem;
FileManager g_FileManager;

Kernel::Kernel()
{
}

Kernel::~Kernel()
{
}

Kernel& Kernel::Instance()
{
	return Kernel::instance;
}

void Kernel::InitMemory()
{
	this->m_KernelPageManager = &g_KernelPageManager;
	this->m_UserPageManager = &g_UserPageManager;
	
	Diagnose::Write("Initilize Memory...");
	this->GetKernelPageManager().Initialize();
	this->GetUserPageManager().Initialize();
	Diagnose::Write("Ok.\n");

	this->m_KernelAllocator = &g_KernelAllocator;

	Diagnose::Write("Initilize KernelAllocator...");
	this->GetKernelAllocator().Initialize();
	Diagnose::Write("Ok.\n");

	/* 设置new/delete operator需要使用的Allocator */
	set_kernel_allocator(this->m_KernelAllocator);

	this->m_SwapperManager = &g_SwapperManager;
	Diagnose::Write("Initialize Swapper...");
	this->GetSwapperManager().Initialize();
	Diagnose::Write("Ok.\n");

}

void Kernel::InitProcess()
{
	this->m_ProcessManager = &g_ProcessManager;

	Diagnose::Write("Initilize Process...");
	this->GetProcessManager().Initialize();
	Diagnose::Write("Ok.\n");
}

void Kernel::InitBuffer()
{
	this->m_BufferManager = &g_BufferManager;
	this->m_DeviceManager = &g_DeviceManager;

	Diagnose::Write("Initialize Buffer...");
	this->GetBufferManager().Initialize();
	Diagnose::Write("OK.\n");

	Diagnose::Write("Initialize Device Manager...");
	this->GetDeviceManager().Initialize();
	Diagnose::Write("OK.\n");
}

void Kernel::InitFileSystem()
{
	this->m_FileSystem = &g_FileSystem;
	this->m_FileManager = &g_FileManager;

	Diagnose::Write("Initialize File System...");
	this->GetFileSystem().Initialize();
	Diagnose::Write("OK.\n");

	Diagnose::Write("Initialize File Manager...");
	this->GetFileManager().Initialize();
	Diagnose::Write("OK.\n");
}

void Kernel::Initialize()
{
	InitMemory();
	InitProcess();
	InitBuffer();
	InitFileSystem();
}

KernelPageManager& Kernel::GetKernelPageManager()
{
	return *(this->m_KernelPageManager);
}

UserPageManager& Kernel::GetUserPageManager()
{
	return *(this->m_UserPageManager);
}

ProcessManager& Kernel::GetProcessManager()
{
	return *(this->m_ProcessManager);
}

KernelAllocator& Kernel::GetKernelAllocator()
{
	return *(this->m_KernelAllocator);
}

SwapperManager& Kernel::GetSwapperManager()
{
	return *(this->m_SwapperManager);
}

BufferManager& Kernel::GetBufferManager()
{
	return *(this->m_BufferManager);
}

DeviceManager& Kernel::GetDeviceManager()
{
	return *(this->m_DeviceManager);
}

FileSystem& Kernel::GetFileSystem()
{
	return *(this->m_FileSystem);
}

FileManager& Kernel::GetFileManager()
{
	return *(this->m_FileManager);
}

User& Kernel::GetUser()
{
	return *(User*)USER_ADDRESS;
}
