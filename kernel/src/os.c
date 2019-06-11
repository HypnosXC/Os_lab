#include <common.h>
#include <klib.h>
#include <devices.h>
int rand();
void pushcli();
void popcli();
void echo_task(void *name);
int printf(const char * tmf,...);
int sprintf(char * g,const char *tmf,...);
spinlock_t irq_lk,trap_lk;
typedef struct _rem_handler{
	handler_t func;
	int seq;
	int event;
}rem_handler;
rem_handler handlers[1000];
int hlen;
static void os_init() {
  pmm->init();
  kmt->spin_init(&trap_lk,"trap");
  kmt->spin_init(&irq_lk,"irq");
  kmt->init();
  _vme_init(pmm->alloc,pmm->free);
  dev->init();
  vfs->init();
  kmt->create(pmm->alloc(sizeof(task_t)),"print",echo_task,"tty1");
  kmt->create(pmm->alloc(sizeof(task_t)),"print",echo_task,"tty2");
  kmt->create(pmm->alloc(sizeof(task_t)),"print",echo_task,"tty3");
  kmt->create(pmm->alloc(sizeof(task_t)),"print",echo_task,"tty4");
  //vfs->init();
  
}
static void hello() {
  for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
    _putc(*ptr);

  }
  _putc("12345678"[_cpu()]); _putc('\n');
}
static void os_run() {
  hello();
  _intr_write(1);
  while (1) {
    _yield();
  }  
}

device_t *dev_lookup (const char* name);
void echo_task(void *name) {
	device_t *tty= dev_lookup(name);
	while(1) {
		char line[128],text[128];
		sprintf(text,"(%s) $ ", name);
		tty->ops->write(tty,0,text,strlen(text));
		int nread = tty->ops->read(tty,0,line,sizeof(line));
		line[nread-1]='\0';
		for(int i=0;i<sizeof(line);i++)
			if(line[i]==' ')
				line[i]=0;
		char *op=line;
		char *path=line+strlen(op)+1;
		printf("op=%s, path=%s\n",op,path);
		if(!strcmp(op,"ls")) {
			printf("ls operation!\n");
			if(path[0]!=0) 
				ls_operation(path);
		}
		if(!strcmp(op,"cd")) {
			printf("cd operation!\n");
			if(path[0]!=0) 
				cd_operation(path);
		}
		if(!strcmp(op,"mkdir")) {
			printf("mkdir operation!\n");
			if(path[0]!=0) 
				mkdir_operation(path);
		}
		if(!strcmp(op,"rmdir")) {
			printf("ls operation!\n");
			if(path[0]!=0) 
				rmdir_operation(path);
		}	
		sprintf(text,"Echo: %s,\n",line);
		tty->ops->write(tty,0,text,strlen(text));
	}
}
static _Context *os_trap(_Event ev, _Context *context) {
  //kmt->spin_lock(&trap_lk);
  pushcli();
 // printf("\033[32mtrap in cpu=%d?\n\033[0m",_cpu());
  //printf("\ncpu#%d hold task %s\n",_cpu(),cur->name);
  _Context *ret=NULL;
//  printf("\n goes as");
   for(int i=0;i<hlen;i++) {
 	if(handlers[i].event==_EVENT_NULL||handlers[i].event==ev.event) {
		_Context *next =handlers[i].func(ev,context);
//		printf(" %d",i);
		if(next!=NULL)	ret=next;
 	 }
  }
  //printf("\n");
  task_t *cur=current_task(); 
  if(ret==NULL) {  
  	printf("\033[31m fk trap%d no recurse!,hlen=%d,task=%s,cpu=%d\n\033[0m",ev.event,hlen,cur->name,_cpu());
	assert(0);
  }
 // printf("\nIn trap now for cpu#%d, cnt is %d\n",_cpu(),cnt_cpu()); 
 // kmt->spin_unlock(&trap_lk);
  //printf("\033[32mtrap finished,with task=%s,eip=%d\n\033[0m",cur->name,ret->eip);
  if(ret==NULL) {
  	printf("\n\033[32m panic \003[33m trap return NULL to revise\033[0m\n");
	assert(0);
  } 
  popcli();
  return ret;
}

static void os_on_irq(int seq, int event, handler_t handler) {
//	printf("irq set started");
	kmt->spin_lock(&irq_lk);
	hlen++;
	int i=hlen-1;
	handlers[i].func=handler;
	handlers[i].seq=seq;
	handlers[i].event=event;
	for(i=hlen-2;i>=0;i--)	{
		if(handlers[i].seq>handlers[i+1].seq)	{
			rem_handler t=handlers[i];
			handlers[i]=handlers[i+1];
			handlers[i+1]=t;
		}
	}
	//printf("1,hlen=%d\n,cpu=%d,seq=%d\n",hlen,_cpu(),seq);
	kmt->spin_unlock(&irq_lk);
//	printf("irq set!\n");
	return;
}

MODULE_DEF(os) {
  .init   = os_init,
  .run    = os_run,
  .trap   = os_trap,
  .on_irq = os_on_irq,
};
