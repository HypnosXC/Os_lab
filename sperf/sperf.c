#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<assert.h>
int filedes[2];
char path[100];
char localpath[100];
int main(int argc, char *argv[]) {
  if(pipe(filedes)!=0) {
  	printf("wrong pipe operation\n");
	assert(0);
  }
 // int pid=fork();
  int l=readlink("proc/self/exe",localpath,100);
  assert(l<0);
  printf("local=%s\n",localpath);

/*
  if(pid==0)	{
  	char *subargv[100];
	subargv[0]="ls";
	subargv[1]="-a";
	for(int i=1;i<argc;i++){
		subargv[i+1]=argv[i];
		printf("%s ",subargv[i]);
	}
	printf("\n");
	subargv[argc+1]=
	subargv[argc+2]=(char *)0;
	char * envp[]={0,NULL};
	execve("/bin/ls",subargv,envp);
	assert(0);
  }
  else
	  return 0;
	  */
}
