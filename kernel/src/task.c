#include<common.h>
#include<klib.h>
#define FL_IF 0x00000200  
#define STACK_SIZE 4096
task_t *current[32];
spinlock_t tsk_lk,yield_lk;
int rand();
// spin_lock started
static int cpu_cnt[100];
static int cpu_intr[100];
void cli(){
	asm volatile ("cli"); 
}
void sti() {
	asm volatile ("sti");
}
int readeflags(){
  int eflags;
  asm volatile("pushfl; popl %0" : "=r" (eflags));
  return eflags;
};
void spin_init(struct spinlock *lk,const char *name) {
	lk->locked=0;
	lk->hcpu=-1;
	strcpy(lk->name,name);
	printf("lk %s=%s\n",name,lk->name);
}
void pushcli() {
	int eflags=readeflags();
	cli();
	if(cpu_cnt[_cpu()]==0)
		cpu_intr[_cpu()]=eflags&FL_IF;
	cpu_cnt[_cpu()]++;
}
void popcli() {
	cpu_cnt[_cpu()]--;
	if(!cpu_cnt[_cpu()]&&cpu_intr[_cpu()])
		sti();
}
int holding(spinlock_t *lk) {
	int r;
	pushcli();
	r= lk->locked && lk->hcpu==_cpu();
	popcli();
	return r;
}
void spin_lock(struct spinlock *lk) {
	/*cli();//disable interrupts
	if(lk->locked&&lk->hcpu==_cpu()) {
		printf("\n\033[31m fk lock reholding ,cpu#%d for %s\n\033[0m",_cpu(),lk->name);
		assert(0);
	}*/
	pushcli();
	if(holding(lk)) {
		printf("\n\033[31m fk lock reholding ,cpu#%d for %s\n\033[0m",_cpu(),lk->name);
		assert(0);
 	}
	while(_atomic_xchg(&lk->locked,1)!=0);
//	printf("\ncpu#%d holding the lock %s\n",_cpu(),lk->name);
	__sync_synchronize();
	lk->hcpu=_cpu();
}
int cnt_cpu() {
	return cpu_cnt[_cpu()];
}
void spin_unlock(struct spinlock *lk) {
/*	if(_atomic_xchg(&lk->locked,0)!=1) {
		printf("\033[31m%s: unlock but no hold!\n\033[0m",lk->name);
		assert(0);
	}
	int cpu_n=_cpu();
	if(cpu_n!=lk->hcpu){
		printf("\033[31m%s: wrong cpu unlock at%d,original %d\n\033m",lk->name,cpu_n,lk->hcpu);// different cpu ,one hold, but another unlock
		assert(0);
	}
	printf("\ncpu#%d realse the lock %s\n",_cpu(),lk->name);
	cpu_cnt[lk->hcpu]--;
//	printf("lk=%s,cpu%d,lkcnt=%d\n",lk->name,lk->hcpu,cpu_cnt[lk->hcpu]);
	__sync_synchronize();
	if(cpu_cnt[lk->hcpu]==0)
		sti();//enable intertupt when no lock*/
	if(!holding(lk)) {
			printf("\033[31m%s: wrong cpu unlock at%d,original %d\n\033m",lk->name,_cpu(),lk->hcpu);// different cpu ,one hold, but another unlock
		assert(0);
	}
	lk->hcpu=-1;
	__sync_synchronize();
	asm volatile("movl $0, %0" : "+m"(lk->locked) : );
//	printf("cpu#%d realse the lock %s",_cpu(),lk->name);
	popcli();
}
// spin_lock finished





