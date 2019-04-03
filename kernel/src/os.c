#include <common.h>
#include <my_os.h>
int printf(const char * tmf,...);
static void os_init() {
  pmm->init();
  printf_lk->name="printf";
}

static void hello() {
  for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
   /* _putc(*ptr);
    _putc('\n');*/
    char *mm=pmm->alloc(100);
    lock(printf_lk);
    printf("mm is %p,%p\n,with cpu %d \n",printf_lk,mm,_cpu());
	unlock(printf_lk);
    mm="maybe the alloc func is not right\n";
    pmm->free(printf_lk);
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
