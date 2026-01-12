/* spamlog.c — 生成多行日志用于滚动测试（C89、只依赖 stdio.h）
 * 用法：
 *   spamlog              // 无限输出，默认延时因子 50（越大越慢）
 *   spamlog -n 500       // 打 500 行
 *   spamlog -n 500 -d 5  // 更快（更小的 d 更快）
 */

#include <stdio.h>

/* 极简 strcmp（只处理以 '\0' 结尾的 C 字符串） */
static int str_eq(const char* a, const char* b) {
    int i = 0;
    while (a[i] != '\0' && b[i] != '\0') {
        if (a[i] != b[i]) return 0;
        ++i;
    }
    return a[i] == '\0' && b[i] == '\0';
}

/* "字符串 -> 无符号长整型"；遇到非数字即停止 */
static unsigned long to_ul(const char* s) {
    unsigned long v = 0;
    int i = 0;
    if (!s || s[0] == '\0') return 0;   /* 注意：不使用 NULL 宏，只做空指针/空串判断 */
    while (s[i] >= '0' && s[i] <= '9') {
        v = v * 10UL + (unsigned long)(s[i] - '0');
        ++i;
    }
    return v;
}

/* 忙等“延时”，d 越大循环越慢；不同机器速度不同，仅用于制造滚动效果 */
static void msleep_busy(unsigned d) {
    volatile unsigned long i, j;
    for (i = 0; i < (unsigned long)d; ++i) {
        for (j = 0; j < 50000UL; ++j) {
            (void)i; /* 防止被优化 */
        }
    }
}

static void print_usage(const char* prog) {
    printf("Usage: %s [-n lines(0=inf)] [-d delay]\n", prog);
    printf("  -n  number of lines (default 0 = infinite)\n");
    printf("  -d  busy-wait delay factor per line (default 50; smaller = faster)\n");
}

int main(int argc, char** argv) {
    unsigned long max_lines = 0; /* 0 = infinite */
    unsigned delay = 50;         /* 忙等延时因子（越大越慢） */
    unsigned long line = 0;
    int i;

    /* 解析参数（C89：变量在块首声明） */
    i = 1;
    while (i < argc) {
        if ((i + 1) < argc && str_eq(argv[i], "-n")) {
            max_lines = to_ul(argv[i + 1]);
            i += 2;
        } else if ((i + 1) < argc && str_eq(argv[i], "-d")) {
            delay = (unsigned)to_ul(argv[i + 1]);
            i += 2;
        } else if (str_eq(argv[i], "-h") || str_eq(argv[i], "--help")) {
            print_usage(argv[0]);
            return 0;
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    /* 固定宽度的负载串，避免动态分配与库函数 */
    {
        char payload[65]; /* 64 字符 + 终止符 */
        const char* pat = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
        int pat_len = 36;
        int k;

        for (k = 0; k < 64; ++k) payload[k] = pat[k % pat_len];
        payload[64] = '\0';

        for (;;) {
            if (max_lines != 0 && line >= max_lines) break;

            /* 仅用 printf，适配你当前运行时 */
            printf("[%8lu] %s\n", line + 1, payload);

            ++line;
            if (delay > 0) msleep_busy(delay);
        }
    }

    return 0;
}
