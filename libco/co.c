#include <stdio.h>
#include "co.h"
#include <setjmp.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#define MAX_CO 1024
#define MAX_HEAP_SIZE 4096
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
	char argc[100] __attribute__((aligned(SIZE_align)));
	char name[100];
	int sleep;
	int start;
	int dead;
	struct co * back;
	struct co * par;
}runtines[MAX_CO*2];
struct co * current;
int rec_sta[MAX_CO*2],rec_top;
void co_init() {
	current=&runtines[0];
	current->sleep=0;
	current->dead=0;
	for(int i=1;i<MAX_CO;i++)	{ 
		runtines[i].sleep=1;
		runtines[i].dead=1;
		runtines[i].back=NULL;
		runtines[i].par=NULL;
		runtines[i].start=0;
		runtines[i].SP=malloc(sizeof(char)*4096);//(void *)runtines[i].stack;
		rec_sta[rec_top++]=MAX_CO-i;
	}
}
void co_func(struct co *thd)  {
	current=thd;
//	asm volatile ("mov %0," _SP :	:"g"(thd->SP));
	(*(current->func))((void *)current->argc);
	if(current->back!=NULL)
		current->back->sleep=0;//wake the thd in wait
	current->dead=1;//thd ends
	assert(current->par!=NULL);
//	asm volatile("mov %0," _SP : :"g"(current->RSP));
}
struct co* co_start(const char *name, func_t func, void *arg) {
  struct co* new_co=&runtines[rec_sta[--rec_top]];
  new_co->func=func;
  asm volatile ("mov " _SP ",%0"  : "=g" (new_co->RSP) : );
  strcpy(new_co->name,name);
  strcpy(new_co->argc,(char *)arg);
  new_co->dead=0;
  new_co->sleep=0;
  return new_co;
}
void co_yield() {
	struct co *rc=current;
	if(!setjmp(current->buf))	{//first return , change  current
/*		for(int i=1;i<MAX_CO;i++)	{
			if(&runtines[i]==current)	{
				continue;
		 	}
			if(!runtines[i].sleep&&!runtines[i].dead)	{
				current=&runtines[i];
				if(current->start){
					longjmp(current->buf,1);
		 		}
				else	{
				   	current->start=1;
					current->par=rc;	
				 	co_func(current);
		 		}
				break;
		 	}
		}*/ 
	}
	current=rc;
}
void co_wait(struct co *thd) {
	struct co *rc=current;
	if(!setjmp(thd->buf))	{
		current->sleep=1;
		if(thd->sleep||thd->back!=NULL)	{
			assert(0);
		}
		thd->back=current;
		current=thd;			
		if(!thd->start)	{
			thd->start=1;				
			thd->par=rc;
			co_func(thd);
		}
		else {
			longjmp(current->buf,1);
		}
	}
	current=rc;
}

