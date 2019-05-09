#include <common.h>
#include <klib.h>
int rand();
int printf(const char * tmf,...);
int sprintf(char * g,const char *tmf,...);
spin_lock_t irq_lk,trap_lk;
typedef struct _rem_handler{
	handler func;
	int seq;
	int event;
}rem_handler;
rem_handler handlers[1000];
int hlen;
static void os_init() {
  pmm->init();
  kmt->init();
  _vme_init(pmm->alloc,pmm->free);
  dev->init();
  vfs->init();
  kmt->spin_init(&trap_lk,"trap");
  kmt->spin_init(&irq_lk,"irq");
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

static _Context *os_trap(_Event ev, _Context *context) {
  kmt->spin_lock(&trap_lk);
  task_t *cur=current_task();
  cur->context=context;
  _Context *ret=NULL;
  for(int i=0;i<hlen;i++) {
 	if(handlers[i].event==_EVENT_NULL||handler->event==ev.event) {
		_Context *next =handlers[i].func(ev,context);
		if(next!=NULL)	ret=next;
	}
  }
  if(ret==NULL) {
  	printf("fk trap no recurse!\n");
	assert(0);
  }
  kmt->spin_unlock(&trap_lk);
  return ret;
}

static void os_on_irq(int seq, int event, handler_t handler) {
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
	kmt->spin_unlock(&irq_lk);
}

MODULE_DEF(os) {
  .init   = os_init,
  .run    = os_run,
  .trap   = os_trap,
  .on_irq = os_on_irq,
};
