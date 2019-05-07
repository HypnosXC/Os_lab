#include "common.h"
static int cpu_cnt[100];
spin_lock alloc_lk={0}
void spin_init(struct spinlock *lk,char *name) {
	lk->loked=0;
	lk->name=name;
}
void spin_lock(spinlock *lk) {
	cli();//disable interrupts
	while(_atomic_xchg(&lk->locked,1)!=0);
	lk->hcpu=_cpu();
	cpu_cnt[lk->hcpu]++;
}
void spin_unlock(spinlock *lk) {
	if(_atomic_xchg(&lk->locked,0)!=1) {
		panic("unlock but no hold!");
	}
	if(_cpu()!=lk->hcpu)
		panic("wrong cpu unlock!");// different cpu ,one hold, but another unlock
	cpu_cnt[lk->hcpu]--;
	if(cpu_cnt[lk->hcpu]==0)
		sti();//enable intertupt when no lock
}
