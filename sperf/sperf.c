#include<stdio.h>
#include<unistd.h>
#include<assert.h>
int filedes[2];
int main(int argc, char *argv[]) {
  if(pipe(filedes)!=0) {
  	printf("wrong pipe operation\n");
	assert(0);
  }
  int pid=fork();
  if(pid==0)	{
  	char *subargv[100];
	subargv[0]="-T -o rt";
	for(int i=0;i<argc;i++){
		subargv[i+1]=argv[i];
		printf("%s",subargv[i+1]);
	}
	printf("\n");
	subargv[argc+2]=(char *)0;
//	char * envp[]={0,NULL};
//	execve("/usr/bin/strace",subargv,envp);
//	assert(0);
  }
  else
	  return 0;
}
