#ifndef __KVDB_H__
#define __KVDB_H__
typedef struct _jour_mod{
	int state;//1 for head, 2 for end ,3 for used or not useful;
	int size;//file size 
	char name[130];
}jmod;
struct kvdb {
	int fd;
	int closed;
};
typedef struct kvdb kvdb_t;
void journaling(kvdb_t *db);
int kvdb_open(kvdb_t *db, const char *filename);
int kvdb_close(kvdb_t *db);
int kvdb_put(kvdb_t *db, const char *key, const char *value);
char *kvdb_get(kvdb_t *db, const char *key);

#endif
