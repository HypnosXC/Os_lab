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
  int fnul=-1;
  int fake_loc=0;
  if(pid==0)	{
  	char *subargv[100];
	subargv[0]=strdup("strace");
	subargv[1]=strdup("-T");
	for(int i=1;i<argc;i++){
		subargv[i+1]=strdup(argv[i]);
	}
	subargv[argc+1]=(char*)0;
	FILE* fnull=fopen("/dev/null","w");
	fnul=fileno(fnull);
	if(dup2(filedes[1],STDERR_FILENO)<0||dup2(fnul,STDOUT_FILENO)<0) {
		printf("wrong file redirection!\n");
		assert(0);
	}
	for(int i=0;i<argc+2;i++)
		printf("%s ",subargv[i]);
	char * envp[]={0,NULL};
	execve("/usr/bin/strace",subargv,envp);
	assert(0);
  }
  else {
	  if(fnul<0){
	  	printf("panic /dev/null!\n");
	  	assert(0);
	  }
	  close(filedes[0]);
	  close(filedes[1]);
  }
}
