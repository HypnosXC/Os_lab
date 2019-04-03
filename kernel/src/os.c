#include <common.h>
#include <my_os.h>
int rand();
int printf(const char * tmf,...);
int sprintf(char * g,const char *tmf,...);
static void os_init() {
  pmm->init();
  printf_lk->name="printf";
}

static void hello() {
  for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
    _putc(*ptr);

  }
  _putc("12345678"[_cpu()]); _putc('\n');
}
static void test() {
    char* q[1009];
	int top=0;
	for(int i=1;i<=1000;i++){
		int len=rand()%100*128;
		int f=rand()%5;
		if(!top||!f)	{
			q[top++]=pmm->alloc(len);
			sprintf(q[top-1],"alloc mem with %d",len);
		}
		else {
			lock(printf_lk);
			printf("%s\n",q[top-1]);
			unlock(printf_lk);
			pmm->free(q[--top]);
		}
	}
}
static void os_run() {
  hello();
  test();
  _intr_write(1);
  while (1) {
    _yield();
  }
}

static _Context *os_trap(_Event ev, _Context *context) {
  return context;
}

static void os_on_irq(int seq, int event, handler_t handler) {
}

MODULE_DEF(os) {
  .init   = os_init,
  .run    = os_run,
  .trap   = os_trap,
  .on_irq = os_on_irq,
};
