#ifndef PAGE_DIRECTORY_H
#define PAGE_DIRECTORY_H

#include "PageTable.h"

struct PageDirectoryEntry
{
	unsigned char	m_Present : 1;
	unsigned char	m_ReadWriter : 1;
	unsigned char	m_UserSupervisor : 1;
	unsigned char	m_WriteThrough : 1;
	unsigned char	m_CacheDisabled : 1;
	unsigned char	m_Accessed : 1;
	unsigned char	m_Reserved : 1;
	unsigned char	m_PageSize : 1;
	unsigned char	m_GlobalPage : 1;
	unsigned char	m_ForSystemUser : 3;
	unsigned int	m_PageTableBaseAddress : 20;		
}__attribute__((packed));


class PageDirectory
{
public:
	PageDirectory();
	~PageDirectory();

	PageTable& GetPageTableByIdx(int idx);
	PageDirectoryEntry& GetPageDirectoryEntryByIdx(int idx);

public:
	PageDirectoryEntry m_Entrys[1024];
};

#endif

