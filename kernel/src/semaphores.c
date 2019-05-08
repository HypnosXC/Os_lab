#include<common.h>
#include<klib.h>
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
		spin_unlock(&sem->sem_lk);
		_yield();
		spin_lock(&sem->sem_lk);
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
