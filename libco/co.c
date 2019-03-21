#include <stdio.h>
#include "co.h"
#include <setjmp.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#define MAX_CO 1024
#define MAX_HEAP_SIZE 16384
#if defined(__i386__)
	#define _SP "%%esp"
#elif defined(__x86_64__)
	#define _SP "%%rsp"
#endif
#if defined(__i386__)
	#define  SIZE_align 8
#elif defined(__x86_64__)
	#define SIZE_align 16
#endif
	

struct co {
	void *RSP __attribute__ ((aligned(SIZE_align)));
	void* SP __attribute__((aligned(SIZE_align)));
	jmp_buf buf;
	func_t func;
	//argc[100] __attribute__((aligned(SIZE_align)));
	void *arg;
	char name[100];
	int sleep;
	int start;
	int dead;
	struct co * back;
	struct co * par;
}runtines[MAX_CO*2];
struct co * current;
int rec_sta[MAX_CO*2],rec_top;
void co_change(struct co* target) {//only use for one context over
	if(target->sleep)//invaild operation
		assert("wrong op"&& 0);
	if(target->dead)
		co_change(target->par);
	longjmp(target->buf,1);
}
void co_init() {
	for(int i=0;i<MAX_CO;i++)	{ 
		runtines[i].sleep=1;
		runtines[i].dead=1;
		runtines[i].back=NULL;
		runtines[i].par=NULL;
		runtines[i].start=0;
		rec_sta[rec_top++]=MAX_CO-i;
	}
	rec_top=0;	
	current=&runtines[0];
	current->sleep=0;
	current->dead=0;
}
void co_func()  {
	asm volatile ("mov %0," _SP :	:"g"(current->SP));
	(*(current->func))((void *)current->arg);
	if(current->back!=NULL)
		current->back->sleep=0;//wake the thd in wait
	current->dead=1;//thd ends
	assert(current->par!=NULL);//no parent is impossible
	co_change(current->par);
}
struct co* co_start(const char *name, func_t func, void *arg) {
  struct co* new_co=&runtines[++rec_top];
  new_co->func=func;
  strcpy(new_co->name,name);
  //strcpy(new_co->argc,(char *)arg);
  new_co->arg=arg;
  printf("%p\n",arg);
  new_co->dead=0;
  new_co->start=0;
  new_co->par=new_co->back=NULL;
  new_co->SP=malloc(MAX_HEAP_SIZE*sizeof(char));
  new_co->SP+=MAX_HEAP_SIZE*sizeof(char);
  new_co->sleep=0;
  return new_co;
}
void co_yield() {
	struct co * rc=current;
	if(!setjmp(current->buf))	{//first return , change  current
		for(int j=1;j<MAX_CO;j++)	{
			int i=rand()%rec_top+1;
			if(&runtines[i]==current)	{
				continue;
	 		}
			if(!runtines[i].sleep&&!runtines[i].dead)	{
				current=&runtines[i];
				if(current->start){
					longjmp(current->buf,1);
		 			assert(0);//never reach
				}
				else	{
				   	current->start=1;
					current->par=rc;	
				 	co_func();
		 		}
				break;
		 	}
		}
	}
	current=rc;
}
void co_wait(struct co *thd) {
	struct co *rc=current;
	if(!setjmp(rc->buf))	{
		rc->sleep=1;
		if(thd->sleep||thd->back!=NULL)	{
			assert(0);
		}
		thd->back=rc;
		current=thd;		
		if(!thd->start)	{
			thd->start=1;				
			thd->par=rc;
			co_func();
		}
		else {
			longjmp(current->buf,1);
		}
	}
	current=rc;
}

