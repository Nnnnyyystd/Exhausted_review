#ifndef UACCESS_H_
#define UACCESS_H_

#include <stddef.h>

/* 最小可用的“拷贝到用户区/从用户区拷贝”接口
   这里我们先实现“直接写”版本（你的内核是平坦映射/一致映射时可用）。
   日后若要做地址合法性检查，再在实现里增强即可。 */

bool copy_to_user(void* user_dst, const void* kernel_src, size_t n);
bool copy_from_user(void* kernel_dst, const void* user_src, size_t n);

#endif
