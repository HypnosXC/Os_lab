#include<stdio.h>
#include<assert.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<sys/mman.h>
char *start;
int BLO_NUM;
int BLO_SZ;
int FAT_NUM;
int FAT_BLO;
//void *fat1,fat2,data;
void init(void *start) {
	int jm=*((int *)(start+0x03));
	BLO_NUM=*((short*)(start+0x0e));
	BLO_SZ=*((short*)(start+0x0b));
	FAT_NUM=(int)(*((unsigned char *)(start+0x10)));
	FAT_BLO=*((int *)(start+0x24));
	printf("jmp=%x\n",jm);
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
}
