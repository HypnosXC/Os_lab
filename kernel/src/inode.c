#include<common.h>
#include<devices.h>
#include<klib.h>
spinlock_t *inode_lk;
extern filesystem_t fs_tab[];
extern task_t *loader[];
char cpu_info[100],mem_info[100];
void info_update() {
	memset(cpu_info,0,sizeof(cpu_info));
	sprintf(cpu_info,"Total CPU num:%d \n Running cpu:%d",_ncpu(),_cpu());
	int icnt=0,dcnt=0;
	device_t *dev=dev_lookup("ramdisk1");
	for(int i=0;i<BLOCK_SIZE;i++) {
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
	memset(mem_info,0,sizeof(mem_info));
	sprintf(mem_info,"Now inode used : %d blocks \n Data used : %d blocks!",icnt,dcnt);
	printf("Information is %s\n%s\n,%p,%p\n",cpu_info,mem_info,cpu_info,mem_info);

}
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
//	printf("\033[42m basic_read : offset is %d,%d\033[0m\n",offset,size);
	device_t * dev=inode->fs->dev;
	//printf("")
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
//	printf("\033[42m basic:write : offset is %d,%d\033[0m\n",offset,size);
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
	char tname[100];
	memcpy(tname,name,strlen(name));
	memset(pname,0,sizeof(pname));
	int len=strlen(name);
	int l=len-1;
	while(name[l]!='/')
		l--;
	if(name[l]=='/')
		l++;
	strcpy(pname,name+l);
	l=len-1;
	while(tname[l]!='/')
		tname[l--]=0;
	if(tname[l]=='/'&&l!=0)
		tname[l--]=0;
	printf("\n...%s    %s....\n",tname,pname);
	inode_t *pre=inode->fs->ops->lookup(inode->fs,tname,9);
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
ssize_t devfs_read(file_t *file,char *buf,size_t size) {
	printf("defv read!\n");
	if(file->inode->prio==3) {
		printf("\033[42mm ramdisk!\033[0m\n");
		device_t *dev=(device_t *) file->inode->ptr;
		ssize_t nread=dev->ops->read(dev,file->offset,buf,size);
		file->offset+=size;
		return nread;
	}
	else if(file->inode->prio==0) {//rand
		for(int i=0;i<size;i++)
			buf[i]=rand()%256;
	}
	else if(file->inode->prio==1) {
		for(int i=0;i<size;i++)
			buf[i]=0;
	}
	else  {
		printf("\033[41mOperation not supported!\033[0m\n");
		assert(0);
	}
	return size;
}
ssize_t devfs_write(file_t *file,const char *buf,size_t size) {
	if(file->inode->prio==3){//dev
		device_t *dev=(device_t *)file->inode->ptr;
		ssize_t nread=dev->ops->write(dev,file->offset,buf,size);
		file->offset+=size;
		return nread;
	} 
	else if(file->inode->prio==2) {
		return size;
	} 
	else { 
		printf("\033[41mOperation not supported!\033[0m\n");
		assert(0);
	}
}
//aimed at type=0 1 2 3
char taskinfo[100];
ssize_t proc_read(file_t *file,char *buf,size_t size) {
	kmt->spin_lock(inode_lk);
	info_update();
	ssize_t len=-1;
	printf("\033[42mgot here proc_read!\n");
	if(file->inode->type==4) {
		memcpy(buf,cpu_info,100);
		len=strlen(cpu_info);
		//strcpy(buf,u_ha?");
 	}
 	else if(file->inode->type==5) {
		memcpy(buf,mem_info,100);
		len=strlen(mem_info);
	}
	else {
		int pos=(int)file->inode->ptr;
		memset(taskinfo,0,sizeof(taskinfo));
		if(loader[pos]!=NULL)
			sprintf(taskinfo,"Task %d : name :%s\n running :   %d\n",pos,loader[pos]->name,loader[pos]->state==2);	
		else
			sprintf(taskinfo,"No such a task!\n");
		memcpy(buf,taskinfo,size);
		len=strlen(taskinfo);
 	}
	kmt->spin_unlock(inode_lk);
	return len;
}
ssize_t proc_write(file_t *file,const char *buf,size_t size) {
	printf("\033[41m Permission denied !\n\033[0m");
	return 0;
}

//aimed at type=5 6
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
inodeops_t dev_ops= {
	.open=inode_open,
	.close=inode_close,
	.read=devfs_read,
	.write=devfs_write,
	.lseek=inode_lseek,
	.mkdir=mkdir,
	.rmdir=rmdir,
	.link=link,
	.unlink=unlink,
};
inodeops_t proc_ops= {
	.open=inode_open,
	.close=inode_close,
	.read=proc_read,
	.write=proc_write,
	.lseek=inode_lseek,
	.mkdir=mkdir,
	.rmdir=rmdir,
	.link=link,
	.unlink=unlink,
};

