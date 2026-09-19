#include "sys.h"
#include "stdlib.h"
#include "v6pp.h"
#include "v6pp_pids.h"

int execv(char *pathname, char *argv[])
{
	int res;
	int argc = 0;
	while(argv[argc] != 0)
		argc++;	
	__asm__ volatile ( "int $0x80":"=a"(res):"a"(11),"b"(pathname),"c"(argc),"d"(argv));
	if ( res >= 0 )
		return res;
	return -1;
}

int fork()
{
	int res;
	__asm__ __volatile__ ( "int $0x80":"=a"(res):"a"(2));
	if ( res >= 0 )
		return res;
	return -1;
}

int wait(int* status)	/* ��ȡ�ӽ��̷��ص�Return Code */
{
	int res;
	__asm__ __volatile__ ( "int $0x80":"=a"(res):"a"(7),"b"(status));
	if ( res >= 0 )
		return res;
	return -1;
}

int exit(int status)	/* �ӽ��̷��ظ������̵�Return Code */
{
	int res;
	__asm__ __volatile__ ( "int $0x80":"=a"(res):"a"(1),"b"(status));
	if ( res >= 0 )
		return res;
	return -1;
}

int signal(int signal, void (*func)())
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(48),"b"(signal), "c"(func) );
	if ( res >= 0 )
		return res;
	return -1;
}

int kill(int pid, int signal)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(37),"b"(pid), "c"(signal) );
	if ( res >= 0 )
		return res;
	return -1;
}

int sleep(unsigned int seconds)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(35),"b"(seconds) );
	if ( res >= 0 )
		return res;
	return -1;
}

/* ʹ��errno��Ҫinclude "stdlib.h" */
extern errno;
int brk(void * newEndDataAddr)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(17),"b"(newEndDataAddr));
	/* ϵͳ���õķ���ֵ��ֵAPP��ȫ�ֱ���errno */
	if ( res >= 0 )
		return res;
	errno = -1*res;
	printf("%d\n",errno);
	return -1;
}

int syncFileSystem()
{
	int res;
	__asm__ volatile ( "int $0x80":"=a"(res):"a"(36) );
	if ( res >= 0 )
		return res;
	return -1;
}

int getPath(char *path)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(39),"b"(path));
	if ( res >= 0 )
		return res;
	return -1;
}

int getpid()
{
	int res;
	__asm__ volatile ( "int $0x80":"=a"(res):"a"(20) );
	if ( res >= 0 )
		return res;
	return -1;
}
int getppid(int pid)
{
    int res;
    __asm__ volatile ( "int $0x80" : "=a"(res) : "a"(49), "b"(pid) );
    if (res >= 0) return res;
    return -1;
}
int get_proc(struct v6pp_procinfo* buf)
{
    int res;
    __asm__ volatile ("int $0x80" : "=a"(res) : "a"(51), "b"(buf));
    return res;   // 0 成功；-1 失败
}
int get_pids(struct v6pp_pids* out)
{
    int res;
    __asm__ volatile ("int $0x80" : "=a"(res) : "a"(50), "b"(out));
    return res;   /* 0=成功，-1=失败 */
}
unsigned int getgid()
{
	int res;
	__asm__ volatile ( "int $0x80":"=a"(res):"a"(47) );
	if ( res >= 0 )
		return res;
	return -1;
}

unsigned int getuid()
{
	int res;
	__asm__ volatile ( "int $0x80":"=a"(res):"a"(24) );
	if ( res >= 0 )
		return res;
	return -1;
}

int setgid(short gid)
{
	int res;
	__asm__ volatile ( "int $0x80":"=a"(res):"a"(46),"b"(gid) );
	if ( res >= 0 )
		return res;
	return -1;
}

int setuid(short uid)
{
	int res;
	__asm__ volatile ( "int $0x80":"=a"(res):"a"(23),"b"(uid) );
	if ( res >= 0 )
		return res;
	return -1;
}

int gettime(struct tms* ptms)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(13),"b"(ptms) );
	if ( res >= 0 )
		return res;
	return -1;
}

int times(struct tms* ptms)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(43),"b"(ptms) );
	if ( res >= 0 )
		return res;
	return -1;
}

int getswtch()
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(38) );
	if ( res >= 0 )
		return res;
	return -1;
}

int trace(int lines)
{
	int res;
	__asm__ volatile ("int $0x80":"=a"(res):"a"(29),"b"(lines) );
	if ( res >= 0 )
		return res;
	return -1;
}

unsigned int fakeedata = 0;
int sbrk(int increment)
{
	if (fakeedata == 0)
	{
		fakeedata = brk(0);
	}
	unsigned int newedata = fakeedata + increment - 1;
	brk(((newedata >> 12) + 1) << 12);
	fakeedata = newedata + 1;
	return fakeedata;
}
