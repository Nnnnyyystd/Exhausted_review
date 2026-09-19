#include <stdio.h>
#include <stdlib.h>  
#include <malloc.h>
#include <sys.h>

int version = 1;
  
int main1()
{  
    int i, j;
    printf("The address of version is %x\n", &version);

    printf("test of malloc\n"); 
    int *c = malloc(128);
    printf("the address of c: %x\n", c);
    int *d = malloc(256);
    printf("the address of d: %x\n", d);
    int *e = malloc(256);
    printf("the address of e: %x\n", e);
    free(d); 
    int *f = malloc(83);
    printf("the address of f: %x\n", f);

    printf("the end of heap: %x\n", sbrk(0));

    f = malloc(3*4096);
    printf("the end of heap(after another expand): %x\n", sbrk(0));

    return 0; 
}
