#include<stdio.h>
#include<string.h>
#include<stdllib.h>
#include<dlfcn.h>
#include<sys/types.h>
char dat_inline[1000];
int main(int argc, char *argv[]) {
  while(1) {
 	gets_s(dat_inline,1000);
	char *func=strstr(dat_inline,"int");
	if(func==NULL||func!=dat_inline) {//expression
		
	}
	else {//function
		FILE* fd=fopen("a.c",O_RDWR|O_APPEND);
		fprintf(fd,dat_inline);

		fclose(fd);
	}
  }
//  system("gcc a.c -shared -fPIC -o test.so");
}
