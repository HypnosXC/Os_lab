/*#include <kernel.h>
#include <klib.h>
int main() {
  _ioe_init();
  _cte_init(os->trap);
  // call sequential init code
  os->init();
  printf("3\n");
  _mpe_init(os->run); // all cores call os->run()
  return 1;
}*/
#include <common.h>
#include <kernel.h>
#include <klib.h>

typedef struct{
  int n;
  sem_t sem;
}fib_t;

sem_t sem_halt;

const char f_name[][100] = {
  "f0",
  "f1",
  "f2",
  "f3",
  "f4",
  "f5",
  "f6",
  "f7",
  "f8",
  "f9",
  "f10",
  "f11",
  "f12",
  "f13",
  "f14",
  "f15",
  "f16",
  "f17",
  "f18",
  "f19",
  "f20",
  "f21",
  "f22",
  "f23",
  "f24",
};

void fib(void *arg){
  fib_t *f = arg;
  if(f->n >= 2){
    fib_t *f1 = pmm->alloc(sizeof(fib_t));
    fib_t *f2 = pmm->alloc(sizeof(fib_t));

    f1->n = f->n - 1;
    f2->n = f->n - 2;
    kmt->sem_init(&f1->sem, f_name[f->n - 1], 0);
    kmt->sem_init(&f2->sem, f_name[f->n - 2], 0);
    kmt->create(pmm->alloc(sizeof(task_t)), f_name[f->n - 1], fib, f1);
    kmt->create(pmm->alloc(sizeof(task_t)), f_name[f->n - 2], fib, f2);

    kmt->sem_wait(&f1->sem);
    kmt->sem_wait(&f2->sem);

    f->n = f1->n + f2->n;

    pmm->free(f1);
    pmm->free(f2);
  }
  kmt->sem_signal(&f->sem);
  kmt->sem_wait(&sem_halt);
}

void fib_c(void *arg){
  long n = (long)arg;
  fib_t *f = pmm->alloc(sizeof(fib_t));
  f->n = n;
  kmt->sem_init(&f->sem, "f", 0);
  kmt->create(pmm->alloc(sizeof(task_t)), "", fib, f);
  kmt->sem_wait(&f->sem);
  printf("------------------%d--------------------\n", f->n);
  pmm->free(f);
  kmt->sem_wait(&sem_halt);
}

int main() {
  _ioe_init();
  _cte_init(os->trap);

  // call sequential init code
  os->init();
  kmt->create(pmm->alloc(sizeof(task_t)), "ff", fib_c, (void *)3l);
  printf("thead??");
  kmt->sem_init(&sem_halt, "sem_halt", 0);
  _mpe_init(os->run); // all cores call os->run()
  return 1;
}

