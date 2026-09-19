#include "PageDirectory.h"

// // ------------- NOTE 3: 从页目录中获取两张用户页表的地址 -------------
// PageTable* PageDirectory::GetUserPageTableFromDirectory() {
//     unsigned int pUserPageTable[2] = {this->m_Entrys[0].m_PageTableBaseAddress, this->m_Entrys[1].m_PageTableBaseAddress};
//     return (PageTable*)pUserPageTable;
// }
// // ------------- END NOTE 3 -------------