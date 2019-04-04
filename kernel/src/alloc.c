#include <common.h>
#include <klib.h>
#include <my_os.h>
#include <alloc.h>
static uintptr_t pm_start, pm_end,pm_size;
lc* alloc_lk;
static void pmm_init() {
  pm_start = (uintptr_t)_heap.start;
  pm_end   = (uintptr_t)_heap.end;
  pm_size=pm_end-pm_start;
  bt_init((intptr_t *)pm_start,pm_size/(BLOCK_SIZE+32));//block used
  pm_start+=pm_size/(BLOCK_SIZE+32)*32;
  alloc_lk=allc_lc;
  alloc_lk->name="alloc";
}

static void* kalloc(size_t size) {
 lock(alloc_lk);
 size=size+(BLOCK_SIZE-size%BLOCK_SIZE);
 size/=BLOCK_SIZE;
 int pos=bt_alloc(size);
// lock(printf_lk);
// printf("alloc %d block at%x,with cpu %d\n",size,pm_end-pos*BLOCK_SIZE,_cpu());
// unlock(printf_lk);
 unlock(alloc_lk);
 return (void *)(pm_end-pos*BLOCK_SIZE);
}

static void kfree(void *ptr) {
	lock(alloc_lk);
	int pos=pm_end-(intptr_t)ptr;
	pos/=BLOCK_SIZE;
//	lock(printf_lk);
///	printf("free %p at %d with cpu %d\n",ptr,pos,_cpu());
//	unlock(printf_lk);
	bt_free(pos);
	unlock(alloc_lk);
}

MODULE_DEF(pmm) {
  .init = pmm_init,
  .alloc = kalloc,
  .free = kfree,
};
