#include <get_folder_stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>

int main(int argc, char* argv[]){
    char* path = argv[1];
    struct folder_stat stat;
    if(get_folder_stat(path,&stat) == 0){
        printf("MSSV: %lu",stat.studentID);
    }
    else {
        printf("path is error\n");
    }
    return 0;
}