#include<common.h>
#include<devices.h>
#include<klib.h>
#define BLOCK_SIZE 2048
#define INODE_MAP_ENTRY 0
#define DATA_MAP_ENTRY 2048
#define INODE_ENTRY 4096
#define DATA_ENTRY (4096+2048*sizeof(inode_t))
#define FLSYS_NUM  10
/*        								using ext file system
 *									Using one block for blockbitmap, another for inode
 * 									only indirect pointer used (no double as well as direct)
 * 									for each inode , one data block used for ref information
 * 									head****inode_map****data_map****inode_block****data_block
 *										
 *									for directories , 128 b for its subdir as 120 for name and 8 
 *									its offset(num of bitmap)
 */
extern inodeops_t *inode_op;
filesystem_t fs_tab[FLSYS_NUM];
void new_block(inode_t* inode) {
	device_t *dev=inode->fs->dev;
	for(int i=0;i<BLOCK_SIZE*8;i++) {
		int pos=i/8;
		int loc=1<<(i%8-1);
		char realva;
		dev->ops->read(dev,DATA_MAP_ENTRY+pos,realva,sizeof(char));
		if(!(realva&loc)) {
			realva|=loc;
			dev->ops->write(dev,DATA_MAP_ENTRY+pos,realva,sizeof(char));
			off_t ptr=DATA_ENTRY+i*BLOCK_SIZE;
			pos=inode->size/BLOCK_SIZE;
			dev->ops->write(dev,DATA_MAP,(off_t)inode->ptr+pos*sizeof(off_t),&ptr,sizeof(off_t));
			inode->msize+=BLOCK_SIZE;
			return;
		}
	}
	printf("No avialiable block!\n");
	assert(0);
}
int inode_create(filesystem_t *fs,int prio,int type,inodeops_t *ops) {
	inode_t *pre=pmm->alloc(sizeof(inode_t));
	pre->fs=fs;
	pre->prio=prio;
	pre->type=type;
	pre->ops=ops;
	pre->size=BLOCK_SIZE;	
	int i=0;
	for(;i<BLOCK_SIZE*8;i++) {
		int pos=i/8;
		int loc=1<<(i%8-1);
		char realva;
		dev->ops->read(dev,DATA_MAP_ENTRY+pos,realva,sizeof(char));
		if(!(realva&loc)) {
			realva|=loc;
			dev->ops->write(dev,DATA_MAP_ENTRY+pos,realva,sizeof(char));
			pre->ptr=(void *)(DATA_ENTRY+i*BLOCK_SIZE);
			break;
		}
	}
	new_block(pre);
	return i;
}
void fs_init(filesystem_t *fs,const char *name,device_t *dev) {
	memcpy(fs->name,name,strlen(name));
	fs->dev=dev;
	int f=1;
	dev->ops->write(dev,INODE_MAP_ENTRY,&f,sizeof(f));
	dev->ops->write(dev,DATA_MAP_ENTRY,&f,sizeof(f));
	// inode for filesystem
	inode_t *s=inode_create(fs,4,0,inode_op);
	dev->ops->write(dev,INODE_ENTRY,&s,sizeof(inode_t));
	fs->inode=s;
	// one inode;
}
void del_map(device_t *dev,off_t entry,int num) {
	int pos=num/8;
	unsigned char va=1<<(num%8-1);
	unsigned char realval=0;
	dev->ops->read(dev,entry+pos,&realval,sizeof(char));
	if(!(realval&va)) {
		printf("map not recorded!,entry %x,location %d,\n",entry,num);
	}
	realval-=va;
	dev->ops->write(dev,entry+pos,&realval,sizeof(char));
}
int fs_close(inode_t *inode) {
	inode_t *pre=pmm->alloc(sizeof(inode_t));
	pre->ptr=NULL;
	device_t* dev=inode->fs->dev;
	int i=0;
	for(;i<BLOCK_SIZE;i++) {
			dev->ops->read(dev,INODE_ENTRY+i*sizeof(inode_t),pre,sizeof(inode_t));
			if(pre==NULL)
				continue;
			if(pre->ptr==inode->ptr)
				break;
	}
	if(pre->ptr==NULL) {
		printf("\033[32m wrong inode close!\n");
		assert(0);
	}
	// To change the inode bitmap
	del_map(dev,INODE_MAP_ENTRY,i);
	// To change the data bitmap
/*	if(pre->refptr!=NULL) {
		i=(int)(pre->refptr-DATA_ENTRY)/BLOCK_SIZE;
		del_map(dev,DATA_MAP_ENTRY,i);
	}*/
	void* *page=pmm->alloc(BLOCK_SIZE);
	dev->ops->read(dev,(off_t)pre->ptr,page,BLOCK_SIZE);
	for(int j=0;j<=128;j++) {
		if(page[j]==NULL)
			break;
		i=(int)(page[j]-DATA_ENTRY)/BLOCK_SIZE;
		del_map(dev,DATA_MAP_ENTRY,i);
	}
	i=(int)(pre->ptr-DATA_ENTRY)/BLOCK_SIZE;
	del_map(dev,DATA_MAP_ENTRY,i);
	pmm->free(pre);
	return 0;
}
off_t name_lookup(inode_t *inode,const char *name) {
	if(inode->type!=DIR_FILE) {// must be a directory inode
		printf("lookup in a non directory file!\n");
		assert(0);
	}
	device_t *dev=inode->fs->dev;
	off_t doff=0;
	char pname[124];
	off_t off=0;
	while(doff<inode->size) {
		basic_read(inode,doff,pname,100);
		if(!strcmp(pname,name)) {
			basic_read(inode,doff+112,off,sizeof(off_t));
			return off;
		}
		doff+=128;
	}
	printf("name %s no found!\n",name);
	return 1;
	assert(0);	
}
inode_t * fs_lookup(filesystem_t *fs,const char *path,int flags) {
	char name[100];
	inode_t *pre=pmm->alloc(BLOCK_SIZE);
	memcpy(pre,fs->inode,sizeof(inode_t));
	//int num;
	int i=0,l=strlen(path);
	device_t * dev=fs->dev;
	if(path[i]!='/') {
		printf("Not a correct path!\n");
		assert(0);
	}
	i++;
	while(path[i]!='/')
		i++;
	i++;
	while(i<l) {
		int j=0;
		while(path[i+j]!='/'&&i+j<l)
			j++;
		strncpy(name,path+i,j);
		off_t doff=name_lookup(pre,name);
		if (doff!=1) {
			dev->ops->read(dev,doff,pre,sizeof(inode_t));	
			i+=j;
		}
		else {
			if(pre->prio!=4) {//not a dir
				printf("wrong position,not a dir!\n");
				assert(0);
			}
			basic_write(pre,pre->size,name,100);
			pre->size+=12;
			int i=inode_create(fs,flags,(flags!=4),inode_op);
			off_t addr=INODE_ENTRY+i*sizeof(inode);
			basic_write(pre,pre->size,addr,sizeof(off_t));
			pre->size+=128-pre->size%128;
		}

	}
	return pre;
}
fsops_t fs_op = {
	.init=fs_init;
	.lookup=fs_lookup;
	.close=fs_close;
};
/* file sysytem finished
*
*
*
*
*
*
*
*
*
*
*
*
*
*
*
*
* vfs started!
*/
void vfs_init() {
	device_t *dev=dev_lookup("ramdisk0");
	fs_init(fs_tab[0],"/",dev);
	fs_tab[0]->ops=fs_op;
}
int vfs_access(const char *path,int mode){
	printf("access : TODO!\n");
	assert(0);
	return 0;
}
int vfs_mount(const char *path,filesystem_t *fs) {
	printf("mount : to do \n");
	assert(0);
	memncpy(fs->name,path,strlen(path));
	return 0;
}
int unmount(const char *path) {
	printf("unmount: todo\n");
	assert(0);
	return 0;
}
int vfs_mkdir(const char *path) {
	filesystem_t *fs=&fs_tab[0];
	fs->inode->ops->mkdir(path);
	return 0;
}
int vfs_rmdir(const char *path) {
	filesystem_t *fs=&fs_tab[0];
	fs->inode->ops->rmdir(path);
	return 0;
}
int vfs_open(char *path,int flags) {
	filesystem_t *fs=&fs_tab[0];
	inode_t *p=fs->ops->lookup(path,flags);
	file_t *file=pmm->alloc(sizeof(file_t));
	file->inode=p;
	file->offset=0;
	file->inode->ops->open(file,flags);
	return 0;
}
ssize_t vfs_read(int fd,void *buf,size_t size) {
	task_t * cur=current_task();
	file_t * file=cur->flides[fd];
	return file->inode->ops->read(file,buf,size);
}
ssize_t vfs_write(int fd,void *buf,size_t size) {
	task_t *cur=current_task();
	file_t file=cur->flides[fd];
	return file->inode->ops->write(file,buf,size);
}
off_t vfs_lseek(int fd,off_t offset,int whence) {
	task_t *cur=current_task();
	file_t *file=cur->flides[fd];
	file->inode->ops->lseek(file,offset,whence);
}
int vfs_close(int fd) {
	task_t *cur=current_task();
	file_t *file=cur->flides[fd];
	cur->flides[fd]=NULL;
	pmm->free(file->inode);
	pmm->free(file);
}
int vfs_link(const char *oldpath,const char *newpath) {
	task_t *cur=current_task();
	filesystem_t *fs=cur->preloc->fs;
	inode_t *pre=fs->ops->lookup(oldpath);
	return pre->ops->link(newpath,pre);
}
int vfs_unlink(const char *path) {
	task_t *cur=current_task();
	return cur->preloc->ops->unlink(path);
}
MODULE_DEF(vfs) {
	.init=vfs_init;
	.access=vfs_access;
	.mount=vfs_mount;
	.unmount=vfs_unmount;
	.mkdir=vfs_mkdir;
	.rmdir=vfs_rmdir;
	.link=vfs_link;
	.unlink=vfs_unlink;
	.open=vfs_open;
	.read=vfs_read;
	.write=vfs_write;
	.lseek=vfs_lseek;
	.close=vfs_close;
};


