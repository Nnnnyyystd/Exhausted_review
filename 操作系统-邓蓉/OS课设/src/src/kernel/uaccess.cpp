#include "uaccess.h"

/* 在你当前的 V6++ 教学内核里，用户空间与内核共享一份线性地址映射，
   因此我们先给出最简实现：直接 memcpy。若有安全需求再加边界检查。 */

static inline void kmemcpy(void* dst, const void* src, size_t n) {
    unsigned char* d = (unsigned char*)dst;
    const unsigned char* s = (const unsigned char*)src;
    while (n--) *d++ = *s++;
}

bool copy_to_user(void* user_dst, const void* kernel_src, size_t n) {
    if (!user_dst || !kernel_src) return false;
    kmemcpy(user_dst, kernel_src, n);
    return true;
}

bool copy_from_user(void* kernel_dst, const void* user_src, size_t n) {
    if (!kernel_dst || !user_src) return false;
    kmemcpy(kernel_dst, user_src, n);
    return true;
}
