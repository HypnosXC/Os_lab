#include<stdio.h>
#include<string.h>
#include<stdlib.h>
//#include<dir.h>
#include<dlfcn.h>
#include<sys/types.h>
char dat_inline[1000];
void sgetline()	{
	char x=getchar();
	char *p=dat_inline;
	while(x!='\n')	{
		*p=x;
		p++;
		x=getchar();
	}
	*p='\0';
}
char dlstore[]="dl-XXXXXX";
char gccode[1000];
void dyn_reload(char *func){
  FILE* fd=fopen("dl-XXXXXX.c","a+");	
  fprintf(fd,"%s\n",dat_inline);
  fclose(fd);
  sprintf(gccode,"gcc %s.c -shared -fPIC -o %s.so",dlstore,dlstore);
  system(gccode);
  memset(gccode,0,strlen(gccode));
  sprintf(gccode,"%s.so",dlstore);
  dlopen(gccode,RTLD_LAZY|RTLD_GLOBAL);
}
int main(int argc, char *argv[]) {
 // dlstore=mktemp(dlstore);
  while(1) {
	sgetline();
	char *func=strstr(dat_inline,"int");
	if(func ==NULL||func!=dat_inline) {//expression
		sprintf(expr,"int expr_%d() { return (%s); }",i,dat_inline);		
		dyn_reload(expr);
		memset(expr,0,strlen(expr));
	}
	else 
		dyn_reload(dat_inline);//function
   	  	
  }
//  system("gcc a.c -shared -fPIC -o test.so");
}
