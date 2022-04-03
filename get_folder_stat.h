#ifndef _GET_FOLDER_STAT_H
#define _GET_FOLDER_STAT_H

#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

struct folder_info {
	char name[128];
	mode_t permission;
	uid_t uid;
	gid_t gid;
	double size;
	time_t atime;
};

struct folder_stat {
	long studentID;
	struct folder_info folder;
	struct folder_info parent_folder;
	struct folder_info last_access_child_folder;
};

long get_folder_stat(char* path, struct folder_stat * info);
#endif
