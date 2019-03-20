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
	asm volatile ("mov " _SP ",%0":"=g"(current->SP):);
	for(int i=1;i<MAX_CO;i++)	{
		runtines[i].sleep=1;
		runtines[i].dead=1;
		runtines[i].back=NULL;
		runtines[i].par=NULL;
		runtines[i].start=0;
		runtines[i].SP=malloc(MAX_HEAP_SIZE*sizeof(char));
		runtines[i].SP+=MAX_HEAP_SIZE/2*sizeof(char);
//		printf("%p\n",runtines[i].SP);
		rec_sta[rec_top++]=MAX_CO-i;
	}
}
void co_func(struct co *thd)  {
	asm volatile ("mov %0, " _SP :	:"g"(thd->SP));
	printf("%p",thd->SP);
	(*(thd->func))((void *)thd->argc);
	printf("teminated here!");
	thd->back->sleep=0;//wake the thd in wait
	thd->dead=1;//thd ends
	for(int i=1;i<MAX_CO;i++)
		if(thd==&runtines[i]) {
			rec_sta[rec_top++]=i;
			break;
		}
	assert(thd->par!=NULL);
	while(thd->par->dead)
		thd->par=thd->par->par;
	asm volatile("mov %0," _SP : :"g"(thd->par->SP));
}
struct co* co_start(const char *name, func_t func, void *arg) {
  struct co* new_co=&runtines[rec_sta[--rec_top]];
  new_co->func=func;
  strcpy(new_co->name,name);
  strcpy(new_co->argc,(char *)arg);
  new_co->dead=0;
  new_co->sleep=0;
  return new_co;
}
void co_yield() {
	struct co *rc=current;
	if(!setjmp(current->buf))	{//first return , change current
		for(int i=1;i<MAX_CO;i++)	{
//			if(&runtines[i]==current)	{
//				continue;
//			}
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
		}
	}
	current=rc;
}
void co_wait(struct co *thd) {
	struct co * rc=current;
	current->sleep=1;
	for(int i=1;i<MAX_CO;i++)
		if(thd==&runtines[i])	{
			if(thd->sleep||thd->back!=NULL)	{
				printf("wait for dead  or sleeping %d thd!",thd->sleep);
				assert(0);
			}
			thd->back=current;
			current=thd;
			if(!thd->start)	{
				printf("goes to func");
				thd->start=1;
				thd->par=rc;
				co_func(thd);
			}
			else {
				">>>>??";
				longjmp(current->buf,1);
			}
		}
	printf("here reach at co_wait!\n");
}

