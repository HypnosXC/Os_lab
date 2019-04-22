#include<stdio.h>
#include<time.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<assert.h>
int filedes[2];
char name[100];
char cost[100];
char localpath[100];
char data_inline[100009];
struct item{
	char name[100];
	double ct;
}pthd[1000];
int tot;
void new_pd() {
	strcpy(pthd[tot].name,name);
	pthd[tot].ct=atof(cost);
	tot++;
}
int get_line(char s[]) {
	char x=getchar();
	int i=0;
	while(x!=0&&x!='\n'&&x!=EOF)	{
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
	  double total_time=0,ts=clock(); 
	  int i=100000;
	  while(get_line(data_inline)&&i) {
		 i--;
	     int tl=strchr(data_inline,'(')-data_inline;
		 int el=strrchr(data_inline,'<')-data_inline;
		 //printf("%d %d\n",tl,el);
		 strncpy(name,data_inline,tl);
		 if(!strcmp("exit_group",name))
			 break;
		 strncpy(cost,data_inline+el+1,strlen(data_inline)-el-2);
		 total_time+=atof(cost);
		 int matc=0;
		 for(int i=0;i<tot;i++)	{
		 	if(!strcmp(pthd[i].name,name)) {
				matc=1;
				pthd[i].ct+=atof(cost);
			}
		 }
		 if(!matc)
			 new_pd();
		 memset(name,0,sizeof(name));
		 memset(cost,0,sizeof(cost));
		 if((clock()-ts)/CLOCKS_PER_SEC>0.1) {
		 	for(int i=0;i<tot;i++)	{
				printf("%s : %lf\n",pthd[i].name,pthd[i].ct);
			}
		 }
	  }
	  printf("\033[2J");
	  for(int i=0;i<tot;i++)	{
		printf("\033[%dm%s : %.2lf%%\n\033[0m",i%10+40,pthd[i].name,pthd[i].ct/total_time*100);
	  }
	  close(filedes[0]);
	  close(filedes[1]);
	  return 0;
  }
}
