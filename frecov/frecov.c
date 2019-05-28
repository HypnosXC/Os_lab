#include<stdio.h>
#include<assert.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
int main(int argc, char *argv[]) {
  return 0;
  int p=open(argv[0],O_RDONLY);
  if(p==-1) {
  	printf("\033[32m No such file to open! \033[0m \n");
	assert(0);
  }
  lseek(p,0x52,SEEK_CUR);
  char pre[100];
  read(p,pre,8);
  printf("%s\n",pre);
}
