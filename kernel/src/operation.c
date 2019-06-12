#include<common.h>
#include<klib.h>
void cd_operation(const char * path) {
	task_t *cur=current_task();
	filesystem_t *fs=cur->preloc->fs;
	inode_t *pre=fs->ops->lookup(fs,path,7);
	pmm->free(cur->preloc);
	cur->preloc=pre;
}
void mkdir_operation(const char *path) {
	vfs->mkdir(path);
}
void rmdir_operation(const char *path) {
	vfs->rmdir(path);
}
char* ls_operation(const char *path) {
	char **pre=pmm->alloc(256);
	int fd=vfs->open(path,4);
	int end=vfs->lseek(fd,0,2);
	int doff=0;
	while(doff!=end) {
	    char *name=pmm->alloc(128);	
		vfs->lseek(fd,doff,0);
		vfs->read(fd,name,100);
		pre[doff/128]=name;
		printf("%s ",name);
		doff+=128;
	}
	pre[doff/128]=NULL;
	return pre;
}
