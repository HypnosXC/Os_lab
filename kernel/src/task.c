#include<common.h>
#include<klib.h>
#define FL_IF 0x00000200  
#define STACK_SIZE 4096
#define TASK_SIZE 32
extern filesystem_t fs_tab[];
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
//	printf("lk %p=%s,ff=%d,bf=%d\n",lk,lk->name,lk->ff,lk->bf);
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
//	printf("\n\033[41m sem init:\033[43 name=%s,value=%d\n",name,value);
}
tasklist_t* sem_append(task_t *tk,tasklist_t *ls) {
	if(ls==NULL) {
		ls=pmm->alloc(sizeof(tasklist_t));
		ls->next=NULL;
		ls->tk=tk;
		return ls;
	}
	else {
		tasklist_t * head=ls;
		while(ls->next!=NULL)
			ls=ls->next;
		ls->next=pmm->alloc(sizeof(tasklist_t));
		ls->next->tk=tk;
		ls->next->next=NULL;
		return head;
	}
}
tasklist_t* sem_decline(tasklist_t * ls) {
	tasklist_t *nt=ls->next;
//	printf("task=%s,signaled!\n",ls->tk->name);
	pmm->free(ls);
	return nt;
}
void sem_wait(sem_t *sem) {
	spin_lock(&sem->sem_lk);
	task_t* cur=current_task();
	sem->value--;
//	printf("\n\033[41m sem_wait : task=%s\033[42m cpu%d for %s,value is%d\033[0m\n",cur->name,_cpu(),sem->name,sem->value*-1);
	if(sem->value<0) {
		if(cur->park!=1)//no sleeped before or waken but no resourse
			sem->tk_list=sem_append(cur,sem->tk_list);
		cur->park=1;//sleep;
//		assert(0);
		spin_unlock(&sem->sem_lk);
//		printf("\n\033[41m sem:wait name=%s,cpu=#%dappend %s,sem_yield!\033[0m\n",sem->name,_cpu(),sem->tk_list->tk->name);
		_yield();
		spin_lock(&sem->sem_lk);
	}
//	printf("\n\033[41m sem_wait : cpu#%d ,name= %s over! \033[0m\n",_cpu(),sem->name);
	cur->park=0;
	spin_unlock(&sem->sem_lk);
}
void sem_signal(sem_t *sem) {
	spin_lock(&sem->sem_lk);
	sem->value++;
///	printf("\n\033[41m sem_signal: wake %s,\033[42m cpu%d for %s\033[0m\n",sem->tk_list->tk->name,_cpu(),sem->name);
	if(sem->tk_list!=NULL) {
		sem->tk_list->tk->park=0;//enrunable
		sem->tk_list=sem_decline(sem->tk_list);
	}
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
	while(1){
		_yield();
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
	task->preloc=fs_tab[0].inode;
	strcpy(task->loc,"/");
	task->stack.start=pmm->alloc(STACK_SIZE);
	task->stack.end=(void *)((intptr_t)task->stack.start+STACK_SIZE);
	task->context=_kcontext(task->stack,entry,arg);
//	printf("new task:%s\n",name);
	int i=0;
	for(;i<TASK_SIZE;i++)
		if(loader[i]==NULL) {
			loader[i]=task;
	//		printf("\033[41m task%d\n\033[0m",i);
			break;
		}
	spin_unlock(&tsk_lk);
	char pre[100];
	memset(pre,0,sizeof(pre));
	sprintf(pre,"/proc/%d",i);
	fs_tab[0]->ops->lookup(&fs_tab[0],4,pre);
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
//	spin_lock(&ct_lk);
	if(current[_cpu()]==NULL) {
		int pid=create(pmm->alloc(sizeof(task_t)),"null",noreach,NULL);
	//	printf("cpud %d got here!\n",_cpu());
		current[_cpu()]=loader[pid];
		for(int i=_cpu();i<=TASK_SIZE;i+=_ncpu())
			if(i!=pid&&loader[i]==NULL) {
				loader[i]=loader[pid];
				loader[pid]=NULL;
			    break;		
			}
		current[_cpu()]->state=2;//running
	}
    current[_cpu()]->context=c;
//	printf("save as %s,eip=%d\n",current[_cpu()]->name,c->eip);
//	spin_unlock(&ct_lk);
	return NULL;
}
_Context* context_switch(_Event e,_Context* c) {
//	spin_lock(&ct_lk);
	_Context* ret=NULL;
	for(int i=TASK_SIZE-_ncpu()+_cpu();i>=_cpu();i-=_ncpu()) { 
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
//		printf("\nno runable task! cpu=%d,total=%d\n",_cpu(),_ncpu());
//		for(int i=_cpu();i<TASK_SIZE;i+=_ncpu()) {
//			if(loader[i]!=NULL)
//			printf("task=%s,park=%d,state=%d   ",loader[i]->name,loader[i]->park,loader[i]->state);
//		}
//		printf("\n");
		int i=TASK_SIZE-_ncpu()+_cpu();
		for(;i>=_cpu();i-=_ncpu()) {
			if(loader[i]!=NULL&&!strcmp(loader[i]->name,"null"))
				break;
		}
//		printf("cpu#%d find task=%d\n",_cpu(),i);
		task_t* t=current[_cpu()];
		t->state=0;//runable now
		current[_cpu()]=loader[i];
		current[_cpu()]->state=2;//running
		ret=current[_cpu()]->context;
	}
///	spin_unlock(&ct_lk);
//		printf("cpu#%d find task=%s,eip=%d\n",_cpu(),current[_cpu()]->name,current[_cpu()]->context->eip);
	return ret;	
}
// task over
void kmt_init() {

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
