#include <stdio.h>
#include <string.h>

int main() {
    int authenticated = 0;  // 认证标志，初始为0（未认证）
    char buffer[16];         // 小缓冲区，用于存储输入

    printf("Password?: ");
    gets(buffer);            // 使用不安全的gets函数，容易导致缓冲区溢出

    // 模拟密码检查
    if (strcmp(buffer, "password") == 0) {
        authenticated = 1;
    }

    // 根据认证标志决定程序流程
    if (authenticated) {
        printf("success,already changed\n");
    } else {
        printf("failure.\n");
    }

    return 0;
}