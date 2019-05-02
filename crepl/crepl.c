#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<dlfcn.h>
#include<sys/types.h>
char dat_inline[1000];
void getline()	{
	char x=getchar();
	char *p=dat_inline;
	while(x!='\n')	{
		*p=x;
		p++;
		x=getchar();
	}
	*p='\0';
}
int main(int argc, char *argv[]) {
  while(1) {
	getline();
	char *func=strstr(dat_inline,"int");
	if(func==NULL||func!=dat_inline) {//expression
		
	}
	else {//function
		FILE* fd=fopen("a.c","ab+");
		fprintf(fd,"%s\n",dat_inline);
		fclose(fd);
	}
  }
//  system("gcc a.c -shared -fPIC -o test.so");
}
