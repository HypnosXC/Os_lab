#include<common.h>
#include<devices.h>
#include<klib.h>
#define BLOCK_SIZE (off_t)2048
#define INODE_MAP_ENTRY (off_t)0
#define DATA_MAP_ENTRY (off_t)2048
#define INODE_ENTRY (off_t)4096
#define DATA_ENTRY (off_t)(4096+2048*sizeof(inode_t))
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
extern inodeops_t inode_op,dev_ops,proc_ops;
extern spinlock_t *inode_lk;
spinlock_t *fs_lk;
filesystem_t fs_tab[FLSYS_NUM];
char empty_BLOCK[BLOCK_SIZE*2];
char cpuinfo[100],meminfo[100];
void info_update() {
	memset(cpuinfo,0,sizeof(cpuinfo));
	sprintf(cpuinfo,"Total CPU num:%d\nRunning cpu:%d\n",_npu(),_cpu());
	int icnt=0,dcnt=0;
	device_t *dev=dev_lookup("ramdisk0");
	for(int i=0;i<2048;i++) {
		int loc=1<<(i%8);
		int pos=i/8;
		char realva=0;
		dev->ops->read(dev,INODE_MAP_ENTRY+pos,&realva,1);
		if(realva&loc)
			icnt++;
		dev->ops->read(dev,DATA_MAP_ENTRY+pos,&realva,1);
		if(realva&loc)
			dcnt++;
	}
	memset(meminfo,0,sizeof(meminfo));
	sprintf(meminfo,"Now inode used %d blocks and data used %d blocks!\n",icnt,dcnt);
}
void new_block(inode_t* inode) {
	device_t *dev=inode->fs->dev;
	for(int i=0;i<BLOCK_SIZE*8;i++) {
		int pos=i/8;
		int loc=1<<(i%8);
		char realva;
		dev->ops->read(dev,DATA_MAP_ENTRY+pos,&realva,sizeof(char));
		if(!(realva&loc)) {
	//		printf("\033[32m block %d used,realva=%d!\n\033[0m",i,realva|=loc);
			realva|=loc;
			dev->ops->write(dev,DATA_MAP_ENTRY+pos,&realva,sizeof(char));
			dev->ops->write(dev,DATA_ENTRY+i*BLOCK_SIZE,empty_BLOCK,BLOCK_SIZE);
			off_t ptr=DATA_ENTRY+i*BLOCK_SIZE;
			pos=inode->msize/BLOCK_SIZE;
			printf("\033[42m new_block :%d stored block at %d now used\033[43m",pos,ptr);
			dev->ops->write(dev,(off_t)inode->ptr+pos*sizeof(off_t),&ptr,sizeof(off_t));
			inode->msize+=BLOCK_SIZE;
			return;
	 	}
	} 
	printf("No avialiable block!\n");
	assert(0);
}
void add_inode(inode_t* dir,const char *name,inode_t *fl) {
	if(dir->type!=0) {
		printf("\033[42m Mkdir in a nondirectory!\033[0m\n");
		assert(0);
	}
	char pname[130];
	memset(pname,0,sizeof(pname));
	memcpy(pname,name,strlen(name));
	basic_write(dir,dir->size,pname,112);
	basic_write(dir,dir->size,(char *)&fl->pos,sizeof(off_t));
	int ff=0;
	basic_read(dir,dir->size-4,(char*)&ff,sizeof(off_t));
	printf("\033[42m add_inode: originally size=%d,name=%s\033,off %d=%d[0m\n",dir->size,pname,ff,fl->pos);
	basic_write(dir,dir->size,pname+112,128-dir->size%128);
	printf("\033[42m add_inode Now size is %d\033[0m\n",dir->size);
}/*
int inode_ex(off_t offset,filesystem_t *fs){
	int num=(offset-INODE_ENTRY)/sizeof(inode_t);
	char realva=0;
	device_t *dev=fs->dev;
	dev->ops->read(dev,INODE_MAP_ENTRY+num/8,&realva,sizeof(char));
	printf("num=%d,realva=%d\n",num,realva);
	int f=(realva&(1<<(num%8)));
	if ( f != 0) 
		return 1;
	return 0;
}*/
off_t name_lookup(inode_t *inode,const char *name) {
	self_fetch(inode);
	if(inode->type!=0) {// must be a directory inode
		printf("lookup in a non directory file!\n");
		assert(0);
 	}
	off_t doff=0;
	char pname[124];
	off_t off=0,ioff=0;
	printf("now inode size is %d\n",inode->size);
 	while(doff<inode->size) {
		basic_read(inode,doff,pname,100);
		basic_read(inode,doff+112,(char *)&ioff,sizeof(off_t));
		if(ioff==-1){
			doff+=128;
			continue;
		}
		printf("get name as %s\n",pname);
 		if(!strcmp(pname,name)) {
			basic_read(inode,doff+112,(char *)&off,sizeof(off_t));
			return off;
		}
		doff+=128;
		printf("doff=%d\n",doff);
	}
	printf("name %s no found!\n",name);
	return 1;
	assert(0);	
}

