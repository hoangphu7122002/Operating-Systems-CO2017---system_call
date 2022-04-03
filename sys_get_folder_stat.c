#include <linux/kernel.h>
#include <linux/path.h>
#include <linux/syscalls.h>
#include <linux/fs.h>
#include <linux/namei.h>

struct folder_info {
   	char name[128];
   	mode_t permission;
   	uid_t uid;
   	gid_t gid;
   	long long size;
   	long long atime;
};
 
struct folder_stat {
   	long studentID;
   	struct folder_info folder;
   	struct folder_info parent_folder;
   	struct folder_info last_access_child_folder;
};


static struct file* get_file_utils(char* kernel_path){
    struct file* current_file = kmalloc(sizeof(struct file),GFP_KERNEL);
    current_file = filp_open(kernel_path,O_RDONLY,0);
    
    return current_file;
}

static char* map_path_in_kernel(char* path){
    //PATH_MAX = 4096bytes
    char* kernel_path = kmalloc(PATH_MAX, GFP_KERNEL); //GFP_KERNEL is flag for KERNEL allocation smaller than page
    
    long flag = strncpy_from_user(kernel_path,path,PATH_MAX);
    
    if (path == NULL || flag == -EFAULT){
        return NULL;
    }
    return kernel_path;
}

// static struct folder_info constructor_utils(void){
// 	struct folder_info folder_utils;
// 	folder_utils.atime = 0;
// 	folder_utils.size = 0;
// 	folder_utils.uid = -1;
// 	folder_utils.gid = -1;
// 	folder_utils.permission = -1;

// 	return folder_utils;
// }

static struct dentry* save_dir = NULL;
static time_t late_access = 0;
static long long loop_entry_utils(long mode,struct dentry* dir){
	struct dentry* current_dentry = NULL;
	long long size = 0;
	if (dir == NULL){
		printk("NULL direction\n");
		return 0;
	}
	list_for_each_entry(current_dentry, &dir->d_subdirs,d_child){
		if (current_dentry == NULL){
			break;
		}
		if (strlen(current_dentry->d_iname) == 0){
			continue;
		}
		if (current_dentry->d_inode == NULL){
			continue;
		}
		if (mode == 0){
			if (S_ISDIR(current_dentry->d_inode->i_mode)){
				size += loop_entry_utils(0,current_dentry);
			}
			else {
				size += (long long)current_dentry->d_inode->i_size;
			}
		}
		else {
			if (S_ISDIR(current_dentry->d_inode->i_mode)){
				if (current_dentry->d_inode->i_atime.tv_sec > late_access){
					save_dir = current_dentry;
					late_access = current_dentry->d_inode->i_atime.tv_sec;
				}
			}
		}
	}
	return size;
}

static void print_information(const char* text,struct folder_info folder){
    printk("\n=============================\n");
    printk("%s information\n",text);
    printk("%s name: %s\n",text,folder.name);
    printk("%s size: %lld\n",text,folder.size);
    printk("%s atime: %lld\n",text,folder.atime);
    printk("%s permission: %d\n",text,folder.permission);    
    printk("%s gid: %u\n",text,folder.gid);
    printk("%s uid: %u\n",text,folder.uid);
    printk("\n=============================\n");    
}

static struct folder_info assign_information(struct dentry* assign_dentry){
	struct folder_info folder_utils;
	
    folder_utils.atime = assign_dentry->d_inode->i_atime.tv_sec;
    folder_utils.gid = assign_dentry->d_inode->i_gid.val;
    folder_utils.uid = assign_dentry->d_inode->i_uid.val;
    strlcpy(folder_utils.name,assign_dentry->d_name.name,128);
    folder_utils.permission = assign_dentry->d_inode->i_mode;
    folder_utils.size = loop_entry_utils(0,assign_dentry);
    
    return folder_utils;
}

static void state_copy_user_space(struct folder_stat* info, struct folder_stat* stat){
    copy_to_user(stat, info, sizeof(struct folder_stat));
   	copy_to_user(&stat->parent_folder, &info->parent_folder,sizeof(struct folder_info));
   	copy_to_user(&stat->folder, &info->folder, sizeof(struct folder_info));
   	copy_to_user(&stat->last_access_child_folder,&info->last_access_child_folder,sizeof(struct folder_info));
	copy_to_user(&stat->studentID,&info->studentID,sizeof(long));
}

static struct folder_info initialize_current_folder(struct inode* current_file_node){
    struct folder_info current_folder;
    
    //Assign value for current_folder;
    current_folder.size = 0;
    current_folder.permission = (mode_t)current_file_node->i_mode;
    current_folder.gid = (gid_t)current_file_node->i_gid.val;
    current_folder.uid = (uid_t)current_file_node->i_uid.val;
        
    //
    return current_folder;   
}

static struct inode* get_inode_from_file(struct file* current_file){
    struct inode* current_file_node = kmalloc(sizeof(struct inode), GFP_KERNEL); //state with current_file
    current_file_node = current_file->f_inode;
    
    return current_file_node;   
}

SYSCALL_DEFINE2(get_folder_stat, char*, path, struct folder_stat*, stat) {
	//inode* instance variable
	struct inode* current_file_node = NULL;
	
	//dentry* instance
	struct dentry* current_file_dentry = NULL;
	struct dentry* subdir_dentry = NULL;
	struct dentry* parentdir_dentry = NULL;
	
	//file* instance
	struct file* current_file = NULL;
	
	//stat* instance 
	struct folder_stat* info = NULL;	
	
	//folder instance variable
	struct folder_info current_folder;
	struct folder_info parent_folder;
	struct folder_info lastest_access_child_folder;
	
	char* kernel_path = map_path_in_kernel(path);
    if (kernel_path == NULL){
        printk("path is NULL\n");
        return EINVAL; //Null Adrees
    }
    
    current_file = get_file_utils(kernel_path);
    //Macro for check error_value
    if (IS_ERR(current_file)){
        printk("file is error when opening\n");
        return ENOENT; //ERROR_NO_ENTITY
    }	
    current_file_node = get_inode_from_file(current_file);
    current_folder = initialize_current_folder(current_file_node);
    
    current_file_dentry = current_file->f_path.dentry;
    strlcpy(current_folder.name, current_file_dentry->d_name.name,128);
    current_folder.size = loop_entry_utils(0,current_file_dentry);
    
    loop_entry_utils(1,current_file_dentry);
	subdir_dentry = save_dir;
    if (subdir_dentry != NULL){
    	lastest_access_child_folder = assign_information(subdir_dentry);
	}
	if (subdir_dentry == NULL){
		printk("====NO SUBDIR FOLDER====\n");
	}
	else {
		print_information("last access child folder",lastest_access_child_folder);	
	}
	
	parentdir_dentry = current_file_dentry->d_parent;
	if (parentdir_dentry != NULL){
		parent_folder = assign_information(parentdir_dentry);
	}
	
	print_information("current_folder",current_folder);
	if (parentdir_dentry == NULL){
		printk("====NO PARENT FOLDER====\n");
	}
	else {
		print_information("parent_folder",parent_folder);	
	}
	

	filp_close(current_file,0);
	info = kmalloc(sizeof(struct folder_stat), GFP_KERNEL);
	copy_from_user(info, (struct folder_stat *)stat,sizeof(struct folder_stat));
	
    info->studentID = 2010514;
	info->folder = current_folder;
    info->last_access_child_folder = lastest_access_child_folder;
    info->parent_folder = parent_folder;
    
    state_copy_user_space(info,stat);
    //free 
    //======================
    kfree(kernel_path);
    kfree(info);
    kfree(current_file_node);
    //======================

    return 0;
}