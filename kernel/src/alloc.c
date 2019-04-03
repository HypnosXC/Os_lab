#include <common.h>
#include <klib.h>
#include <my_os.h>
#define BLOCK_SIZE 128// basic block for every malloc
static uintptr_t pm_start, pm_end,pm_size;
lc* alloc_lk;
static void pmm_init() {
  pm_start = (uintptr_t)_heap.start;
  pm_end   = (uintptr_t)_heap.end;
  pm_size=pm_end-pm_start;
  bt_init((intptr_t *)pm_start,pm_size/(BLOCK_SIZE+1));//block used
  pm_start+=pm_size/(BLOCK_SIZE+1);
  printf("%d and %d\n",pm_start+pm_size/(BLOCK_SIZE+1)*BLOCK_SIZE,pm_end);
  alloc_lk=allc_lc;
  alloc_lk->name="alloc";
}

static void *kalloc(size_t size) {
  return NULL;
}

static void kfree(void *ptr) {
}

MODULE_DEF(pmm) {
  .init = pmm_init,
  .alloc = kalloc,
  .free = kfree,
};
