#ifndef SYS_H
#define SYS_H

#include <time.h>
#include "v6pp.h"   /* get_proc 使用的共享结构体定义 */
#include "v6pp_pids.h"

/* 兼容 C++ */
#ifdef __cplusplus
extern "C" {
#endif

	/* p_sig接收到的信号定义 */
	#define SIGNUL   0  /* No Signal Received */
	#define SIGHUP   1  /* Hangup (kill controlling terminal) */
	#define SIGINT   2  /* Interrupt from keyboard */
	#define SIGQUIT  3  /* Quit from keyboard */
	#define SIGILL   4  /* Illegal instrution */
	#define SIGTRAP  5  /* Trace trap */
	#define SIGABRT  6  /* use abort() API */
	#define SIGBUS   7  /* Bus error */
	#define SIGFPE   8  /* Floating point exception */
	#define SIGKILL  9  /* Kill(can't be caught or ignored) */
	#define SIGUSR1  10 /* User defined signal 1 */
	#define SIGSEGV  11 /* Invalid memory segment access */
	#define SIGUSR2  12 /* User defined signal 2 */
	#define SIGPIPE  13 /* Write on a pipe with no reader, Broken pipe */
	#define SIGALRM  14 /* Alarm clock */
	#define SIGTERM  15 /* Termination */
	#define SIGSTKFLT 16 /* Stack fault */
	#define SIGCHLD  17 /* Child process has stopped or exited, changed */
	#define SIGCONT  18 /* Continue executing, if stopped */
	#define SIGSTOP  19 /* Stop executing */
	#define SIGTSTP  20 /* Terminal stop signal */
	#define SIGTTIN  21 /* Background process trying to read, from TTY */
	#define SIGTTOU  22 /* Background process trying to write, to TTY */
	#define SIGURG   23 /* Urgent condition on socket */
	#define SIGXCPU  24 /* CPU limit exceeded */
	#define SIGXFSZ  25 /* File size limit exceeded */
	#define SIGVTALRM 26 /* Virtual alarm clock */
	#define SIGPROF  27 /* Profiling alarm clock */
	#define SIGWINCH 28 /* Window size change */
	#define SIGIO    29 /* I/O now possible */
	#define SIGPWR   30 /* Power failure restart */
	#define SIGSYS   31 /* invalid sys call */

int execv(char* pathname, char* argv[]);
int fork();
int wait(int* status);
int exit(int status);
int signal(int signal, void (*func)());
int kill(int pid, int signal);
int sleep(unsigned int seconds);
int brk(void* newEndDataAddr);
int sbrk(int increment);
int syncFileSystem();
int getPath(char* path);
int getpid();
int getppid(int pid);
unsigned int getgid();
unsigned int getuid();
int setgid(short gid);
int setuid(short uid);
int gettime(struct tms* ptms);   /* 系统时间 */

/* 获取进程的用户态/内核态 CPU 时间片等 */
int times(struct tms* ptms);

/* 获取系统进程切换次数 */
int getswtch();

/* 在屏幕底部以 lines 行显示调试信息 */
int trace(int lines);
int get_pids(struct v6pp_pids* out);
/* 51#: 获取当前进程的虚拟/物理信息（v6pp 结构体版本） */
int get_proc(struct v6pp_procinfo* buf);

#ifdef __cplusplus

#endif

#endif /* SYS_H */
