#include<common.h>
#include<klib.h>
spinlock_t *inode_lk;
int open(file_t *file,int flags) {
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
int close(file_t* file) {
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
off_t lseek(file_t * file,off_t offset,int whence) {
	switch(whence){
	  0:
	  	file->offset=offset;
	  1:
		file->offset=0;
	  2:
		file->offset=file->size;
	}
	return file->offset;
}

