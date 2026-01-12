#include "TestFileSystem.h"
#include "..\KernelInclude.h"
#include "..\TestUtility.h"

SuperBlock spb;

void PrintInode(char* InodeName, Inode* pInode)
{
	if ( NULL == pInode)
	{
		return;
	}
	Diagnose::Write("Inode INFO:  %s\n", InodeName);
	Diagnose::Write("Print Inode: dev = %d, No. [%d] On Block [%d], flag = %x, i_size = %d\n",pInode->i_dev, pInode->i_number % 8, FileSystem::INODE_ZONE_START_SECTOR + pInode->i_number / 8, pInode->i_flag, pInode->i_size);
	Diagnose::Write("addr = %x, number = %d, count = %d, nlink = %d, mode = %x\n", pInode, pInode->i_number, pInode->i_count, pInode->i_nlink, pInode->i_mode);
}

void LoadSuperBlock()
{
	User& u = Kernel::Instance().GetUser();
	BufferManager& bufMgr = Kernel::Instance().GetBufferManager();
	FileSystem& fileSys = Kernel::Instance().GetFileSystem();
	Buf* pBuf;

	for (int i = 0; i < 2; i++)
	{
		int* p = (int *)&spb + i * 128;

		pBuf = bufMgr.Bread(DeviceManager::ROOTDEV, FileSystem::SUPER_BLOCK_SECTOR_NUMBER + i);

		Utility::DWordCopy((int *)pBuf->b_addr, p, 128);

		bufMgr.Brelse(pBuf);
	}

	if (u.u_error)
	{
		Utility::Panic("Load SuperBlock Error....!\n");
	}

	fileSys.m_Mount[0].m_dev = DeviceManager::ROOTDEV;
	fileSys.m_Mount[0].m_spb = &spb;

	spb.s_flock = 0;
	spb.s_ilock = 0;
	spb.s_time = Time::time;

	return;
}

void MakeFS()
{
	/* 说明:
	 * c.img扇区区域划分，常量定义在FileSystem类中，扇区号 0 ~ 20,159 。
	 * (0 - 99) (100, 101) (102 - 1023) (1024 - 20,159)
	 * (boot & kernel) (SuperBlock) (DiskInode Zone) (Data Zone)
	 */

	FileSystem& filesys = Kernel::Instance().GetFileSystem();
	
	/* initialize spb and mount[0] */
	filesys.m_Mount[0].m_dev = DeviceManager::ROOTDEV;
	filesys.m_Mount[0].m_spb = &spb;
	spb.s_flock = 0;
	spb.s_ilock = 0;
	spb.s_ronly = 0;
	spb.s_isize = FileSystem::INODE_ZONE_SIZE;
	spb.s_fsize = 20160;
	/* write some feature bytes of Superblock */
	spb.s_time = 0xAABBCCDD;
	spb.padding[46] = 0x473C2B1A;
	
	/* 
	 * 对数据区( 1024 <= blkno < 18000 )中每个扇区，Free(dev, blkno)一下，
	 * 即可将所有free block按照"栈的栈"方式组织起来。
	 */
	for(int blkno = FileSystem::DATA_ZONE_END_SECTOR; blkno >= FileSystem::DATA_ZONE_START_SECTOR; --blkno)
	{
		filesys.Free(DeviceManager::ROOTDEV, blkno);
		//if(spb.s_nfree == 1) {Delay();Delay();}
	}

	/* Init spb.s_inode[]; */
	int total_inode = FileSystem::INODE_ZONE_SIZE * FileSystem::INODE_NUMBER_PER_SECTOR;
	
	spb.s_ninode = 0;
	for(int inode_num = total_inode - 1, count = 31; count > 0; --count )
	{
		spb.s_inode[spb.s_ninode++] = inode_num;
		inode_num--;
	}
	/* 0# DiskInode permanently for root dir */
	spb.s_inode[spb.s_ninode++] = 0;

	/****************** 将0# DiskInode分配做根目录的DiskInode ******************/
    Inode* pNode = filesys.IAlloc(DeviceManager::ROOTDEV);

    User& u = Kernel::Instance().GetUser();
    pNode->i_flag |= (Inode::IACC | Inode::IUPD);
	pNode->i_mode = Inode::IALLOC | Inode::IFDIR /* Most vital!! */| Inode::IREAD | Inode::IWRITE | Inode::IEXEC | (Inode::IREAD >> 3) | (Inode::IWRITE >> 3) | (Inode::IEXEC >> 3) | (Inode::IREAD >> 6) | (Inode::IWRITE >> 6) | (Inode::IEXEC >> 6);
    pNode->i_nlink = 1;
    pNode->i_uid = u.u_uid;
    pNode->i_gid = u.u_gid;

    g_InodeTable.IPut(pNode);	/* 将rootDir DiskInode写入磁盘 */
	/****************** 将0# DiskInode分配做根目录的DiskInode ******************/
	
	/* 将SuperBlock写到c.img中去 */
	spb.s_fmod = 1;
	filesys.Update();
	
	return;
}

