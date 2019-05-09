#include<common.h>
#include<klib.h>
#define STACK_SIZE 4096
task_t *current[32];
spinlock_t tsk_lk,prf_lk;

// spin_lock started
static int cpu_cnt[100];
void cli(){
	asm volatile ("cli"); 
}
void sti() {
	asm volatile ("sti");
}
void spin_init(struct spinlock *lk,const char *name) {
	lk->locked=0;
	strcpy(lk->name,name);
	printf("lk %s=%s\n",name,lk->name);
}
void spin_lock(struct spinlock *lk) {
	cli();//disable interrupts
	while(_atomic_xchg(&lk->locked,1)!=0);
	lk->hcpu=_cpu();
	cpu_cnt[lk->hcpu]++;
}
int cnt_cpu() {
	return cpu_cnt[_cpu()];
}
void spin_unlock(struct spinlock *lk) {
	if(_atomic_xchg(&lk->locked,0)!=1) {
		printf("\033[31m%s: unlock but no hold!\n\033[0m",lk->name);
		assert(0);
	}
	int cpu_n=_cpu();
	if(cpu_n!=lk->hcpu){
		printf("\033[31m%s: wrong cpu unlock at%d,original %d\n\033m",lk->name,cpu_n,lk->hcpu);// different cpu ,one hold, but another unlock
		assert(0);
	}
	cpu_cnt[lk->hcpu]--;
//	printf("lk=%s,cpu%d,lkcnt=%d\n",lk->name,lk->hcpu,cpu_cnt[lk->hcpu]);
	if(cpu_cnt[lk->hcpu]==0)
		sti();//enable intertupt when no lock
}
// spin_lock finished





//sem started
void sem_init(sem_t *sem, const char *name,int value){
	spin_init(&sem->sem_lk,name);
	sem->value=value;
    strcpy(sem->name,name);
	sem->top=0;
}
void sem_wait(sem_t *sem) {
	spin_lock(&sem->sem_lk);
	sem->value--;
	while(sem->value<0) {
	  	task_t* cur=current_task();
		if(cur->state!=1)//no sleeped before or waken but no resourse
			sem->sem_st[sem->top++]=cur;
		cur->state=1;//sleep;
		printf("no hanlded yet for sem yield");
		assert(0);
		kmt->spin_unlock(&sem->sem_lk);
		_yield();
		kmt->spin_lock(&sem->sem_lk);
	}
	spin_unlock(&sem->sem_lk);
}
void sem_signal(sem_t *sem) {
	spin_lock(&sem->sem_lk);
	sem->value++;
	if(sem->value>=0)
		sem->sem_st[sem->top--]->state=0;//runable
	spin_unlock(&sem->sem_lk);
}
// sem over
//
//
//
//
//
//task started
void noreach() {
//	printf("never been here!\n");
 	while(1){
		spin_lock(&prf_lk);
//		printf("cpu=%d\n",_cpu());
		spin_unlock(&prf_lk);
		_yield();
	}
}
task_t* current_task() {
	return current[_cpu()];
}
_Context* context_switch(_Event e,_Context* c) {
	spin_lock(&tsk_lk);
	for(int i=_ncpu();i<32;i++)	{
		if(i%_ncpu()!=_cpu())
			continue;
		if(current[i]!=NULL&&current[i]->state==0)	{
		   task_t* t=current[_cpu()];
		   t->state=0;//runable now
		   current[_cpu()]=current[i];
	       current[i]=t;
		   current[_cpu()]->state=2;//running
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
		if(current[i]==NULL) {
			current[i]=task;
			printf("\033[task%d\n\033[0m",i);
			break;
		}
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
// task over
void kmt_init() {
	printf("set started\n");
	os->on_irq(-19999,_EVENT_NULL,context_save);
	printf("!!!!\n");
	os->on_irq(19999,_EVENT_NULL,context_switch);
	printf("set over\n");
	for(int i=0;i<8;i++) 	{
		char pre[100];
		sprintf(pre,"empty%d",i);
		create(pmm->alloc(sizeof(task_t)),pre,noreach,NULL);
	} 
	spin_init(&tsk_lk,"task");
	spin_init(&prf_lk,"printf");
}
MODULE_DEF(kmt) {
	.init = kmt_init,
	.spin_init=spin_init,
	.sem_init=sem_init,
	.spin_lock=spin_lock,
	.spin_unlock=spin_unlock,
	.sem_wait=sem_wait,
	.sem_signal=sem_signal,
	.create=create,
	.teardown=teardown,
};