//sem started
void sem_init(sem_t *sem, const char *name,int value){
	spin_init(&sem->sem_lk,name);
	sem->value=value;
    strcpy(sem->name,name);
	sem->top=0;
	printf("\n\033[41m sem init:\033[43 name=%s,value=%d\n",name,value);
}
void sem_wait(sem_t *sem) {
	spin_lock(&sem->sem_lk);
	task_t* cur=current_task();
//	sem->value--;
	printf("\n\033[41m sem_wait : \033[42m cpu%d for %s,value is%d\033[0m\n",_cpu(),sem->name,sem->value*-1);
	while(sem->value<=0) {
		if(cur->park!=1)//no sleeped before or waken but no resourse
			sem->sem_st[sem->top++]=cur;
		cur->park=1;//sleep;
//		printf("no hanlded yet for sem yield");
//		assert(0);
		spin_unlock(&sem->sem_lk);
		spin_lock(&yield_lk);
		_yield();
		spin_unlock(&yield_lk);
		spin_lock(&sem->sem_lk);
	}
	printf("\n\033[41m sem_wait : cpu#%d ,name= %s over! \033[0m\n",_cpu(),sem->name);
    sem->value--;
	cur->park=0;
	spin_unlock(&sem->sem_lk);
}
void sem_signal(sem_t *sem) {
	spin_lock(&sem->sem_lk);
	sem->value++;
	printf("\n\033[41m sem_signal: \033[42m cpu%d for %s\033[0m\n",_cpu(),sem->name);
	sem->sem_st[--sem->top]->park=0;//runable
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
//		spin_lock(&prf_lk);
//		printf("cpu=%d\n",_cpu());
//		spin_unlock(&prf_lk);
		spin_lock(&yield_lk);
		_yield();
		spin_unlock(&yield_lk);
	}
}
task_t* current_task() {
	return current[_cpu()];
}
int create(task_t *task,const char *name,void (*entry)(void *arg),void *arg) {
	spin_lock(&tsk_lk);
	strcpy(task->name,name);
	task->state=0;
	task->park=0;
	task->stack.start=pmm->alloc(STACK_SIZE);
	task->stack.end=(void *)((intptr_t)task->stack.start+STACK_SIZE);
	task->context=_kcontext(task->stack,entry,arg);
	printf("new task:%s\n",name);
	int i=0;
	for(;i<32;i++)
		if(current[i]==NULL) {
			current[i]=task;
			printf("\033[task%d\n\033[0m",i);
			break;
		}
	spin_unlock(&tsk_lk);
	return i;
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
_Context* context_save(_Event e,_Context *c) {
	spin_lock(&tsk_lk);
	if(current[_cpu()]==NULL) {
		int pid=create(pmm->alloc(sizeof(task_t)),"null",noreach,NULL);
		 current[_cpu()]=current[pid];
		 current[_cpu()]->state=2;//running
	}
    current[_cpu()]->context=c;
	spin_unlock(&tsk_lk);
	return NULL;
}
_Context* context_switch(_Event e,_Context* c) {
	spin_lock(&tsk_lk);
	int ind=32/_ncpu();
	int cnt=0;
	while(1)	{
		cnt++;
		if(cnt>100000)	{
			printf("\033[41m No runable task!\033[0m\n");
			assert(0);
		}
		int i=(rand()%ind)*_ncpu()+_cpu();
		if(i>=32)//too large random
			continue;
	 	if(current[i]==NULL||current[i]->park)//empty
			continue;
//		printf("\033[41m task :\033[42m num %d, park %d,state %d\033[0m\n",i,current[i]->park,current[i]->state);
//		if(current[i]->park)//sleep
//			continue;
	//	printf("current is %d\n",current[i]->state);
		if(current[i]->state==0)	{
		   task_t* t=current[_cpu()];
		   t->state=0;//runable now
		   current[_cpu()]=current[i];
	       current[i]=t;
		   current[_cpu()]->state=2;//running
		   break;	   
		}
	}
	if(current[_cpu()]->context==NULL) {
		printf("wrong switch!\n");
		assert(0);
	}
	_Context* re=current[_cpu()]->context;
	spin_unlock(&tsk_lk);
//	printf("\nreturn task=%s\n",current[_cpu()]->name);
	return re;
}
// task over
void kmt_init() {
//	printf("set started\n");
	os->on_irq(-19999,_EVENT_NULL,context_save);
//	printf("!!!!\n");
	os->on_irq(19999,_EVENT_NULL,context_switch);
//	printf("set over\n");
 	for(int i=0;i<8;i++) 	{
		char pre[100];
		sprintf(pre,"empty%d",i);
		create(pmm->alloc(sizeof(task_t)),pre,noreach,NULL);
	} 
	spin_init(&tsk_lk,"task");
	spin_init(&yield_lk,"yield");
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