void InitTTyInode()
{
	User& u = Kernel::Instance().GetUser();
	FileManager& fileMgr = Kernel::Instance().GetFileManager();
	Inode* pInode;
	
	u.u_dirp = "/tty1";
	pInode = fileMgr.NameI(FileManager::NextChar, FileManager::CREATE);
	if (u.u_error != User::NOERROR)
	{
		Diagnose::Write("user error = %d \n", u.u_error);
		while(true);
	}
	/* tty1本身就不存在，NameI显然返回NULL，调用MakNode创建tty1 */
	if ( NULL == pInode )
	{
		pInode = fileMgr.MakNode(0x1FF);	/* rwxrwxrwx */
		if(NULL == pInode)
		{
			Diagnose::Write("Create /tty1 failed!\n");
			while(true);
		}
		else
		{
			/* Modify tty1's Inode attribute */
			pInode->i_flag |= (Inode::IACC | Inode::IUPD);
			pInode->i_mode = Inode::IALLOC | Inode::IFCHR /* Most vital!! */| Inode::IREAD | Inode::IWRITE | Inode::IEXEC | (Inode::IREAD >> 3) | (Inode::IWRITE >> 3) | (Inode::IEXEC >> 3) | (Inode::IREAD >> 6) | (Inode::IWRITE >> 6) | (Inode::IEXEC >> 6);
			pInode->i_addr[0] = DeviceManager::TTYDEV;

			PrintInode("rootDir", u.u_pdir);
			/* Print tty1 Inode */
			PrintInode("tty1", pInode);

			pInode->Prele(); /* IPut(pInode)效果等同 */

			Diagnose::Write("Create /file1 succeed! \n");
		}
	}/* 至此，创建tty1成功! */

	Kernel::Instance().GetFileSystem().Update();
}

/* 相对于MakeFS()的逆过程，但本函数不将superblock写回c.img，仅为测试目的。 */
bool AllocAllBlock()
{
	FileSystem& filesys = Kernel::Instance().GetFileSystem();
	BufferManager& bufMgr = Kernel::Instance().GetBufferManager();

	Buf* pBuf;
	/* Empty all the free block */
	for(int i = 0; i < FileSystem::DATA_ZONE_SIZE; i++)
	{
		pBuf = filesys.Alloc(DeviceManager::ROOTDEV);
		Diagnose::Write("blkno Allocated = %d\n", pBuf->b_blkno);
		
		/* 
		 * 由于Alloc()会调用GetBlk()，如果不Brelse()，
		 * 会很快将NBUF个Buf耗尽... :(
		 */
		bufMgr.Brelse(pBuf);

		if( i + FileSystem::DATA_ZONE_START_SECTOR != pBuf->b_blkno)
		{
			/* 分配到的字符块号(or扇区号)应该是1024, 1025, 1026, ... , 20,159这样。 */
			Diagnose::Write("Test Failed in AllocAllBlock()!\n");
			while(1);
			return false;
		}
	}
	return true;
}


