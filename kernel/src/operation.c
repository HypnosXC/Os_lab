#include<common.h>
#include<klib.h>
void road_free(char *path,const char *pre) {
	int r=1,len=strlen(pre);
	memset(path,0,len);
	path[0]='/';
	while(r<len) {
		printf("now r is %d,pre=%s\n",r,pre);
		int j=0;
		while(pre[r+j]!='/'&&r+j<len)
			j++;
		if(pre[r]=='.') {
			if(pre[r+1]=='.') {
				while(path[strlen(path)-1]!='/')
					path[strlen(path)-1]=0;
			}
		}
		else {
			if(path[strlen(path)-1]!='/')
				path[strlen(path)]='/';
			for(int i=0;i<j;i++)
				path[strlen(path)]=pre[r+i];
		}
		r+=j+1;
	}
}
void real_path(char *path,const char *tpath) {
 	task_t *cur =current_task();
	char rpath[110];
	memset(path,0,sizeof(char)*90);
	memset(rpath,0,sizeof(rpath));
	if(tpath[0]=='.') {
		strcat(rpath,cur->loc);
		if(path[strlen(rpath)-1]!='/')
			strcat(rpath,"/");
	}
	if(strlen(tpath)>2) {
		strcat(rpath,tpath);
		road_free(path,rpath);
	}
}
void cd_operation(const char * rpath) {
	char path[100];
	char tpath[100];
	memset(path,0,sizeof(path));
	memset(tpath,0,sizeof(tpath));
	task_t *cur=current_task();
	//real_path(path,rpath);
	if(rpath[0]=='.') {
		strcat(path,cur->loc);
		if(cur->loc[strlen(cur->loc)-1]!='/')
			strcat(path,"/");
	}
	strcat(path,rpath);
	printf("\033[42mcd path:now path is %s\033[0m\n",path);
	filesystem_t *fs=cur->preloc->fs;
	inode_t *pre=fs->ops->lookup(fs,path,7);
	pmm->free(cur->preloc);
	cur->preloc=pre;
	real_path(tpath,path);
	printf("\033[42mcd path:now real path is %s\033[0m\n",tpath);
	memset(cur->loc,0,sizeof(cur->loc));
	strcpy(cur->loc,tpath);
}
void mkdir_operation(const char *tpath) {
	task_t *cur =current_task();
	char path[100];
	memset(path,0,sizeof(path));
	if(tpath[0]=='.') {
		strcat(path,cur->loc);
		if(tpath[1]=='.') {
			while(path[strlen(path)-1]!='/')
				path[strlen(path)-1]=0;
			if(strlen(tpath)==2)
				path[strlen(path)-1]=0;
		}
		else if(strlen(tpath)!=1&&path[strlen(path)-1]!='/')
			strcat(path,"/");
	}
	strcat(path,tpath);

	vfs->mkdir(path);
}
void rmdir_operation(const char *tpath) {
	task_t *cur =current_task();
	char path[100];
	memset(path,0,sizeof(path));
 	if(tpath[0]=='.') {
		strcat(path,cur->loc);
 		if(tpath[1]=='.') {
			while(path[strlen(path)-1]!='/')
				path[strlen(path)-1]=0;
			if(strlen(tpath)==2)
				path[strlen(path)-1]=0;
		}
		else if(strlen(tpath)!=1&&path[strlen(path)-1]!='/')
			strcat(path,"/");
	}
	strcat(path,tpath);

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
			path[strlen(path)-1]=0;
		}
	    if(path[strlen(path)-1]!='/')
			strcat(path,"/");
	}
	strcat(path,tpath);
	char **pre=pmm->alloc(256);
	char rpath[100];
	memset(rpath,0,sizeof(rpath));    	
	strcpy(rpath,path);
	int fd=vfs->open(path,4);

	int end=vfs->lseek(fd,0,2);
	printf("\033[42m end=%d\033[0m\n",end);
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
		if(rpath[strlen(rpath)-1]!='/')
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
void link_op(const char *roldpath,const char *rnewpath) {
	char opath[100],npath[100];
	real_path(opath,roldpath);
	real_path(npath,rnewpath);
	vfs->link(opath,npath);
}
void unlink_op(const char *rpath){
	char path[100];
	real_path(path,rpath);
	vfs->unlink(path);
}
void read_op(int fd,void *buf,size_t size) {
	task_t *cur=current_task();
	if(cur->flides[fd]==NULL)
		assert(0);
	vfs->read(fd,buf,size);
}
void write_op(int fd,void *buf,size_t size) {
	task_t *cur=current_task();
	if(cur->flides[fd]==NULL)
		assert(0);
	vfs->write(fd,buf,size);
}
void lseek_op(int fd,off_t offset,int whence) {
	task_t *cur=current_task();
	if(cur->flides[fd]==NULL)
		assert(0);
	vfs->lseek(fd,offset,whence);
}
int cat_op(const char *name,char *buf) {
	char path[100];
	real_path(path,name);
	int fd=vfs->open(name,7);
	int sz=vfs->lseek(fd,0,2);
	vfs->lseek(fd,0,0);
	vfs->read(fd,buf,sz);
	vfs->close(fd);
	printf("cat op!\n");
	return sz;
}
void rm_op(const char *name) {
	char path[100];
	real_path(path,name);
	task_t *cur=current_task();
	filesystem_t *fs=cur->preloc->fs;
	fs->ops->lookup(fs,name,8);
}
int open_op(const char *name) {
	char path[100];
	//memset(path,0,sizeof(path));
	real_path(path,name);
	printf("open path =%s\n",path);
	return vfs->open(path,7);
}

