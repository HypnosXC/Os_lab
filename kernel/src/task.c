#include<common.h>
#include<klib.h>
#define STACK_SIZE 4096
task_t *current[32];
spinlock_t tsk_lk;

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
// spin_lock finished





//sem started
void sem_init(sem_t *sem,const char *name,int value){
	spin_init(&sem->sem_lk,name);
	sem->value=value;
	sem->name=pmm->alloc(sizeof(name));
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
// task over
void kmt_init() {
	os->on_irq(-19999,_EVENT_NULL,context_save);
	os->on_irq(19999,_EVENT_NULL,context_switch);
	for(int i=0;i<8;i++)	{
		create(pmm->alloc(sizeof(task_t)),"empty",noreach,NULL);
	} 
	spin_init(&tsk_lk,"task");
}