bool IAllocTest()
{
	Inode* pNode;
	FileSystem& filesys = Kernel::Instance().GetFileSystem();

	/* IAlloc()执行10次，大于先前初始化spb直接管理的DiskInode数量count个，
	 * IAlloc()会重新收集满100个Inode，并且不会将已IAlloc()出去的0# DiskInode
	 * 收集进来。
	 */
	for( int i = 0; i < 10; i++ )
	{
		if( pNode = filesys.IAlloc(DeviceManager::ROOTDEV))
		{
			Diagnose::Write("Inode [%d] allocated.. No.[%d] on Block [%d] !\n", pNode->i_number, pNode->i_number % 8, FileSystem::INODE_ZONE_START_SECTOR + pNode->i_number / 8 );
			Diagnose::Write("Inode.count = %d, Inode.nlink = %d, i_dev = %d, i_number = %d\n", pNode->i_count, pNode->i_nlink, pNode->i_dev, pNode->i_number);
		}
		Delay();
	}
	
	/* 从这一行输出可以看出已分配出去的0# Inode确实没有被IAlloc()进spb.s_Inode[] */
	Diagnose::Write("spb.s_Inode[0] = %d, spb.s_Inode[%d] = [%d]\n", spb.s_inode[0], spb.s_ninode-1, spb.s_inode[spb.s_ninode-1]);
	Delay();
	
	filesys.Update();
	return true;
}

bool NameIandMakNodeTest()
{
	User& u = Kernel::Instance().GetUser();
	FileManager& fileMgr = Kernel::Instance().GetFileManager();
	Inode* pInode;
	int parentDirSize = 0;

	/* u.u_dirp指向要搜索的路径char* pathname，对于open(),creat()系统调用
	 * u.u_dirp值在SystemCall::Trap()中初始化，这里写Test case需要手工进行，
	 * 以CREATE方式进行NameI()。
	 */

	//Case 1: Open non-exist file --> Create non-exist file --> Open exist file
	char* filePath1 = "/testfile1";	
	u.u_dirp = filePath1;
	u.u_error = User::NOERROR;
	/* 查找不存在的/testfile1，NameI()返回NULL */
	pInode = fileMgr.NameI(FileManager::NextChar, FileManager::OPEN);
	PrintResult(
		"NameITest 1-1",
		User::ENOENT == u.u_error && NULL == pInode
		);
	
	u.u_dirp = filePath1;
	u.u_error = User::NOERROR;
	pInode = fileMgr.NameI(FileManager::NextChar, FileManager::CREATE);
	PrintResult(
		"NameITest 1-2",
		User::NOERROR == u.u_error && NULL == pInode
		);
	/* MakNode()后必须解锁/testfile1的Inode,否则以后NameI()中将无法IGet()此Inode! */
	parentDirSize = u.u_pdir->i_size;
	pInode = fileMgr.MakNode(Inode::IRWXU | Inode::IRWXG | Inode::IRWXO);	/* rwxrwxrwx */
	PrintResult(
		"NameITest 1-3",
		User::NOERROR == u.u_error && NULL != pInode
		&& u.u_pdir->i_size == parentDirSize + (int)sizeof(DirectoryEntry)
		);
	pInode->Prele();

	u.u_dirp = filePath1;
	u.u_error = User::NOERROR;
	pInode = fileMgr.NameI(FileManager::NextChar, FileManager::OPEN);
	PrintResult(
		"NameITest 1-4",
		User::NOERROR == u.u_error && NULL != pInode
		&& 0 == pInode->i_size		/* Empty file */
		);
	pInode->Prele();
	
	u.u_dirp = filePath1;
	u.u_error = User::NOERROR;
	pInode = fileMgr.NameI(FileManager::NextChar, FileManager::DELETE);
	PrintResult(
		"NameITest 1-5",
		User::NOERROR == u.u_error && pInode == u.u_pdir
		);
	pInode->Prele();


	//Case 2: Open non-exist file in non-exist folder --> Create non-exist file in non-exist folder
	/* NameI()无法一次性建立深目录层次的文件，只能多次调用逐层建立目录结构，最后创建文件 */
	char* filePath2 = "/nonexist/testfile2";
	u.u_dirp = filePath2;
	u.u_error = User::NOERROR;
	pInode = fileMgr.NameI(FileManager::NextChar, FileManager::OPEN);
	PrintResult(
		"NameITest 2-1", 
		User::ENOENT == u.u_error && NULL == pInode
		);

	u.u_dirp = filePath2;
	u.u_error = User::NOERROR;
	pInode = fileMgr.NameI(FileManager::NextChar, FileManager::CREATE);
	PrintResult(
		"NameITest 2-2",
		User::ENOENT == u.u_error && NULL == pInode
		);


	//Case 3:  Delete non-exist file
	char* filePath3 = "/testfile3";
	u.u_dirp = filePath3;
	u.u_error = User::NOERROR;
	pInode = fileMgr.NameI(FileManager::NextChar, FileManager::DELETE);
	PrintResult(
		"NameITest 3-1",
		User::ENOENT == u.u_error && NULL == pInode
		);


	//Case 4: Create new folder --> Create new file in the new folder
	char* filePath4 = "/testfolder/testfile4";
	u.u_dirp = filePath4;
	u.u_error = User::NOERROR;
	pInode = fileMgr.NameI(FileManager::NextChar, FileManager::CREATE);
	PrintResult(
		"NameITest 4-1",
		User::ENOENT == u.u_error && NULL == pInode
		);
	u.u_error = User::NOERROR;
	parentDirSize = u.u_pdir->i_size;
	pInode = fileMgr.MakNode(Inode::IFDIR/* Vital */ | Inode::IRWXU | Inode::IRWXG | Inode::IRWXO);	/* rwxrwxrwx */
	PrintResult(
		"NameITest 4-2",
		User::NOERROR == u.u_error && NULL != pInode
		&& u.u_pdir->i_size == parentDirSize + (int)sizeof(DirectoryEntry)
		);
	pInode->Prele();

	u.u_dirp =filePath4;
	u.u_error = User::NOERROR;
	pInode = fileMgr.NameI(FileManager::NextChar, FileManager::CREATE);
	PrintResult(
		"NameITest 4-3",
		User::NOERROR == u.u_error && NULL == pInode
		&& 0 == u.u_pdir->i_size	/* new created folder */
		);
	parentDirSize = u.u_pdir->i_size;
	pInode = fileMgr.MakNode(Inode::IRWXU | Inode::IRWXG | Inode::IRWXO);
	PrintResult(
		"NameItest 4-4",
		User::NOERROR == u.u_error && NULL != pInode
		&& u.u_pdir->i_size == parentDirSize + (int)sizeof(DirectoryEntry)
		);
	pInode->Prele();

	/* 对内存Inode的修改写回c.img */
	fileMgr.m_FileSystem->Update();
	/* MakNode()->WriteDir()->WriteI()中把未满512字节的盘块标记"延迟写"，
	 * 所以这里需要把延迟盘块写入c.img，注意是ROOTDEV，而Update()中是NODEV。
	 */
	Kernel::Instance().GetBufferManager().Bflush(DeviceManager::ROOTDEV);
	return true;
}

