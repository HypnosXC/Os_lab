#include<stdio.h>
#include<assert.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
char *start;
int main(int argc, char *argv[]) {
  int p=open(argv[0],O_RDONLY);
  if(p==-1) {
  	printf("\033[32m No such file to open! \033[0m \n");
	assert(0);
  }
  int size=lseek(p,0,SEEK_END)-lseek(p,0,SEEK_SET);
  start=alloc(size*sizeof(char));
  start=mmap(NULL,size,PROT_READ,MAP_SHARD,p,0);
  printf("%s,%d\n",(start+0x52),size);
}
