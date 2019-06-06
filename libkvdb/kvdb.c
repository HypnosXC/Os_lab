#include "kvdb.h"
#include<sys/file.h>
#include<stdio.h>
#include<fcntl.h>
#include<assert.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
void journaling(kvdb_t* db) {
	int offset=8;
	int max_off=lseek(db->fd,0,SEEK_END),doff=0;
	while(offset<=max_off) {
		lseek(db->fd,offset,SEEK_SET);
		jmod* curs=malloc(sizeof(jmod));
		jmod *cure=malloc(sizeof(jmod));
		read(db->fd,curs,sizeof(jmod));
		offset+=sizeof(jmod);
		lseek(db->fd,offset,SEEK_SET);
		offset+=sizeof(jmod);
		read(db->fd,cure,sizeof(jmod));
		if(curs->state==1&&cure->state==2) {//recovery able
			char *buf=malloc(sizeof(char)*curs->size);
			lseek(db->fd,offset,SEEK_SET);
			offset+=curs->size;
			read(db->fd,buf,curs->size);
			write(db->fd,buf,curs->size);
			offset+=curs->size;
			sync();
			curs->state=3;
			lseek(db->fd,offset-2*(curs->size+sizeof(jmod)),SEEK_SET);
			write(db->fd,curs,sizeof(jmod));
			free(buf);
			doff=offset;
		} 
		free(curs);
		free(cure);
	}
	lseek(db->fd,0,SEEK_SET);
	write(db->fd,&doff,sizeof(int));
	sync();
}
int kvdb_open(kvdb_t *db,const char *filename) {
	db->closed=0;
	db->fd=open(filename,O_RDWR|O_CREAT,777);
	if(db->fd==-1)
		return -1;
	flock(db->fd,LOCK_EX);
//	db->mutex=PTHREAD_MUTEX_INITIALIZER;
	db->mutex=malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(db->mutex,NULL);
	pthread_mutex_lock(db->mutex);
	if(lseek(db->fd,0,SEEK_END)!=0) {
		journaling(db);
	}
	else {
		long long f=0;
		write(db->fd,&f,sizeof(long long));
	}
	pthread_mutex_unlock(db->mutex);
	flock(db->fd,LOCK_UN);
	return 0;
}
int kvdb_close(kvdb_t *db) {
	pthread_mutex_lock(db->mutex);
	db->closed=1;
	int f=close(db->fd);
	pthread_mutex_unlock(db->mutex);
	pthread_mutex_destroy(db->mutex);
	return f;
}
int kvdb_put(kvdb_t *db,const char * key,const char *value) {
	if(db->closed==1) {
		printf("error: dataset closed!\n");
		return -1;
	}
	pthread_mutex_lock(db->mutex);
	flock(db->fd,LOCK_EX);
	int off=lseek(db->fd,0,SEEK_END);
	jmod s;
	s.state=1;
	s.size=strlen(value);
	strcpy(s.name,key);
	write(db->fd,&s,sizeof(jmod));
	write(db->fd,&s,sizeof(jmod));
	write(db->fd,value,strlen(value));
	sync();
	//create head jour and record data
	s.state=2;
	lseek(db->fd,off+sizeof(jmod),SEEK_SET);
	write(db->fd,&s,sizeof(jmod));
	sync();
	// create end jour
	lseek(db->fd,0,SEEK_END);
	write(db->fd,value,sizeof(value));
	// write data
	s.state=3;
	lseek(db->fd,off,SEEK_SET);
	write(db->fd,&s,sizeof(jmod));
	off=lseek(db->fd,0,SEEK_END);
	lseek(db->fd,0,SEEK_SET);
	write(db->fd,&off,sizeof(int));
	// change  the maxoff
	flock(db->fd,LOCK_UN);
	pthread_mutex_unlock(db->mutex);
	return 0;
}
char* kvdb_get(kvdb_t *db,const char *key) {
	pthread_mutex_lock(db->mutex);
	int offset=8;
	int max_off=0,doff=0;
	char* value=0;
	read(db->fd,&max_off,sizeof(int));
	jmod *s=malloc(sizeof(jmod));
	while(offset<=max_off) {
		lseek(db->fd,offset,SEEK_SET);
		read(db->fd,s,sizeof(jmod));
		if(!strcmp(s->name,key))
		   	doff=offset;
		offset+=2*sizeof(jmod)+s->size*2;	
	} 
	if(doff==0) {
		free(s);
		printf("No such a key !\n");
		pthread_mutex_unlock(db->mutex);
		return NULL;
	} 
	else {
		lseek(db->fd,doff,SEEK_SET);
		read(db->fd,s,sizeof(jmod));
		doff+=s->size+sizeof(jmod)*2;
		lseek(db->fd,doff,SEEK_SET);
		value=malloc(s->size+1);
		read(db->fd,value,s->size);
		free(s);
		pthread_mutex_unlock(db->mutex);
		return value;
 	}
}