bool NameITest()
{
	User& u = Kernel::Instance().GetUser();
	FileManager& fileMgr = Kernel::Instance().GetFileManager();
	Inode* pInode;

	/* 要搜索的pathname，以create方式进行NameI() */
	u.u_dirp = "/file1";
	pInode = fileMgr.NameI(FileManager::NextChar, FileManager::CREATE);
	if (u.u_error != User::NOERROR)
	{
		Diagnose::Write("user error = %d \n", u.u_error);
		while(true);
	}
	/* file1本身就不存在，NameI显然返回NULL，调用MakNode创建file1 */
	if ( NULL == pInode )
	{
		pInode = fileMgr.MakNode(0x1FF);
		if(NULL == pInode)
		{
			Diagnose::Write("Create /file1 failed!\n");
			while(true);
		}
		else
		{
			PrintInode("rootDir", u.u_pdir);
			/* Print file1 Inode */
			PrintInode("pInode", pInode);

			/* 必须Unlock file1 Inode!!! 否则以后NameI()中将无法IGet() file1 Inode */
			pInode->Prele(); /* IPut(pInode)效果等同 */

			Diagnose::Write("Create /file1 succeed! \n");
		}
	}/* 至此，创建file1成功! */

	/* 要搜索的路径 */
	u.u_dirp = "/fileX";
	pInode = fileMgr.NameI(FileManager::NextChar, FileManager::OPEN);
	/* Obviously it will fail! */
	if ( NULL == pInode )
	{
		Diagnose::Write("/fileX: NameI() failed !\n");
	}
	else
	{
		Diagnose::Write("/fileX: NameI() OK !\n");
		while(true);
	}

	/* u_error一定要清零，否则前一次NameI()的失败会导致搜索file1也失败 */
	u.u_error = User::NOERROR;
	u.u_dirp = "/file1";
	pInode = fileMgr.NameI(FileManager::NextChar, FileManager::OPEN);
	if ( NULL == pInode )
	{
		Diagnose::Write("/file1: NameI() failed !\n");
		while(true);
	}
	else
	{
		pInode->Prele();	/* Unlock Inode */
		Diagnose::Write("/file1: NameI() OK !\n");
		PrintInode("file1", pInode);
	}

	/* Disable Output!!! */
	Diagnose::TraceOff();

	/* 对内存Inode的修改写回c.img */
	fileMgr.m_FileSystem->Update();
	/* MakNode()->WriteDir()->WriteI()中把未满512字节的盘块标记"延迟写"，所以这里需要把延迟盘块写入c.img */
	Kernel::Instance().GetBufferManager().Bflush(DeviceManager::ROOTDEV);

	Diagnose::TraceOn();
	return true;
}

