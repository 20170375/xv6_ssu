#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

// exec.c
int main()
{
    char *argv[2];
    argv[0] = "/bin/ls";
    argv[1] = 0;

    int pid = fork();

    if(pid > 0){
        printf("parent: child=%d\n", pid);
        pid = wait(NULL);
        printf("child %d is done\n", pid);
    } else if(pid == 0){
        printf("child: exiting\n");
        execve(argv[0], argv, NULL);
    } else{
        printf("exec error\n");
    }
    return 0;
}