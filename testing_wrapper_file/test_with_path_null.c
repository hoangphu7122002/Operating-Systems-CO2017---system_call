#include <sys/syscall.h>
#include <stdio.h>
#include <unistd.h>
#define SIZE 200
int main(){
    long sys_return_value;
    unsigned long stat[SIZE];
    sys_return_value = syscall(548, NULL, &stat);
    printf("MSSV %lu\n", stat[0]);
    return 0;
}