int inode_create(filesystem_t *fs,int prio,int type,inodeops_t *ops) {
	inode_t *pre=pmm->alloc(sizeof(inode_t));
	device_t *dev=fs->dev;
	pre->fs=fs;
	pre->prio=prio;
	pre->type=type;
	pre->ops=ops;
	pre->size=0;
	pre->msize=0;	
	int i=0;
	for(;i<BLOCK_SIZE*8;i++) {
		int pos=i/8;
		int loc=1<<(i%8);
		char realva;
		dev->ops->read(dev,DATA_MAP_ENTRY+pos,&realva,sizeof(char));	
		if(!(realva&loc)) {	
//			printf("At %d , now mark is %d\n",i,realva);
			realva|=loc;
			dev->ops->write(dev,DATA_MAP_ENTRY+pos,&realva,sizeof(char));
			pre->ptr=(void *)(DATA_ENTRY+i*BLOCK_SIZE);
	//		printf("pre is %p\n",pre->ptr);
			break;
		}
	}
	new_block(pre);
	for(i=0;i<BLOCK_SIZE;i++) {
		int pos=i/8;
		int loc=1<<(i%8);
		char realva;
		dev->ops->read(dev,INODE_MAP_ENTRY+pos,&realva,sizeof(char));
	 	if(!(realva&loc)) {
			realva|=loc;
			printf("At %d, now mark is %d\n",i,realva);
			dev->ops->write(dev,INODE_MAP_ENTRY+pos,&realva,sizeof(char));
			pre->pos=INODE_ENTRY+i*sizeof(inode_t);
			dev->ops->write(dev,INODE_ENTRY+i*sizeof(inode_t),(char *)pre,sizeof(inode_t));
			dev->ops->read(dev,INODE_ENTRY+i*sizeof(inode_t),pre,sizeof(inode_t));
	//		printf("now pre is %p\n",pre->ptr);
			break;
	 	}
	}  
	printf("created finished, inode is %p!\n",pre->pos);
	pmm->free(pre);
	return i;
}
int dev_create(filesystem_t *fs,int prio,int type,inodeops_t *ops,char *name) {
	inode_t *pre=pmm->alloc(sizeof(inode_t));
	int num=inode_create(fs,prio,type,ops);
	int off=INODE_ENTRY+sizeof(inode_t)*num;
	device_t *dev=fs->dev;
	dev->ops->read(dev,off,pre,sizeof(inode_t));
	pre->size=100;
	if(prio==3) {
		device_t *ndev=dev_lookup(name);
		pre->ptr=(void *)ndev;
	}
	else 
		pre->ptr=NULL;
	self_update(pre);
	pmm->free(pre);
	return num;
}
int proc_create(filesystem_t *fs,int prio,int type,inodeops_t *ops,char *name) {
	inode_t *pre=pmm->alloc(sizeof(inode_t));
	int num=inode_create(fs,prio,type,ops);
	int off=INODE_ENTRY+sizeof(inode_t)*num;
	device_t *dev=fs->dev;
	dev->ops->read(dev,off,pre,sizeof(inode_t));
	pre->size=100;
	if(name!=NULL) {
		if(name[0]=='c')//cpuinfo
			pre->ptr=(void *)cpuinfo;
		else if(name[0]=='m')
			pre->ptr=(void *)meminfo;
		else {
			int f=0;
			char *p=pre->ptr;
			while('0'<=*p&&*p<='9') {
				f=f*10+*p-'0';
				p++;
			}
			pre->ptr=(void *)f;
		}
	}
	else
		pre->ptr=NULL;
	self_update(pre);
	pmm->free(pmm);
	return num;
}
void del_map(device_t *dev,off_t entry,int num) {
	printf("\033[34m block %x,%d realsed !\n\033[0m",entry,num);
	int pos=num/8;
	unsigned char va=1<<(num%8);
	unsigned char realval=0;
	dev->ops->read(dev,entry+pos,&realval,sizeof(char));
	if(!(realval&va)) {
		printf("map not recorded!,entry %x,location %d,\n",entry,num);
	}
	realval-=va;
	dev->ops->write(dev,entry+pos,&realval,sizeof(char));
}
int in_close(inode_t *pre) {
	printf("\033[42m DELETE Inode !\033[0m\n");
	device_t* dev=pre->fs->dev;
	int i=(pre->pos-INODE_ENTRY)/sizeof(inode_t);
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
	off_t *page=pmm->alloc(BLOCK_SIZE);
	dev->ops->read(dev,(off_t)pre->ptr,page,BLOCK_SIZE);
	for(int j=0;j<pre->msize/BLOCK_SIZE;j++) {
		i=(int)(page[j]-DATA_ENTRY)/BLOCK_SIZE;
		del_map(dev,DATA_MAP_ENTRY,i);
	}
	i=(int)(pre->ptr-DATA_ENTRY)/BLOCK_SIZE;
	del_map(dev,DATA_MAP_ENTRY,i);
	inode_t *mpre=pmm->alloc(sizeof(inode_t));
	off_t poff=name_lookup(pre,"..");
	dev->ops->read(dev,poff,mpre,sizeof(inode_t));
	printf("\033[42m inode_close: pre inode is %p,size=%p\033[0m\n",poff,mpre->pos);
	off_t doff=0,ioff=0;
	while(doff<=mpre->size) {
		basic_read(mpre,doff+112,(char *)&ioff,sizeof(off_t));
		if(ioff==pre->pos) {
			ioff=-1;
			basic_write(mpre,doff+112,(char *)&ioff,sizeof(int));
			break;
		}
		doff+=128;
	}
	pmm->free(pre);
	pmm->free(mpre);
	return 0;
}
void fs_init(filesystem_t *fs,const char *name,device_t *dev) {
	memcpy(fs->name,name,strlen(name));
	fs->dev=dev;
	for(int i=0;i<2048;i++) {
		char f=0;
		dev->ops->write(dev,INODE_MAP_ENTRY+i,&f,sizeof(char));
		dev->ops->write(dev,DATA_MAP_ENTRY+i,&f,sizeof(char));
	}
	// inode for filesystem
	int i=inode_create(fs,4,0,&inode_op);
	inode_t *pre=pmm->alloc(sizeof(inode_t));
	dev->ops->read(dev,INODE_ENTRY+i*sizeof(inode_t),pre,sizeof(inode_t));
	add_inode(pre,".",pre);
	add_inode(pre,"..",pre);
	fs->inode=pre;
	printf("pre is %p\n",pre->ptr);
	// one inode;
}

