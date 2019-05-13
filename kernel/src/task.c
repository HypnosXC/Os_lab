#include<common.h>
#include<klib.h>
#define FL_IF 0x00000200  
#define STACK_SIZE 4096
#define TASK_SIZE 64
task_t *current[TASK_SIZE];
task_t *loader[TASK_SIZE];
spinlock_t ct_lk,tsk_lk,yield_lk;
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
void spin_init(spinlock_t *lk,const char *name) {
	lk->ff=111;
	lk->bf=222;
	lk->locked=0;
	lk->hcpu=1000;
	strcpy(lk->name,name);
	printf("lk %p=%s,ff=%d,bf=%d\n",lk,lk->name,lk->ff,lk->bf);
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
	/*cli();//disable int errupts
	if(lk->locked&&lk->hcpu==_cpu()) {
		printf("\n\033[31m fk lock reholding ,cpu#%d for %s\n\033[0m",_cpu(),lk->name);
		assert(0);
	}*/
	pushcli();
	if(lk->ff!=111|| lk->bf!=222)	{			
		printf("\n lock:%s shit lock changed in space coindently,ff=%d,bf=%d\n",lk->name,lk->ff,lk->bf);
		assert(0);
	}
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
			printf("\033[31mlock:%s,wrong cpu unlock at%d,original %d\n\033m",lk->name,_cpu(),lk->hcpu);// different cpu ,one hold, but another unlock
		assert(0);
	}
	lk->hcpu=1000;
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
	sem->tk_list=NULL;
	printf("\n\033[41m sem init:\033[43 name=%s,value=%d\n",name,value);
}
void sem_append(task_t *tk,tasklist_t *ls) {
	if(ls==NULL) {
		ls=pmm->alloc(sizeof(tasklist_t));
		ls->next=NULL;
		ls->tk=tk;
		return;
	}
	else {
		while(ls->next!=NULL)
			ls=ls->next;
		ls->next=pmm->alloc(sizeof(tasklist_t));
		ls->next->tk=tk;
		ls->next->next=NULL;
	}
}
tasklist_t* sem_decline(tasklist_t * ls) {
	tasklist_t *nt=ls->next;
	pmm->free(ls);
	return nt;
}
void sem_wait(sem_t *sem) {
	spin_lock(&sem->sem_lk);
	task_t* cur=current_task();
//	sem->value--;
	printf("\n\033[41m sem_wait : \033[42m cpu%d for %s,value is%d\033[0m\n",_cpu(),sem->name,sem->value*-1);
	sem->value--;
	if(sem->value<0) {
		if(cur->park!=1)//no sleeped before or waken but no resourse
			sem_append(cur,sem->tk_list);
		cur->park=1;//sleep;
//		assert(0);
		spin_unlock(&sem->sem_lk);
		printf("sem_yield!\n");
		_yield();
		spin_lock(&sem->sem_lk);
	}
	printf("\n\033[41m sem_wait : cpu#%d ,name= %s over! \033[0m\n",_cpu(),sem->name);
	cur->park=0;
	spin_unlock(&sem->sem_lk);
}
void sem_signal(sem_t *sem) {
	spin_lock(&sem->sem_lk);
	sem->value++;
	printf("\n\033[41m sem_signal: \033[42m cpu%d for %s\033[0m\n",_cpu(),sem->name);
	sem->tk_list->tk->park=0;//enrunable
	sem->tk_list=sem_decline(sem->tk_list);
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
	int cnt=0;
 	while(1){
		cnt++;
//		spin_lock(&prf_lk);
		if(cnt>1e9) {
			cnt=0;
			printf("cpu=%d\n",_cpu());
		}
//		spin_unlock(&prf_lk);
	//	spin_lock(&yield_lk);
		_yield();
	//	spin_unlock(&yield_lk);
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
	for(;i<TASK_SIZE;i++)
		if(loader[i]==NULL) {
			loader[i]=task;
			printf("\033[task%d\n\033[0m",i);
			break;
		}
	spin_unlock(&tsk_lk);
	return i;
}
void teardown(task_t *task) {
	spin_lock(&tsk_lk);
	for(int i=0;i<TASK_SIZE;i++)
		if(loader[i]==task)
				current[i]=NULL;
	pmm->free(task->stack.start);
	pmm->free(task);
	spin_unlock(&tsk_lk);
}
_Context* context_save(_Event e,_Context *c) {
	spin_lock(&ct_lk);
	if(current[_cpu()]==NULL) {
		int pid=create(pmm->alloc(sizeof(task_t)),"null",noreach,NULL);
		printf("got here!\n");
		 current[_cpu()]=loader[pid];
		 current[_cpu()]->state=2;//running
	}
    current[_cpu()]->context=c;
	printf("save as %s,eip=%d\n",current[_cpu()]->name,c->eip);
	spin_unlock(&ct_lk);
	return NULL;
}
_Context* context_switch(_Event e,_Context* c) {
	spin_lock(&ct_lk);
	_Context* ret=NULL;
	for(int i=_cpu();i<TASK_SIZE;i+=_ncpu()) {
	 	if(loader[i]==NULL||loader[i]->park)//empty
			continue;
//		printf("\033[41m task :\033[42m num %d, park %d,state %d\033[0m\n",i,current[i]->park,current[i]->stat
	//	printf("current is %d\n",current[i]->state);
		if(loader[i]->state==0)	{
		   current[_cpu()]->state=0;//runable
		   current[_cpu()]=loader[i];
		   current[_cpu()]->state=2;//running
		   ret=current[_cpu()]->context;
		   break; 
		}
	}
	if(ret==NULL) {
		printf("\nno runable task! cpu=%d,total=%d\n",_cpu(),_ncpu());
		int i=_cpu();
		for(;i<TASK_SIZE;i+=_ncpu())
			if(loader[i]!=NULL&&!strcmp(loader[i]->name,"null"))
				break;
//		printf("find task=%d,%s\n",i,current[i]->name);
		if(i!=_cpu()) {
			task_t* t=current[_cpu()];
			t->state=0;//runable now
			current[_cpu()]=loader[i];
			current[_cpu()]->state=2;//running
		}
		ret=current[_cpu()]->context;
	}
	spin_unlock(&ct_lk);
	printf("\nreturn task=%s\n",current[_cpu()]->name);
	return ret;
}
// task over
void kmt_init() {
	printf("set started\n");
	spin_init(&ct_lk,"save and switch"); 
	spin_init(&tsk_lk,"task");
	spin_init(&yield_lk,"yield");
	os->on_irq(-19999,_EVENT_NULL,context_save);
//	printf("!!!!\n");
	os->on_irq(19999,_EVENT_NULL,context_switch);
//	printf("set over\n");
 //	for(int i=0;i<8;i++) 	{
//		char pre[100];
//		sprintf(pre,"empty%d",i);
//		printf("%d!\n",i);
//		create(pmm->alloc(sizeof(task_t)),pre,noreach,NULL);
//		null[i]=current[pid];
//		current[pid]=NULL;
//	}

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
