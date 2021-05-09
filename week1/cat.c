#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

// cat.c
void cat(int fd)
{
    int n;
    char buf[10];

    while((n = read(fd, buf, sizeof(buf))) > 0){
        if(write(STDOUT_FILENO, buf, n) != n){
            printf("cat: write error\n");
            return;
        }
    }
    if(n < 0){
        printf("cat: read error\n");
        return;
    }
}

int main(int argc, char *argv[])
{
    int fd, i;

    if(argc <= 1){
        cat(0);
        exit(0);
    }

    for(i=1; i<argc; i++){
        if((fd = open(argv[i], O_RDONLY)) < 0){
            printf("cat: cannot open %s\n", argv[i]);
            exit(-1);
        }
        cat(fd);
        close(fd);
    }
    exit(0);
}