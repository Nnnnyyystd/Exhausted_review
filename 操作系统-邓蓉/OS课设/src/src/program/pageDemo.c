#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <sys.h>

static int g_data = 0x12345678;

static void print_block(char *name, unsigned long addr)
{
	printf("%s addr = %x, page = %x, offset = %x\n",
		name,
		addr,
		addr >> 12,
		addr & 0xfff);
}

int main1(int argc, char *argv[])
{
	int local_stack = 0x55aa;
	int *heap_a = (int *)malloc(32);
	int *heap_b = (int *)malloc(4096);
	int *heap_c = (int *)malloc(4096);
	int status = 0;
	int pid;

	if (heap_a == 0 || heap_b == 0 || heap_c == 0)
	{
		printf("malloc failed\n");
		exit(1);
	}

	*heap_a = 0x11111111;
	*heap_b = 0x22222222;
	*heap_c = 0x33333333;

	printf("==== address layout of one process ====\n");
	print_block("main1", (unsigned long)main1);
	print_block("g_data", (unsigned long)&g_data);
	print_block("stack ", (unsigned long)&local_stack);
	print_block("heap_a", (unsigned long)heap_a);
	print_block("heap_b", (unsigned long)heap_b);
	print_block("heap_c", (unsigned long)heap_c);
	print_block("brk   ", (unsigned long)sbrk(0));

	printf("heap_b - heap_a = %x\n", (unsigned long)heap_b - (unsigned long)heap_a);
	printf("heap_c - heap_b = %x\n", (unsigned long)heap_c - (unsigned long)heap_b);
	printf("heap_a value = %x, heap_b value = %x, heap_c value = %x\n", *heap_a, *heap_b, *heap_c);

	printf("==== fork test: same virtual address, isolated write ====\n");
	pid = fork();
	if (pid == 0)
	{
		printf("[child %d] before write\n", getpid());
		print_block("g_data", (unsigned long)&g_data);
		print_block("heap_b", (unsigned long)heap_b);
		printf("[child %d] values before: g_data=%x heap_b=%x\n", getpid(), g_data, *heap_b);

		g_data = 0xcafebabe;
		*heap_b = 0x0badc0de;

		printf("[child %d] values after : g_data=%x heap_b=%x\n", getpid(), g_data, *heap_b);
		exit(0);
	}

	wait(&status);

	printf("[parent %d] after child exit\n", getpid());
	print_block("g_data", (unsigned long)&g_data);
	print_block("heap_b", (unsigned long)heap_b);
	printf("[parent %d] values now   : g_data=%x heap_b=%x\n", getpid(), g_data, *heap_b);

	free(heap_c);
	free(heap_b);
	free(heap_a);
	exit(0);
}
