#include "common.h"
#include "kernel.h"

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
		assert(0);
		panic("unlock but no hold!");
	}
	if(_cpu()!=lk->hcpu)
		panic("wrong cpu unlock!");// different cpu ,one hold, but another unlock
	cpu_cnt[lk->hcpu]--;
	if(cpu_cnt[lk->hcpu]==0)
		sti();//enable intertupt when no lock
}
