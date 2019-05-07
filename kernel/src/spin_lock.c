#include "common.h"
#include "kernel.h"
#include <assert.h>
#include <stdio.h>
static int cpu_cnt[100];
void cli();
void sti();
void spin_init(struct spinlock *lk,char *name) {
	lk->locked=0;
	lk->name=name;
}
void spin_lock(struct spinlock *lk) {
	cli();//disable interrupts
	while(_atomic_xchg(&lk->locked,1)!=0);
	lk->hcpu=_cpu();
	cpu_cnt[lk->hcpu]++;
}
void spin_unlock(struct spinlock *lk) {
	if(_atomic_xchg(&lk->locked,0)!=1) {
		printf("%s: unlock but no hold!\n",lk->name);
		assert(0);
	}
	if(_cpu()!=lk->hcpu){
		printf("%s: wrong cpu unlock!\n".lk->name);// different cpu ,one hold, but another unlock
		assert(0);
	}
	cpu_cnt[lk->hcpu]--;
	if(cpu_cnt[lk->hcpu]==0)
		sti();//enable intertupt when no lock
}
