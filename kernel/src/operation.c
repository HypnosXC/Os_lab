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
	task_t *cur=current_task();
	char **pre=pmm->alloc(256);
	int fd=vfs->open(path,4);
	int end=vfs->lseek(fd,0,2);
	printf("\033[42m end=%d\033[0m\n");
	int doff=0,i=0;
	while(doff!=end) {
	    char *name=pmm->alloc(128);	
		vfs->lseek(fd,doff,0);
		vfs->read(fd,name,100);
		vfs->lseek(fd,doff+112,0);
		off_t off;
		vfs->read(fd,&off,sizeof(off_t));
		if(!inode_ex(off,cur->preloc->fs))
			continue;
		name[strlen(name)]='\n';
		pre[i]=name;
		i++;
		printf("%s ",name);
		doff+=128;
	}
	pre[i]=NULL;
	return (char *)pre;
}
