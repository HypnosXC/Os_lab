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
  int pid=fork();
  printf("pid is %d\n",pid);
  if(pid==0)	{
  	char *subargv[100];
	subargv[0]=strdup("strace");
	subargv[1]=strdup("-T");
	for(int i=1;i<argc;i++){
		subargv[i+1]=strdup(argv[i]);
	}
	subargv[argc+1]=(char*)0;
	for(int i=0;i<argc+2;i++)
		printf("%s ",subargv[i]);
	char * envp[]={0,NULL};
	execve("/usr/bin/strace",subargv,envp);
	assert(0);
  }
  else
	  return 0;
}
