/*#include<stdio.h>
#include<stdlib.h>
#include "../libkvdb/kvdb.h"
int main() {
	kvdb_t db;
//	const char *key="opearating-ss";
	char *value;
	kvdb_open(&db,"a.db");
//	kvdb_put(&db,key,"hahahah");
//	value=kvdb_get(&db,key);
	while(1) {
		char name[100],valu[100];
		printf("echo name=");
		scanf("%s",name);
		printf("\necho value=");
		scanf("%s",valu);
		kvdb_put(&db,name,valu);
		value=kvdb_get(&db,name);
		printf("\nnow [%s] is [%s]\n",name,value);
		free(value);
	
	}
	kvdb_close(&db);
	return 0;
}*/
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
//#include <pthread.h>
#include "kvdb.h"

extern bool isBroke;
char alphaOff = 0;
void work_place_r(kvdb_t *db){
	char a[128], b[128];
	int num = 0;;
	memset(a, 0, sizeof(a));
	memset(b, 0, sizeof(b));
	for(int i=0;i < 20;i++){
		a[i] = 'a'+alphaOff;
		b[i] = 'b'+alphaOff;
		printf("$$$ seek %s %s\n", a, b);
		//alphaOff += 1;
		char* str = NULL;
		while(str == NULL)
			str= kvdb_get(db, a);

		printf("\33[1;35 get num:%d %s\33[0m\n", num, str);
	}
	printf("end\n");
}

void work_place_w(kvdb_t *db){
	char a[128], b[128];
	int num = 0;;
	memset(a, 0, sizeof(a));
	memset(b, 0, sizeof(b));
	for(int i=0;i < 20;i++){
		a[i] = 'a'+alphaOff;
		b[i] = 'b'+alphaOff;
		printf("$$$ create %s %s\n", a, b);
		//alphaOff += 1;
		printf("create num:%d %d\n", num, kvdb_put(db, a, b));
	}
	printf("end\n");
}

int main(){
	kvdb_t db;
	int pid = fork();
	pid++;
	pid += fork();
	printf("%d\n", kvdb_open(&db, "hehehe"));

	pthread_t id1, id2, id3, id4;
	pthread_create(&id1, NULL, (void*)work_place_r, (void*)&db);
	pthread_create(&id2, NULL, (void*)work_place_r, (void*)&db);
	pthread_create(&id3, NULL, (void*)work_place_w, (void*)&db);
	pthread_create(&id4, NULL, (void*)work_place_w, (void*)&db);
	/*
	if(ret1!=0 || ret2!=0 || ret3!=0 || ret4!=0){
		printf("\33[1;31mthread error\33[0m\n");
		return 0;
	}
	*/

	pthread_join(id1, NULL);
	pthread_join(id2, NULL);
	pthread_join(id3, NULL);
	pthread_join(id4, NULL);

	kvdb_close(&db);

	return 0;
}
