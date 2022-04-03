#include "get_folder_stat.h"
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>

long get_folder_stat(char* path,struct folder_stat * info){
    long sys_return_value = syscall(548,path,info);
    return sys_return_value;
}
