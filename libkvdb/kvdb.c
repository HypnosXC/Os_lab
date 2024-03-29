#include "kvdb.h"
#include<time.h>
#include<sys/file.h>
#include<stdio.h>
#include<fcntl.h>
#include<assert.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#define cursk(x) ((int)lseek(x->fd,0,SEEK_CUR))
#define zerosk(x) (lseek(x->fd,0,SEEK_SET))
#define endsk(x)  lseek(x->fd,0,SEEK_END)
#define cgsk(x,offset) lseek(x->fd,offset,SEEK_SET)
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
	if(doff!=0)
		write(db->fd,&doff,sizeof(int));
	sync();
}
void may_crash(char *s) {
/*	int n=rand();	
	int f=1;
	for(int i=1;i<=n%1000;i++)
		f=rand()%30;
	if(f==0) {
		printf("\033[031mcrash at %s,offset=%d\n\033[0m",s,f);
		fflush(stdout);
		exit(0);
	}
	used for debugging
	*/
	return ;
}
int kvdb_open(kvdb_t *db,const char *filename) {
	srand(time(0));
	db->closed=0;
	db->fd=open(filename,O_RDWR|O_CREAT,0777);
	if(db->fd==-1) {
		printf("Not opened!\n");
		return -1;
	}
	flock(db->fd,LOCK_EX);
//	db->mutex=PTHREAD_MUTEX_INITIALIZER;
	db->mutex=malloc(sizeof(pthread_mutex_t));
	pthread_mutex_init(db->mutex,NULL);
	pthread_mutex_lock(db->mutex);
	printf("off %d\n",(int)lseek(db->fd,0,SEEK_END));
	fflush(stdout);
	if(lseek(db->fd,0,SEEK_END)!=0) {
		journaling(db);
	}
	else {
		int f=8;
		write(db->fd,&f,sizeof(int));
		int ff=write(db->fd,&f,sizeof(int));
		printf("scu=%d,\n",ff);
	//	printf("reach init ,id=%d,off=%d\n",db->fd,(int)lseek(db->fd,0,SEEK_END));
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
	int off;
	lseek(db->fd,0,SEEK_SET);
	read(db->fd,&off,sizeof(int));
//	printf("now off is%d,jmod size is %d",off,(int)sizeof(jmod));
	lseek(db->fd,off,SEEK_SET);
	jmod s;
	s.state=1;
	s.size=strlen(value);
	strcpy(s.name,key);
	write(db->fd,&s,sizeof(jmod));
	write(db->fd,&s,sizeof(jmod));
	write(db->fd,value,strlen(value));
	sync();
	may_crash("journaling finished");
	//create head jour and record data
	s.state=2;
	lseek(db->fd,off+sizeof(jmod),SEEK_SET);
	write(db->fd,&s,sizeof(jmod));
	sync();
	// create end jour
	may_crash("end journaling");
	lseek(db->fd,off+sizeof(jmod)*2+s.size,SEEK_SET);
	write(db->fd,value,strlen(value));
//	printf("data end off is%d\n",cursk(db));
	// write data
	may_crash("data write");
	s.state=3;
	lseek(db->fd,off,SEEK_SET);
	write(db->fd,&s,sizeof(jmod));
	off+=2*(s.size+sizeof(jmod));
//	printf("finished with offset=%d\n",off);
	lseek(db->fd,0,SEEK_SET);
	write(db->fd,&off,sizeof(int));
	// change  the maxoff
	flock(db->fd,LOCK_UN);
//	printf("[%s]=[%s],offset=%d\n",key,value,off);
	pthread_mutex_unlock(db->mutex);
	may_crash("whole finished!");
	return 0;
}
char* kvdb_get(kvdb_t *db,const char *key) {
	pthread_mutex_lock(db->mutex);
	int offset=8;
	int max_off=0,doff=0;
	char* value=0;
	lseek(db->fd,0,SEEK_SET);
	read(db->fd,&max_off,sizeof(int));
	jmod *s=malloc(sizeof(jmod));
	while(offset<max_off) {
		lseek(db->fd,offset,SEEK_SET);
		read(db->fd,s,sizeof(jmod));
//		printf("\033[32m offset=%d,name=%s,%d\033[0m\n",offset,s->name,strcmp(s->name,key));
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
		printf("\033[34mfind key :%s,size=%d \n\033[0m",s->name,s->size);
		doff+=s->size+sizeof(jmod)*2;
		lseek(db->fd,doff,SEEK_SET);
		value=malloc(s->size+1);
		read(db->fd,value,s->size);
		free(s);
		pthread_mutex_unlock(db->mutex);
		return value;
 	}
}



