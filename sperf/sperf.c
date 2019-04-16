#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<assert.h>
int filedes[2];
char path[100];
char localpath[100];
int main(int argc, char *argv[]) {
 // if(pipe(filedes)!=0) {
  //	printf("wrong pipe operation\n");
	//assert(0);
 // }
  int l=readlink("/proc/self/exe",localpath,99);
//  while(localpath[l]!='/')
//	  	l--;
  localpath[l]=0;
  assert(l>0);
//  printf("local=%s\n",localpath);
  int pid=fork();
  if(pid==0)	{
  	char *subargv[100];
	subargv[0]=strdup("strace");
//	subargv[1]=strdup("-T");
	subargv[1]=strdup(localpath);
	for(int i=1;i<argc;i++){
		subargv[i+1]=strdup(argv[i]);
	}
//	subargv[argc+2]=">/dev/null";
	subargv[argc+3]=(char*)0;
	for(int i=0;i<argc+3;i++)
		printf("%s ",subargv[i]);
	printf("\n");
	char * envp[]={0,NULL};
	execve("/usr/bin/strace",subargv,envp);
	assert(0);
  }
  else
	  return 0;
}
