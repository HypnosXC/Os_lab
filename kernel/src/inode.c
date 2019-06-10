#include<common.h>
#include<devices.h>
#include<klib.h>
#define BLOCK_SIZE 2048
spinlock_t *inode_lk;
int inode_open(file_t *file,int flags) {
	kmt->spin_lock(inode_lk);
	task_t* cur=current_task();
	int i=0;
	for(;i<=32;i++) {
		if(cur->flides[i]==NULL)
			break;
	}
	if(i==33)
		return -1;
	cur->flides[i]=file;
	return i;
	kmt->spin_unlock(inode_lk);
}
int inode_close(file_t* file) {
	kmt->spin_lock(inode_lk);
	task_t *cur=current_task();
	int i=0;
	for(;i<=32;i++)
		if(cur->flides[i]==file)
			break;
	if(i>32)
		return -1;
	cur->flides[i]=NULL;
	return i;
	kmt->spin_unlock(inode_lk);
}
off_t inode_lseek(file_t * file,off_t offset,int whence) {
	switch(whence){
	  case 0:
	  	file->offset=offset;
	  	break;
	  case 1:
		file->offset=0;
		break;
	  case 2:
	//	file->offset=file->size;
		break;
	}
	return file->offset;
}
void basic_read(inode_t *inode,off_t offset,char *buf,size_t size) {
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
			if(rsize>size)
				rsize=size;
			dev->ops->write(dev,page[i]+offset-doff,buf,rsize);
			size-=rsize;
			if(size>0)
				offset=doff+BLOCK_SIZE;
		}
	}
	pmm->free(page);	
}
void inode_read(file_t *file,char *buf,size_t size) {
	basic_read(file->inode,file->offset,buf,size);

}
void inode_write(file_t *file,const char *buf,size_t size) {
	basic_write(file->inode,file->offset,buf,size);
}
