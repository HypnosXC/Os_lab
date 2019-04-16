#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<assert.h>
int filedes[2];
char name[100];
char cost[100];
char localpath[100];
char data_inline[100009];
int get_line(char s[]) {
	char x=getchar();
	int i=0;
	while(x!='\n'&&x!=EOF)	{
		s[i++]=x;
		x=getchar();
	}
	s[i]=0;
	if(i==0||x==EOF)
		return 0;
	return 1;
}
int main(int argc, char *argv[]) {
  if(pipe(filedes)!=0) {
  	printf("wrong pipe operation\n");
	assert(0);
  }
  int pid=fork();
  if(pid==0)	{
  	char *subargv[100];
	subargv[0]=strdup("strace");
	subargv[1]=strdup("-T");
	for(int i=1;i<argc;i++){
		subargv[i+1]=strdup(argv[i]);
	}
	subargv[argc+1]=(char*)0;
	FILE* fnull=fopen("/dev/null","w");
	int fnul=fileno(fnull);
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
	  if(dup2(filedes[0],STDIN_FILENO)<0)
		  assert(0);
	  while(get_line(data_inline)) {
	     int tl=strchr(data_inline,'(')-data_inline;
		 int el=strrchr(data_inline,'<')-data_inline;
		// strncpy(name,data_inline,tl);
		// strncpy(cost,data_inline+el,strlen(data_inline)-el);
		 printf("%s\n,%s : %s\n",data_inline,name,cost);
	  }
	  printf("over!");
	  close(filedes[0]);
	  close(filedes[1]);
	  return 0;
  }
}
