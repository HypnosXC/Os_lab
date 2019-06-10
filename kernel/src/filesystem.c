#include<common.h>
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
 *
 */
extern inodeops_t *inodeops;
filesystem_t fs_tab[FLSYS_NUM];
void fs_init(filesystem_t *fs,const char *name,device_t *dev) {
	memcpy(fs->name,name,strlen(name));
	fs->dev=dev;
	int f=1;
	dev->ops->write(dev,INODE_MAP_ENTRY,&f,sizeof(f));
	dev->ops->wrtie(dev,DATA_MAP_ENTRY,&f,sizeof(f));
	// inode for filesystem
	inode_t s;
	//s.ops=inode_ops;
	s.size=0;
	s.ptr=(void *)DATA_ENTRY;
	s.fs=fs;
	s.refcnt=0;
	s.refptr=NULL;
	dev->write(dev,INODE_ENTRY,&s,sizeof(inode_t));
	// one inode;
}
void del_map(device_t *dev,off_t entry,int num) {
	int pos=num/8;
	unsigned char va=1<<(i%8-1);
	unsigned char realval=0;
	dev->ops->read(dev,entry+pos,realval,sizeof(char));
	realval-=va;
	dev->ops->write(dev,entry+pos,realval,sizeof(char));
}
int fs_close(inode_t *inode) {
	inode_t *pre=pmm->alloc(sizeof(inode_t));
	pre->ptr=NULL;
	device_t* dev=inode->fs->dev;
	int i=0;
	for(;i<BLOCK_SIZE;i++) {
			dev->ops->read(dev,INODE_ENTRY+i*sizeof(inode_t),pre,sizeof(inode_t));
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
	dev->read(dev,(off_t)pre->ptr,page,BLOCK_SIZE);
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
/*inode_t * fs_lookup(filesystem_t *fs,const char *path,int flags) {
	char name[100];
//	char *page=pmm->alloc(BLOCK_SIZE);
	//int num;
	int i=0,l=strlen(path);
	device_t * dev=fs->dev;
	inode_t *pre=pmm->alloc(sizeof(inode_t));
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
		i+=j;		
	}
}
*/
