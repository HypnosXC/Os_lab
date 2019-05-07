#include <common.h>
#include <klib.h>
#include <alloc.h>
static uintptr_t pm_start, pm_end,pm_size,current_ptr=0,cu_pos,off_set=0;
struct spinlock alloc_lc={0,0,"alloc"};
struct spinlock* alloc_lk;
static void pmm_init() {
  pm_start = (uintptr_t)_heap.start;
  pm_end   = (uintptr_t)_heap.end;
  pm_size=pm_end-pm_start;
  bt_init((intptr_t *)pm_start,pm_size/(BLOCK_SIZE+32));//block used
  pm_start+=pm_size/(BLOCK_SIZE+32)*32;
  alloc_lk=&alloc_lc;
}

static void* kalloc(size_t size) {
 if(size>=BLOCK_SIZE) {
 	spin_lock(alloc_lk);
 	size=size+(BLOCK_SIZE-size%BLOCK_SIZE);
	size/=BLOCK_SIZE;
	int pos=bt_alloc(size);
// lock(printf_lk);
// printf("alloc %p block at %d,with cpu %d\n",(void *)(pm_end-pos*BLOCK_SIZE),pos,_cpu());
// unlock(printf_lk);
    spin_unlock(alloc_lk);
	return (void *)(pm_end-pos*BLOCK_SIZE);
 }
 else {
	size=size+base_sz-size%base_sz;
 	if(!current_ptr||off_set+size>=BLOCK_SIZE) {
		spin_lock(alloc_lk);
		cu_pos=bt_alloc(1);
		current_ptr=pm_end-cu_pos*BLOCK_SIZE;	
		off_set=size;
//		 lock(printf_lk);
//		 printf("alloc %p block at%d,with cpu %d\n",pm_end-cu_pos*BLOCK_SIZE,cu_pos,_cpu());
//		 unlock(printf_lk);
 		spin_unlock(alloc_lk);
	}
	else {
//		lock(printf_lk);
//		printf("small mem required at %p\n",current_ptr+off_set);
//		unlock(printf_lk);
		off_set+=size;
		bt_add(cu_pos);
	}
	return (void *)(current_ptr+off_set-size);
 }
}

static void kfree(void *ptr) {
	spin_lock(alloc_lk);
	intptr_t pos=(intptr_t)ptr;
	while(pos%BLOCK_SIZE)	{ 
		pos-=pos%BLOCK_SIZE;
	}
	pos=pm_end-pos;
	pos/=BLOCK_SIZE;
//	lock(printf_lk);
//	printf("free %p at %d with cpu %d\n",ptr,pos,_cpu());
//	unlock(printf_lk);
	bt_free(pos);
	spin_unlock(alloc_lk);
}

MODULE_DEF(pmm) {
  .init = pmm_init,
  .alloc = kalloc,
  .free = kfree,
};
