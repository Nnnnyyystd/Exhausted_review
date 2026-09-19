/* diagspam.c — 在 -nostdinc/-nostartfiles/-nostdlib 环境下生成大量
 * fork/exit/wait 事件，触发下半屏 Diagnose 输出多行日志用于测试。
 * 说明：
 *  1) 自定义入口：_mainl（配合链接参数 -e _mainl）
 *  2) 无任何标准头；用 extern 声明系统调用
 *  3) 提供 __main 的空桩，防止 MinGW 工具链要求
 */

/* ====== 最小系统调用原型（按你的内核导出名，如有大小写差异可改名） ====== */
extern int  fork(void);
extern int  wait(int *status);      /* 返回子进程 pid，status 可为 NULL */
extern void exit(int code);
extern void sleep(int ticks_or_sec);/* 若无此调用，可把调用删掉，或用忙等替代 */
extern int  write(int fd, const char *buf, int n);

/* 某些 MinGW 变体会在 -nostartfiles 下仍引用 __main；给一个空桩 */
void __main(void) {}

/* ====== 简易工具函数：避免依赖标准库 ====== */
static int my_strlen(const char *s) { int n=0; while (s[n]) n++; return n; }
static void puts1(const char *s) { write(1, s, my_strlen(s)); write(1, "\n", 1); }

/* ====== 入口：_mainl  ======
 * 为了不依赖 C 运行时，这里不返回；如需返回可直接 return; 由链接脚本决定。
 */
void _mainl(void)
{
    const int rounds = 30; /* 可改：循环次数，默认 300 轮 */
    const int delay  = 0;   /* 可改：子进程退出前的 sleep 时间（单位依系统实现） */

    int i, st, pid;

    puts1("[diagspam] start");

    for (i = 0; i < rounds; ++i) {
        pid = fork();
        if (pid < 0) {
            puts1("[diagspam] fork failed");
            break;
        }
        if (pid == 0) {
            /* 子进程：可选 sleep，使日志更“分散” */
            if (delay > 0) sleep(delay);
            /* 退出码随轮次变化，便于观察 */
            exit(i & 255);
        } else {
            /* 父进程等待子进程退出，触发 Diagnose 的进程/等待日志 */
            (void)wait(&st);
        }
    }

    puts1("[diagspam] done");
    /* 不返回也可，由加载器回收；若要返回可直接 return; */
}