bool SetupDirTree()
{
	User& u = Kernel::Instance().GetUser();
	FileManager& fileMgr = Kernel::Instance().GetFileManager();
	Inode* pInode;

	u.u_dirp = "/Folder";
	pInode = fileMgr.NameI(FileManager::NextChar, FileManager::CREATE);
	if (u.u_error != User::NOERROR)
	{
		Diagnose::Write("user error = %d \n", u.u_error);
		while(true);
	}

	if ( NULL == pInode )
	{
		pInode = fileMgr.MakNode(Inode::IFDIR | 0x1FF);
		if ( NULL == pInode )
		{
			Diagnose::Write("Create /New Folder failed!\n");
			while(true);
		}
		else
		{
			PrintInode("Parent Inode", u.u_pdir);
			PrintInode("/New Folder", pInode);
			pInode->Prele();
			Diagnose::Write("Create /New Folder succeed! \n");
		}
	}

	u.u_dirp = "/Folder/SubFolder";
	pInode = fileMgr.NameI(FileManager::NextChar, FileManager::CREATE);
	if ( NULL == pInode )
	{
		pInode = fileMgr.MakNode(Inode::IFDIR | 0x1FF);
		if ( NULL == pInode )
		{
			Diagnose::Write("Create /Sub Folder failed!\n");
			while(true);
		}
		else
		{
			pInode->Prele();
			Diagnose::Write("Create /Sub Folder succeed! \n");
		}
	}

	u.u_dirp = "/bin";
	pInode = fileMgr.NameI(FileManager::NextChar, FileManager::CREATE);
	if (u.u_error != User::NOERROR)
	{
		Diagnose::Write("User error = %d \n", u.u_error);
		while(true);
	}
	if ( NULL == pInode )	/* /bin not exist, pInode should be NULL!! */
	{
		pInode = fileMgr.MakNode(0x1FF);
		if(NULL == pInode)
		{
			Diagnose::Write("Create /bin failed!\n");
			while(true);
		}
		else
		{
			pInode->i_flag |= (Inode::IACC | Inode::IUPD);
			pInode->i_mode = Inode::IALLOC | Inode::IFDIR /* Most vital!! */| Inode::IREAD | Inode::IWRITE | Inode::IEXEC | (Inode::IREAD >> 3) | (Inode::IWRITE >> 3) | (Inode::IEXEC >> 3) | (Inode::IREAD >> 6) | (Inode::IWRITE >> 6) | (Inode::IEXEC >> 6);

			Diagnose::TraceOn();
			PrintInode("/", u.u_pdir);
			PrintInode("/bin", pInode);
			//Delay();Delay();
			pInode->Prele();
			Diagnose::Write("Create /bin succeed! \n");
		}
	}

	//Diagnose::TraceOff();
	fileMgr.m_FileSystem->Update();
	Kernel::Instance().GetBufferManager().Bflush(DeviceManager::ROOTDEV);
	//Diagnose::TraceOn();

	return true;
}

bool TestFileSystem()
{
	if ( true == NameITest() )
	{
		return true;
	}
	else
	{
		return false;
	}
	// MakeFS();

	// /* Related Functions as IFree(), IGet(), IPut(), 
	 // * IUpdate()... Successfully Tested ^_^
	 // */
	// IAllocTest();

	// if(AllocAllBlock() == true)
	// {
		// Diagnose::Write("Test in AllocAllBlock() Succeed!\n");
		// return true;
	// }
	// return false;
}

