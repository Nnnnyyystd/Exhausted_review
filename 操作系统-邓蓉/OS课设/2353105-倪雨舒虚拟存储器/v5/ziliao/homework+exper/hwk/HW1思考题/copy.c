//copy.c
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

int main( int argc, char* argv[] )
{
    if(argc!=3)
        printf("Usage: copy oldfile newfile\n");
    int oldFile = open(argv[1], O_RDONLY | O_BINARY);
    int newFile = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC | O_BINARY, S_IRUSR | S_IWUSR);
    clock_t start, end;
    int cnt;
    start = clock();

    char c[4096];
    while( (cnt = read(oldFile,&c,4096)) > 0 )
    {
        write(newFile,&c,cnt);  
    }
    end = clock();
    printf("Copy Done\n");
    close(oldFile);   
    close(newFile);
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Elapsed time: %.6f seconds\n", elapsed);
    return 0;
}
