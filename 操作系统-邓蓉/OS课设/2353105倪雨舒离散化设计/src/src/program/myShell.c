// myShell.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>   // 只用 strcmp
#include <sys.h>

#define MAXLINE 128
#define MAXARGS 16
static int parse_cmd(char *line, char *argv[], int maxargs)
{
    int argc = 0;
    char *p = line;

    while (*p != '\0' && argc < maxargs - 1) {
        /* 跳过前面的空白 */
        while (*p == ' ' || *p == '\t')
            p++;

        if (*p == '\0')
            break;

        /* 当前单词的开头 */
        argv[argc++] = p;

        /* 走到单词结尾 */
        while (*p != '\0' && *p != ' ' && *p != '\t')
            p++;

        if (*p == '\0')
            break;

        /* 把空白变成字符串结束符，并继续向后 */
        *p = '\0';
        p++;
    }

    argv[argc] = 0;
    return argc;
}
int main1(void)
{
    char line[MAXLINE];
    char *argv[MAXARGS];
    int pid, exitCode;

    while (1) {
        printf("myShell> ");
        gets(line);                     // 课程环境中可用

        // 空行直接跳过
        if (line[0] == '\0')
            continue;

        // 解析命令行为 argv[]
        int argc = parse_cmd(line, argv, MAXARGS);

        if (argc == 0)
            continue;

        // 内部命令：logout
        if (strcmp(argv[0], "logout") == 0) {
            exit(0);
        }

        // 选做：cd
        if (strcmp(argv[0], "cd") == 0) {
            if (argc < 2)
                printf("cd: missing operand\n");
            else
                chdir(argv[1]);
            continue;                   // 不 fork/exec，继续下一轮
        }

        // 外部命令：fork + execv
        pid = fork();
        if (pid == 0) {
            // 子进程：执行用户输入的命令
            execv(argv[0], argv);

            // execv 失败才会执行到这里
            printf("Command not found: %s\n", argv[0]);
            exit(1);
        } else {
            // 父进程：等待子进程结束
            wait(&exitCode);
        }
    }
}
