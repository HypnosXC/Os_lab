#include<common.h>
#include<klib.h>
#define STACK_SIZE 4096
task_t *current[32];
spinlock_t tsk_lk;
static int cpu_cnt[100];
void cli(){
	asm volatile ("cli"); 
}
void sti() {
	asm volatile ("sti");
}
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
		printf("%s: wrong cpu unlock!\n",lk->name);// different cpu ,one hold, but another unlock
		assert(0);
	}
	cpu_cnt[lk->hcpu]--;
	if(cpu_cnt[lk->hcpu]==0)
		sti();//enable intertupt when no lock
}
void noreach() {
	printf("never been here!\n");
	while(1){
		_yield();
	}
}
task_t* current_task() {
	return current[_cpu()];
}
_Context* context_switch(_Event e,_Context* c) {
	spin_lock(&tsk_lk);
	for(int i=_ncpu();i<32;i++)	{
		if(current[i]!=NULL&&current[i]->state==0)	{
		   task_t* t=current[_cpu()];
		   current[_cpu()]=current[i];
	       current[i]=t;
		   break;	   
		}
	}
	spin_unlock(&tsk_lk);
	return current[_cpu()]->context;
}
_Context* context_save(_Event e,_Context *c) {
	spin_lock(&tsk_lk);
    current[_cpu()]->context=c;
	spin_unlock(&tsk_lk);
	return NULL;
}
int create(task_t *task,const char *name,void (*entry)(void *arg),void *arg) {
	spin_lock(&tsk_lk);
	strcpy(task->name,name);
	task->state=0;
	task->stack.start=pmm->alloc(STACK_SIZE);
	task->stack.end=(void *)((intptr_t)task->stack.start+STACK_SIZE);
	task->context=_kcontext(task->stack,entry,arg);
	printf("new task:%s\n",name);
	for(int i=0;i<32;i++)
		if(current[i]==NULL)
			current[i]=task;
	spin_unlock(&tsk_lk);
	return 1;
}
void teardown(task_t *task) {
	spin_lock(&tsk_lk);
	for(int i=0;i<32;i++)
		if(current[i]==task)
				current[i]=NULL;
	pmm->free(task->stack.start);
	pmm->free(task);
	spin_unlock(&tsk_lk);
}
void kmt_init() {
	os->on_irq(-19999,_EVENT_NULL,context_save);
	os->on_irq(19999,_EVENT_NULL,context_switch);
	for(int i=0;i<8;i++)	{
		create(pmm->alloc(sizeof(task_t)),"empty",noreach,NULL);
	}
	kmt->spin_init(&tsk_lk,"task");
}
