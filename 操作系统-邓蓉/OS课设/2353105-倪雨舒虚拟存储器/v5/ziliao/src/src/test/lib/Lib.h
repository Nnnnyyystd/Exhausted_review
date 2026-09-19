#ifndef LIB_H
#define LIB_H

/* 系统调用的C库封装函数，为避免跟内核中函数重名，加上lib_前缀 */

int lib_creat(char* pathname, unsigned int mode);

int lib_open(char* pathname, unsigned int mode);

int lib_close(int fd);

int lib_write(int fd, char* buf, int nbytes);

int lib_read(int fd, char* buf, int nbytes);

int lib_exit(int status);

int lib_wait(int* status);

int lib_fork();

int lib_pipe(int* fildes);

int lib_execv(char* pathname, char* argv[]);

int lib_seek(int fd,unsigned int offset,unsigned int ptrname);

int lib_dup(int fd);

int lib_fstat(int fd,unsigned long statbuf);

int lib_stat(char* pathname,unsigned long statbuf);

int lib_chmod(char* pathname,unsigned int mode);

int lib_chown(char* pathname,short uid, short gid);

short lib_getuid();

int lib_setuid(short uid);

short lib_getgid();

int lib_setgid(short gid);

int lib_getpid();

int lib_nice(int change);

int lib_sig(int signal, void (*func)());

int lib_kill(int pid, int signal);

int lib_sleep(unsigned int seconds);

int lib_pwd(char* pwd);

int lib_brk(unsigned int newSize);

int lib_link(char* pathname,char* newPathname);

int lib_unlink(char* pathname);

int lib_chdir(char* pathname);

int lib_mknod(char* pathname,unsigned int mode, int dev);

int lib_syncFileSystem();

#endif
