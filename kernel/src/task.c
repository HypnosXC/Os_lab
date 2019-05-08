#include<common.h>
#include<klib.h>
#define STACK_SIZE 4096
task_t *current[32];
static int top;
spinlock tsk_lk;
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
		   task* t=current[_cpu()];
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
	task->name=name;
	task->state=0;
	task->stack.start=pmm->alloc(STACK_SIZE);
	task->stack.end=(void *)((intptr_t)task->stack->start+STACK_SIZE);
	task->context=_kcontext(task->stack,entry,arg);
	printf("new task:%s\n",name);
	for(int i=0;i<32;i++)
		if(current[i]==NULL)
			current[i]=task;
	spin_unlock(&tsk_lk);
}
void teardown(task_t *task) {
	spin_lock(&tsk_lk);
	for(innt i=0;i<32;i++)
		if(current[i]==task)
				current[i]=NULL;
	pmm->free(task->stack->start);
	pmm->free(task);
	spin_unlock(&tsk_lk);
}
void kmt_init() {
	os->on_irq(-19999,_ENENT_NULL,context_save);
	os->on_irq(19999,_EVENT_NULL,context_switch);
	for(int i=0;i<8;i++)	{
		create(pmm->alloc(sizeof(task_t)),"empty",noreach,NULL);
	}
	spin_init(&tsk_lk,"task");
}
