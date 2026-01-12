#ifndef ATA_DRIVE_H
#define ATA_DRIVE_H

#include "Regs.h"

class ATADriver
{
public:
	/* 磁盘中断设备处理子程序 */
	static void ATAHandler(struct pt_regs* reg, struct pt_context* context);

	/* 设置磁盘寄存器，启动磁盘进行I/O操作 */
	static void DevStart(struct Buf* bp);

private:
	/* 检查控制器是否就绪，返回值非零表示就绪，才可以发送命令 */
	static int IsControllerReady();

	/* 检查I/O过程中是否有错误发生 */
	static bool IsError();

	// /* 目前未实现DMA，需要此函数。写操作(读操作不需要此函数)前需要
	//  * 确定硬盘缓冲区是否准备好接受数据。顺序：磁盘缓冲区就绪 -> 写入
	//  * 磁盘缓冲区 -> 最终写入目标扇区 -> I/O完成后发出中断。
	//  */
	// static int ReadyToTransfer();

	/* static const member */
public:
	/* 磁盘寄存器界面I/O端口地址 */
	static const unsigned short DATA_PORT	=	0x1f0;	/* 数据寄存器端口 */
	static const unsigned short ERROR_PORT	=	0x1f1;	/* 错误寄存器端口 */
	static const unsigned short NSECTOR_PORT =	0x1f2;	/* 扇区数寄存器端口 */
	static const unsigned short BLKNO_PORT_1 =	0x1f3;	/* 扇区号寄存器端口(1)，存放LBA28寻址模式中磁盘块号的0-7位 */
	static const unsigned short BLKNO_PORT_2 =	0x1f4;	/* 扇区号寄存器端口(2)，存放LBA28寻址模式中磁盘块号的8-15位 */
	static const unsigned short BLKNO_PORT_3 =	0x1f5;	/* 扇区号寄存器端口(3)，存放LBA28寻址模式中磁盘块号的16-23位 */
	static const unsigned short MODE_PORT	=	0x1f6;	/* BIT(0-3)：扇区号寄存器端口(4)，存放LBA28寻址模式中磁盘块号的24-27位 
														   BIT(4-7)：ATA磁盘工作模式，Bit(7和5)为1表示是IDE接口，Bit(6)为1表示开启LBA28模式，Bit(4)为0表示主盘。*/
	static const unsigned short CMD_PORT	=	0x1f7;	/* 只写：命令寄存器端口 */
	static const unsigned short STATUS_PORT	=	0x1f7;	/* 只读：主状态寄存器端口 */
	static const unsigned short CTRL_PORT	=	0x3f6;	/* 控制寄存器端口：主要用于控制磁盘I/O操作完成后是否发送中断 */
	
	/* 主状态寄存器(只读)比特位定义 (端口号：0x1f7) */
	static const unsigned char HD_ERROR	= 0x01;			/* 执行最近一次磁盘操作过程中发生错误 */
	static const unsigned char HD_DEVICE_REQUEST = 0x08;	/* 控制器已经准备好从磁盘缓存区发送和接收数据 */
	static const unsigned char HD_DEVICE_FAULT = 0x20;	/* 驱动器发生写故障 */
	static const unsigned char HD_DEVICE_READY = 0x40;	/* 磁盘控制器已经准备就绪，可以发送读、写命令 */
	static const unsigned char HD_DEVICE_BUSY =	0x80;	/* 驱动器忙，正在执行命令。 */
	
	/* 命令寄存器(只写)比特位定义 (端口号：0x1f7) */
	static const unsigned char HD_READ	= 0x20;	/* 磁盘读命令，如遇到错误允许重试 */
	static const unsigned char HD_WRITE	= 0x30;	/* 磁盘写命令，如遇到错误允许重试 */
	static const unsigned char HD_DMA_READ = 0xC8;	/* DMA方式读磁盘命令 */
	static const unsigned char HD_DMA_WRITE = 0xCA;	/* DMA方式写磁盘命令 */
	
	/* 工作模式寄存器比特位定义 (端口号： 0x1f6) */
	static const unsigned char MODE_IDE = 0xA0;		/* ATA磁盘工作模式，Bit(7和5)为1表示是IDE接口 */
	static const unsigned char MODE_LBA28 = 0x40;	/* ATA磁盘工作模式，Bit(6)为1表示开启LBA28模式 */
};

#endif
