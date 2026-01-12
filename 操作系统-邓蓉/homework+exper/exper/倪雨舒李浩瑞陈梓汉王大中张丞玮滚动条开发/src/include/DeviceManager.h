#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H

#include "BlockDevice.h"
#include "CharDevice.h"

class DeviceManager
{
	/* static const member */
public:
	static const int MAX_DEVICE_NUM = 10;	/* 系统允许最大块设备数量 */
	static const int NODEV = -1;	/* NODEV设备号 */

	static const short ROOTDEV = (0 << 8) | 0;	/* 磁盘的主、从设备号都为0 */
	static const short TTYDEV = (0 << 8) | 0;	/* TTY终端字符设备的主、从设备号都为0 */

public:
	DeviceManager();
	~DeviceManager();

	/* 初始化块设备基类指针数组。相当于是对块设备开关表bdevsw的初始化。*/
	void Initialize();

	int GetNBlkDev();							/* 获取系统中实际块设备数量nblkdev */
	BlockDevice& GetBlockDevice(short major);	/* 根据主设备号major获取相应块设备对象实例 */
	int GetNChrDev();							/* 获取系统中实际字符设备数量nchrdev */
	CharDevice& GetCharDevice(short major);		/* 根据主设备号major获取相应字符设备对象实例 */
	
private:
	int nblkdev;							/* 系统中块设备的数量 @line 4631 */
	BlockDevice *bdevsw[MAX_DEVICE_NUM];	/* 指向块设备基类的指针数组，相当于Unix V6中块设备开关表 */

	int nchrdev;							/* 系统中字符设备的数量 */
	CharDevice	*cdevsw[MAX_DEVICE_NUM];	/* 指向字符设备基类的指针数组，相当于字符设备开关表 */
};

#endif
