#include<stdio.h>
#include<assert.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/mman.h>
void *start;
int BLO_NUM;
int BLO_SZ;
int BAS_GP;
int FAT_NUM;
int GP_BLO;
int FAT_BLO;
void* fat1;
void *fat2;
void *data;
struct file{
	wchar_t *name;
	void *start;
	int sz;
}fl_tab[10000];
wchar_t na[10][100];
int num;
void file_read(void *head) {
	int kd=(int)(*(char *)(head+0xB));
	int rk=(int)(*(char *)(head));
	if(kd==0xf&&rk!=6) {
		printf("Not the end of a file's name!\n");
		assert(0);
	}
	int tot=0;
	while(kd==0xf) {
		tot++;
		memcpy(na[tot],(head+0x1),10);
		memcpy(na[tot]+10,(head+0xe),12);
		memcpy(na[tot]+22,(head+0x1c),4);
		printf("name is %ls\n",na[tot]);
		head+=0x20;
		kd=(int)(*(char *)(head+0x8));
	}
	int pos=*((short *)(head+0x14));
	pos=(pos<<16)+*((short*)(head+0x1a));
	fl_tab[num].start=fat2+(pos-2)*GP_BLO*BLO_SZ;
	fl_tab[num].sz=*((int *)(head+0x1c));
	fl_tab[num].name=malloc(sizeof(char)*13*tot+8);
	memset(fl_tab[num].name,0,sizeof(char)*100);
	memcpy(fl_tab[num].name,head,8);
	num++;
	printf("got file:%ls,offset=%x\n",fl_tab[num-1].name,(int)(fl_tab[num-1].start-start));
}
void init(void *start) {
	BLO_NUM=*((short*)(start+0x0e));
	BLO_SZ=*((short*)(start+0x0b));
	FAT_NUM=(int)(*((unsigned char *)(start+0x10)));
	FAT_BLO=*((int *)(start+0x24));
	BAS_GP=*((int *)(start+0x2c));
	GP_BLO=(int)(*((unsigned char *)(start+0x0d)));
	fat1=start+BLO_NUM*BLO_SZ;
	fat2=fat1+FAT_BLO*BLO_SZ;
	data=fat2+FAT_BLO*BLO_SZ+(BAS_GP-2)*GP_BLO*BLO_SZ;
	printf("f1=%x,f2=%x,data=%x\n",(int)(fat1-start),(int)(fat2-start),(int)(data-start));
}
int main(int argc, char *argv[]) {
  int p=open(argv[1],O_RDONLY);
  if(p==-1) {
  	printf("\033[32m No such file to open! \033[0m \n");
	assert(0);
  } 
  int size=lseek(p,0,SEEK_END)-lseek(p,0,SEEK_SET);
  start=mmap(NULL,size,PROT_READ,MAP_SHARED,p,0);
  init(start);
  void *head=data;
  int i=1;
  char pre[10];
  while(1) {
	  i++;
	  if(i>10000000)
		  break;
	 printf("i=%d,head=%x\n",i,(int)(head-start));
	 memset(pre,0,sizeof(pre));
	 memcpy(pre,(head+0x8),3);
	 printf("%s\n",pre);
	 int kd=(int )*((char *)(head+0xb));
	 int tail=(int)*((char *)(head));
	 if((!strcmp(pre,"BMP"))||(kd==0xf&&tail==6))
		 file_read(head);
	 head+=32;
  }
}
