#include<common.h>
#include<devices.h>
#include<klib.h>
#define BLOCK_SIZE 2048
spinlock_t *inode_lk;
extern filesystem_t fs_tab[];
void self_fetch(inode_t *inode) {
	device_t *dev=inode->fs->dev;
	dev->ops->read(dev,inode->pos,inode,sizeof(inode_t));
}
void self_update(inode_t *inode) {
	device_t *dev=inode->fs->dev;
	dev->ops->write(dev,inode->pos,inode,sizeof(inode_t));
}
int inode_open(file_t *file,int flags) {
	kmt->spin_lock(inode_lk);
	task_t* cur=current_task();
	int i=0;
	for(;i<=32;i++) {
		if(cur->flides[i]==NULL)
			break;
	} 
	if(i<32)
		cur->flides[i]=file;
	else 
		i=-1;
	kmt->spin_unlock(inode_lk);
	return i;
}
int inode_close(file_t* file) {
	kmt->spin_lock(inode_lk);
	task_t *cur=current_task();
	int i=0;
	for(;i<=32;i++)
		if(cur->flides[i]==file)
			break;
	if(i>32) {
		kmt->spin_unlock(inode_lk);
		return -1;
	}
	cur->flides[i]=NULL;
	kmt->spin_unlock(inode_lk);
	return i;
}
off_t inode_lseek(file_t * file,off_t offset,int whence) {
	kmt->spin_lock(inode_lk);
	self_fetch(file->inode);
	switch(whence){
	  case 0:
	  	file->offset=offset;
	  	break;
	  case 1:
		file->offset=0;
		break;
	  case 2:
		file->offset=file->inode->size;
		break;
	}
	kmt->spin_unlock(inode_lk);
	return file->offset;
}
void basic_read(inode_t *inode,off_t offset,char *buf,size_t size) {
	self_fetch(inode);
	device_t * dev=inode->fs->dev;
	off_t doff=0;
	int i=0;
	off_t *page=pmm->alloc(BLOCK_SIZE);
	void *ps=pmm->alloc(BLOCK_SIZE);
	dev->ops->read(dev,(off_t)inode->ptr,page,BLOCK_SIZE);
	while(size ) {
		if(doff+BLOCK_SIZE<=offset) {
			i++;
		} 
		else {
			off_t rsize=doff+BLOCK_SIZE-offset;
			if(rsize>size)
				rsize=size;
			dev->ops->read(dev,page[i],ps,BLOCK_SIZE);
			memcpy(buf,ps+offset-doff,rsize);
			size-=rsize;
			if(size>0)
				offset=doff+BLOCK_SIZE;
		} 
 	}	
 	pmm->free(page);
	pmm->free(ps);
}
void basic_write(inode_t *inode,off_t offset,const char* buf,size_t size){
	self_fetch(inode);
	if(offset+size>inode->size)
		inode->size=offset+size;
//	printf("%d,%d",offset,size);
	while(inode->msize<offset+size) {
		printf("size=%d ",inode->msize);
		new_block(inode);
	}
	device_t * dev=inode->fs->dev;
	off_t doff=0;
	int i=0;
	off_t *page=pmm->alloc(BLOCK_SIZE);
	dev->ops->read(dev,(off_t)inode->ptr,page,BLOCK_SIZE);
	while(size) {
		if(doff +BLOCK_SIZE<=offset) {
			i++;
		}
		else {
			off_t rsize=doff+BLOCK_SIZE-offset;
			printf("\033[42m basic:write : offset is %d\033[0m\n",page[i]+offset-doff);
			if(rsize>size)
				rsize=size;
			dev->ops->write(dev,page[i]+offset-doff,buf,rsize);
			size-=rsize;
			if(size>0)
				offset=doff+BLOCK_SIZE;
		}
	}
	pmm->free(page);
	self_update(inode);	
}
ssize_t inode_read(file_t *file,char *buf,size_t size) {
	kmt->spin_lock(inode_lk);
	basic_read(file->inode,file->offset,buf,size);
	kmt->spin_unlock(inode_lk);
	return size;
}
ssize_t inode_write(file_t *file,const char *buf,size_t size) {
	kmt->spin_lock(inode_lk);
	basic_write(file->inode,file->offset,buf,size);
	kmt->spin_unlock(inode_lk);
	return size;
}
int mkdir(const char *name) {
	task_t * cur=current_task();
	cur->preloc->fs->ops->lookup(cur->preloc->fs,name,4);
	return 0;
}
int rmdir(const char *name) {
	task_t *cur=current_task();
	cur->preloc->fs->ops->lookup(cur->preloc->fs,name,8);	
	return 0;
}
int link(const char *name,inode_t *inode) {
	char pname[100];
	int len=strlen(name);
	int l=len-1;
	while(name[l]!='/')
		l--;
	l++;
	strcpy(pname,name+l);
	while(l<len)
		name[l++]=0;
	inode_t *pre=inode->fs->lookup(inode->fs,name,9);
	add_inode(pre,pname,inode);
	pmm->free(pre);
	return 0;
}
int unlink(const char *name) {
	char fname[100];
	memset(fname,0,sizeof(fname));
	sprintf(fname,"%s%s",name,"/..");
	inode_t *pre=fs_tab[0].ops->lookup(&fs_tab[0],fname,9);
	inode_t *go=fs_tab[0].ops->lookup(&fs_tab[0],name,9);
	int doff=0;
	off_t ptr=0;
	while(doff<pre->size) {
		basic_read(pre,doff+112,(char *)&ptr,sizeof(off_t));
		if(ptr==go->pos) {
			ptr=-1;
			basic_write(pre,doff+112,(char *)&ptr,sizeof(off_t));
			break;
		}
		doff+=128;
	}
	pmm->free(pre);
	pmm->free(go);
	return  0;
}
inodeops_t inode_op = {
	.open=inode_open,
	.close=inode_close,
	.read=inode_read,
	.write=inode_write,
	.lseek=inode_lseek,
	.mkdir=mkdir,
	.rmdir=rmdir,
	.link=link,
	.unlink=unlink,
};
