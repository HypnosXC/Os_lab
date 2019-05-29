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
	wchar_t name[200];
	char *na;
	void *start;
	int sz;
}fl_tab[10000];
char na[10][100];
int num;
void* namcpy(void *bbuf,const void*iin,int sz) {
	char * buf=(char *)bbuf;
	char * in=(char *)iin;
	for(int i=0;i<sz/2;i++) {
		*(buf+i)=*(in+2*i);
	}
	printf("%s",buf);
	return buf+sz;
}	
void* file_read(void *head) {
	int kd=(int)(*(char *)(head+0xB));
	if(kd!=0xf) {
		printf("Not the end of a file's name!\n");
		fflush(stdout);
		assert(0);
	}
	int tot=0;
	memset(na[1],0,sizeof(char)*100);
	memset(na[0],0,sizeof(char)*100);
	char *p=na[0];
	char *np=na[1];
	while(kd==0xf) {
		tot++;
		/*
		memcpy(na[tot],(head+0x1),10);
		memcpy(na[tot]+10,(head+0xe),12);
		memcpy(na[tot]+22,(head+0x1c),4);
		?????????
		*/
		p=namcpy(p,head+1,10);
		p=namcpy(p,head+0xe,12);
		p=namcpy(p,head+0x1c,4);
		np=strcat(np,p);
		p=na[0];
		memset(na[0],0,sizeof(char)*100);
		printf("%s\n",np);
		np=p;
		head+=0x20;
		kd=(int)(*(char *)(head+0xb));
	}
	fl_tab[num].na=(char *)p;
	char pre[100];
	memset(pre,0,sizeof(pre));
	memcpy(pre,head+0x8,3);
	if(!strcmp(pre,"BMP")) {
		int pos=*((short *)(head+0x14));
		pos=(pos<<16)+*((short*)(head+0x1a));
		fl_tab[num].start=fat2+(pos-2)*GP_BLO*BLO_SZ;
		fl_tab[num].sz=*((int *)(head+0x1c));
		num++;
		printf("got file:");
		for(int i=0;i<10;i++)
			printf("fk %s",p);
		wcstombs(pre,fl_tab[num-1].name,90);
		printf("head=%x,%s,offset=%x\n",(int)(head-start),fl_tab[num-1].na,(int)(fl_tab[num-1].start-start));
	}
	return head+32;
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
	  if(i>100000)
		  break;
//	 printf("i=%d,head=%x\n",i,(int)(head-start));
	 memset(pre,0,sizeof(pre));
	 memcpy(pre,(head+0x8),3);
	 int kd=(int )*((char *)(head+0xb));
	 int tail=(int)*((char *)head);
//	 printf("kd=%s,%d\n",pre,kd);
	 if(kd==0xf&&(tail&64))
		head=file_read(head);
	 else
	 	head+=32;
  }
}
