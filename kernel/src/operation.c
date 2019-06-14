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
	char rpath[100];
	strcpy(rpath,path);
	int fd=vfs->open(path,4);
	int end=vfs->lseek(fd,0,2);
	printf("\033[42m end=%d\033[0m\n");
	int doff=0,i=0;
	while(doff<=end) {
	    char *name=pmm->alloc(256);
		vfs->lseek(fd,doff,0);
		vfs->read(fd,name,100);
		vfs->lseek(fd,doff+112,0);
		off_t off;
		vfs->read(fd,&off,sizeof(off_t));
		if(off==-1) {
			doff+=128;
			continue;
		}
		strcat(rpath,name);
		filesystem_t *fs=cur->flides[fd]->inode->fs;
		inode_t *pe=fs->ops->lookup(fs,rpath,9);
		strcpy(rpath,path);
		if(pe->type==0)
			strcat(name,"  dir file  rw.\n");
		else
			strcat(name,"    file    rwx\n");
		pmm->free(pe);
		pre[i]=name;
		i++;
		printf("%s ",name);
		doff+=128;
	}
	pre[i]=NULL;
	vfs->close(fd);
	return (char *)pre;
}
