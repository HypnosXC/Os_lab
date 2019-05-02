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
char dlstore[100];
char gccode[1000];
char expr[1000];
void *dlp;
void dyn_reload(char *func){
  sprintf(gccode,"%s.c",dlstore);
  FILE* fd=fopen(gccode,"w");	
  fprintf(fd,"%s\n",func);
  fclose(fd);
  sprintf(gccode,"gcc %s.c -shared -ldl -fPIC -o %s.so 2>/dev/null",dlstore,dlstore);
  system(gccode);
  memset(gccode,0,strlen(gccode));
  sprintf(gccode,"./%s.so",dlstore);
 // if(dlp!=NULL) 
//	  dlclose(dlp);
  dlp=dlopen(gccode,RTLD_LAZY|RTLD_GLOBAL); 
  sprintf(gccode,"%s.c",dlstore);
  remove(gccode);
  sprintf(gccode,"%s.so",dlstore);
  remove(gccode);
  char *wr=dlerror();
  if(wr!=NULL) {
	  printf("openfault:\n %s\n",wr);
  }
}
void* func_find(char *func) {
	void* p=dlsym(dlp,func);
	char *wr=dlerror();
	if(wr!=NULL) {
		printf("%s havs problem:\n %s\n",func,wr);
		fflush(stdout);
	}
	return p;
}
int main(int argc, char *argv[]) {
 // dlstore=mktemp(dlstore);
  int i=0;
  while(1) {
	i++;
	sgetline();
	sprintf(dlstore,"dl%d",i);
	int func=strstr(dat_inline,"int")-dat_inline;
	if(func!=0) {//expression
		sprintf(expr,"int expr_%d() { return (%s); }",i,dat_inline);		
		dyn_reload(expr);
		memset(expr,0,strlen(expr));
		sprintf(expr,"expr_%d",i);
		int (*func)()= func_find(expr);
		int value=func();
		printf("[crepl] = %d\n",value);
	}
	else { 
		dyn_reload(dat_inline);//function
		printf("Added:%s\n",dat_inline);
	}
   	  	
  }
//  system("gcc a.c -shared -fPIC -o test.so");
}
