#include<common.h>
#include<klib.h>
void cd_operation(const char * path) {
	task_t *cur=current_task();
	filesystem_t *fs=cur->preloc->fs;
	inode_t *pre=fs->ops->lookup(path,7);
	pmm->free(cur->preloc);
	cur->preloc=pre;
}
void mkdir_operation(const char *path) {
	vfs->mkdir(path);
}
void rmdir_operation(const char *path) {
	vfs->rmdir(path);
}
void ls_operation(const char *path) {
	int fd=vfs->open(path,4);
	int end=vfs->lseek(fd,0,2);
	int doff=0;
	char name[109];
	while(doff!=end) {
		vfs->lseek(fd,doff,0);
		vfs->read(fd,name,100);
		printf("%s ",name);
		doff+=128;
	}
	printf("\n");
}
