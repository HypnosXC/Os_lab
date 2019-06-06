#include<stdio.h>
#include<stdlib.h>
#include "../libkvdb/kvdb.h"
int main() {
	kvdb_t db;
	const char *key="opearating-ss";
	char *value;
	kvdb_open(&db,"a.db");
	kvdb_put(&db,key,"hahahah");
	value=kvdb_get(&db,key);
	kvdb_close(&db);
	printf("[%s] is [%s]\n",key,value);
	free(value);
	return 0;
}
