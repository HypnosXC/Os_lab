#include <common.h>
#include <klib.h>
#include <alloc.h>
#define BLOCK_SIZE 128// basic block for every malloc
static uintptr_t pm_start, pm_end,pm_size;
lc* alloc_lk;
static void pmm_init() {
  pm_start = (uintptr_t)_heap.start;
  pm_end   = (uintptr_t)_heap.end;
  pm_size=pm_end-pm_start;
  bt_init((intptr_t *)pm_start,pm_size/(BLOCK_SIZE+2));//block used
  pm_start+=pm_size/(BLOCK_SIZE+2)*2;
  printf("%d and %d\n",pm_start+pm_size/(BLOCK_SIZE+2)*BLOCK_SIZE,pm_end);
  alloc_lk=allc_lc;
  alloc_lk->name="alloc";
}

static void* kalloc(size_t size) {
 printf("alloc %d block\n",size);
 lock(alloc_lk);
 size=size+(BLOCK_SIZE-size%BLOCK_SIZE);
 size/=BLOCK_SIZE;
 int pos=bt_alloc(size);
 return (void *)(pm_end-pos*BLOCK_SIZE);
}

static void kfree(void *ptr) {
	int pos=pm_end-(intptr_t)ptr;
	pos/=BLOCK_SIZE;
	printf("free %p at %d",ptr,pos);
	bt_free(pos);
	unlock(alloc_lk);
}

MODULE_DEF(pmm) {
  .init = pmm_init,
  .alloc = kalloc,
  .free = kfree,
};
