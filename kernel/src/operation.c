#include<common.h>
#include<klib.h>
void cd_operation(const char * path) {
	task_t *cur=current_task();
	filesystem_t *fs=cur->preloc->fs;
	inode_t *pre=fs->ops->lookup(fs,path,7);
	pmm->free(cur->preloc);
	cur->preloc=pre;
}
void real_path(char *path,const char *tpath) {
	task_t *cur=current_task();
	char refpath[100];
	memset(refpath,0,sizeof(refpath));
	if(tpath[0]=='.'&&tpath[1]!='.')  {
		strcpy(refpath,cur->loc);
		strcat(refpath,"/");
		strcat(refpath,tpath+1);
	}
	else if(tpath[0]=='.') {
		strcpy(refpath,cur->loc);
		while(refpath[strlen(refpath)-1]!='/')
			refpath[strlen(refpath)-1]=0;
		strcat(path,tpath+2);
	}
	else 
		strcpy(refpath,tpath);
	printf("refpath is %s",refpath);
	int l=strlen(refpath);
	for(int i=1;i<l;) {
		int j=0;
		while(refpath[i+j]!='/'&&i+j<strlen(refpath))
			j++;
		if(refpath[i]=='.'&&j==2) {
			int k=i+j-1;
			while(refpath[k]!='/')refpath[k]=0;
			refpath[k]=0;
			while(refpath[k]!='/')refpath[k]=0;
		}
		if(refpath[i]=='.'&&j==1) {
			int k=i+j-1;
			while(refpath[k]!='/')refpath[k]=0;
		}
		i+=j;
	}
	for(int i=0;i<l;i++) 
		if(refpath[i]!=0&&(i==0||refpath[i-1]==0)) {
			strcat(path,refpath);
		}
	printf("new path is %s\n",path);
}
void mkdir_operation(const char *path) {
	vfs->mkdir(path);
}
void rmdir_operation(const char *path) {
	vfs->rmdir(path);
}
char* ls_operation(const char *tpath) {
	task_t *cur =current_task();
	char path[100];
	memset(path,0,sizeof(path));
	if(tpath[0]=='.') {
		strcat(path,cur->loc);
		if(tpath[1]=='.') {
			while(path[strlen(path)-1]!='/')
				path[strlen(path)-1]=0;
		}
		else
			strcat(path,"/");
	}
	strcat(path,tpath);
	printf("now path is %s\n",path);
	printf("ls:path=%s\n",path);
	char **pre=pmm->alloc(256);
	char rpath[100];
	memset(rpath,0,sizeof(rpath));
	strcpy(rpath,path);
	int fd=vfs->open(path,4);
	int end=vfs->lseek(fd,0,2);
	printf("\033[42m end=%d\033[0m\n");
	int doff=0,i=0;
	char *name=pmm->alloc(256);
	sprintf(name,"%s","Name                 Type                 Priority\n");
	pre[0]=name;
	i++;
	while(doff<end) {
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
		strcat(rpath,"/");
		strcat(rpath,name);
		filesystem_t *fs=cur->flides[fd]->inode->fs;
		inode_t *pe=fs->ops->lookup(fs,rpath,9);
		memset(rpath,0,sizeof(rpath));
		strcpy(rpath,path);
		while(strlen(name)<21)
			strcat(name," ");
		if(pe->type==0)
			strcat(name,"Directory File");
		else
			strcat(name,"File");
		while(strlen(name)<42)
			strcat(name," ");
		strcat(name,"rw.\n");
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
