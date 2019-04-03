#include <common.h>
#include <my_os.h>
int printf(const char * tmf,...);
static void os_init() {
  pmm->init();
  printf_lk->name="printf";
}

static void hello() {
  for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
    _putc(*ptr);
    char *mm=pmm->alloc(100);
    mm="maybe the alloc func is not right\n";
    lock(printf_lk);
    printf("%s",mm);
    unlock(printf_lk);
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