int fs_close(inode_t *inode ){
	kmt->spin_lock(fs_lk);
	int ret=in_close(inode);
	kmt->spin_unlock(fs_lk);
	return ret;
}
// 1-7 file flags ,8 - delete file,9 cd/find,10 unlink 
inode_t * fs_lookup(filesystem_t *fs,const char *path,int flags) {
	kmt->spin_lock(fs_lk);
	char name[100];
	inode_t *pre=pmm->alloc(sizeof(inode_t));
	inode_t *mpre=pmm->alloc(sizeof(inode_t));
	memcpy(pre,fs->inode,sizeof(inode_t));
	//int num;
	int i=0,l=strlen(path);
	device_t * dev=fs->dev;
	printf("\033[43m Lookup: name=%s\033[0m\n",path);
	if(path[i]!='/') {
		printf("Not a correct path!\n");
		assert(0);
	}
	i++;
	while(i<l) {
		memset(name,0,strlen(name));
		int j=0;
		while(path[i+j]!='/'&&i+j<l)
			j++;
		printf("now name is %s,node is %p\n",name,pre->ptr);
		strncpy(name,path+i,j);
		off_t doff=name_lookup(pre,name);
		if (doff!=1) {	
			dev->ops->read(dev,doff,pre,sizeof(inode_t));	
			if(i+j>=l&&flags==8) {
				in_close(pre);
			}
		}
		else {
			if(pre->prio!=4||flags==8) {//not a dir
				printf("wrong position,not a dir!\n");
				assert(0);
			}
			if(flags==9) {
				printf("No such a file or directory!,i+j=%d,l=%d,path=%s\n",i+j,l,path);
				assert(0);
			}
			int num=0;
			if(flags==4||flags=7)
				num=inode_create(fs,flags,(flags!=4),&inode_op);
			if(0<=flags&&flags<=3)
				num=dev_create(fs,flags,(flags!=4),&dev_ops,name);
			if(flags==5||flags==6)
				num=proc_create(fs,flags,(flags!=4),&proc_ops,name);
			printf("\033[42 m new block is %d\033[0m\n",num);
			off_t addr=INODE_ENTRY+num*sizeof(inode_t);
			memcpy(mpre,pre,sizeof(inode_t));
			dev->ops->read(dev,addr,pre,sizeof(inode_t));
			add_inode(mpre,name,pre);
			if(pre->type==0) {
				add_inode(pre,"..",mpre);
				add_inode(pre,".",pre);
			}
			if(i+j<l) {
				printf("\033[42m fs_lookup,wrong dir happened!\033[0m,l=%d,i=%d,j=%d\n",l,i,j);
				assert(0);
			} 
			break;
		}
		i+=j+1;
	}
	pmm->free(mpre);
	kmt->spin_unlock(fs_lk);
	return pre;
}
fsops_t fs_op = {
	.init=fs_init,
	.lookup=fs_lookup,
	.close=fs_close,
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
	inode_lk=pmm->alloc(sizeof(spinlock_t));
	fs_lk=pmm->alloc(sizeof(spinlock_t));
	kmt->spin_init(inode_lk,"inode");
	kmt->spin_init(fs_lk,"file system");
	device_t *dev=dev_lookup("ramdisk0");
	filesystem_t *fs=&fs_tab[0];
	fs_init(&fs_tab[0],"/",dev);
	fs_lookup(fs,"/proc",4);
	fs_lookup(fs,"/dev",4);
	fs_lookup(fs,"/dev/ramdisk1",3);
	fs_lookup(fs,"/dev/null",2);
	fs_lookup(fs,"/dev/zero",1);
	fs_lookup(fs,"/dev/rand",0);
	fs_lookup(fs,"/proc/cpuinfo",5);
	fs_lookup(fs,"/proc/meminfo",5);
	fs_tab[0].ops=&fs_op;
	printf("\033[42m where dead?\033[0m\n");
}
int vfs_access(const char *path,int mode){
	printf("access : TODO!\n");
	assert(0);
	return 0;
}
int vfs_mount(const char *path,filesystem_t *fs) {
	printf("mount : to do \n");
	assert(0);
	memcpy(fs->name,path,strlen(path));
	return 0;
}
int vfs_unmount(const char *path) {
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
int vfs_open(const char *path,int flags) {
	filesystem_t *fs=&fs_tab[0];
	if(path[1]=='p'&&path[2]=='r'&&path[3]=='o'&&path[4]=='c'&&path[5]=='/')
		fs=&fs_tab[1];
	else if(path[1]=='d'&&path[2]=='e'&&path[3]=='v'&&path[4]=='/')
		fs=&fs_tab[2];
	inode_t *p=fs->ops->lookup(fs,path,flags);
	file_t *file=pmm->alloc(sizeof(file_t));
	file->inode=p;
	file->offset=0;
	return file->inode->ops->open(file,flags);
}
ssize_t vfs_read(int fd,void *buf,size_t size) {
	task_t * cur=current_task();
	file_t * file=cur->flides[fd];
	memset(buf,0,size);
	ssize_t ret=file->inode->ops->read(file,buf,size);
	char *bf=(char *)buf;
	for(int i=0;i<size;i++)
		printf("\033[45m %c\033[0m",bf[i]);
	file->offset+=size;
	return ret;
}
ssize_t vfs_write(int fd,void *buf,size_t size) {
	task_t *cur=current_task();
	file_t *file=cur->flides[fd];
	ssize_t ret=file->inode->ops->write(file,buf,size);
	file->offset+=size;
	return ret;
}
off_t vfs_lseek(int fd,off_t offset,int whence) {
	task_t *cur=current_task();
	file_t *file=cur->flides[fd];
	return file->inode->ops->lseek(file,offset,whence);
}
int vfs_close(int fd) {
	task_t *cur=current_task();
	file_t *file=cur->flides[fd];
	cur->flides[fd]=NULL;
	pmm->free(file->inode);
	pmm->free(file);
	return 0;
}
int vfs_link(const char *oldpath,const char *newpath) {
	task_t *cur=current_task();
	filesystem_t *fs=cur->preloc->fs;
	inode_t *pre=fs->ops->lookup(fs,oldpath,7);
	return pre->ops->link(newpath,pre);
}
int vfs_unlink(const char *path) {
	task_t *cur=current_task();
	return cur->preloc->ops->unlink(path);
}
MODULE_DEF(vfs) {
	.init=vfs_init,
	.access=vfs_access,
	.mount=vfs_mount,
	.unmount=vfs_unmount,
	.mkdir=vfs_mkdir,
	.rmdir=vfs_rmdir,
	.link=vfs_link,
	.unlink=vfs_unlink,
	.open=vfs_open,
	.read=vfs_read,
	.write=vfs_write,
	.lseek=vfs_lseek,
	.close=vfs_close,
};


