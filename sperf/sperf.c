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
	subargv[0]="-T ";
	for(int i=1;i<argc;i++){
		subargv[i]=argv[i];
		printf("%s ",subargv[i]);
	}
	printf("\n");
	subargv[argc+2]=(char *)0;
	char * envp[]={0,NULL};
	execve("/bin/ls",subargv,envp);
	assert(0);
  }
  else
	  return 0;
}
