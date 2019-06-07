#include<stdio.h>
#include<stdlib.h>
#include "../libkvdb/kvdb.h"
int main() {
	kvdb_t db;
	const char *key="opearating-ss";
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
}
