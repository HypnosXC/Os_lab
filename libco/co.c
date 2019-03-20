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
	void* ori_SP __attribute((aligned(SIZE_align)));
	void* SP __attribute__((aligned(SIZE_align)));
	jmp_buf *buf;
	func_t func;
	char argc[100] __attribute__((aligned(SIZE_align)));
	char name[100];
	int sleep;
	int dead;
}runtines[MAX_CO];
struct co * current;
int rec_sta[MAX_CO],rec_top;
void co_init() {
	current=&runtines[0];
	current->sleep=0;
	current->dead=0;
	for(int i=1;i<=MAX_CO;i++)	{
		runtines[i].sleep=1;
		runtines[i].dead=1;
		runtines[i].buf=NULL;
		runtines[i].SP=malloc(MAX_HEAP_SIZE*sizeof(char));
		runtines[i].SP+=MAX_HEAP_SIZE/2*sizeof(char);
//		printf("%p\n",runtines[i].SP);
		runtines[i].ori_SP=0;
		rec_sta[i-1]=i;
	}
	rec_top=MAX_CO;
}
void co_func(struct co *thd)  {
	asm volatile ("mov " _SP ",%0;mov %1, " _SP :
		  	"=g"(thd->ori_SP) :
			"g"(thd->SP));
	printf("%p %s\n",thd->SP,thd->name);
	(*(thd->func))((void *)thd->argc);
	asm volatile("mov %0," _SP : :"g"(thd->ori_SP));
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
	printf("yiedld once at current=%p",current);
	if(!setjmp(*current->buf))	{//first return , change current
		for(int i=1;i<=MAX_CO;i++)	{
			if(!runtines[i].sleep&&!runtines[i].dead)	{
				current=&runtines[i];
				if(current->buf!=NULL)
					longjmp(*current->buf,1);
				else 
					co_func(current);
			}
		}
	}
	current=rc;
}

void co_wait(struct co *thd) {
	struct co *rc=current;
	current->sleep=1;
	for(int i=1;i<=MAX_CO;i++)
		if(thd==&runtines[i])	{
			if(thd->sleep||thd->dead)	{
				printf("wait for dead %d or sleeping %d thd!",thd->dead,thd->sleep);
				assert(0);
			}
			current=thd;
			if(thd->buf==NULL)	{
				printf("here?\n");
				co_func(thd);
			}
			else
				longjmp(*thd->buf,1);
			thd->dead=1;
			rec_sta[rec_top++]=i;
			break;
		}
	current=rc;
	rc->sleep=0;
}

