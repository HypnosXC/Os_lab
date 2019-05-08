#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <nanos.h>
#include <def.h>
//<<<<<<< HEAD
//=======

struct task {
	char * name;
	_Context *context;
	_Area stack;
	int state;//sleep,dead or some 
}tasks[32]; 
struct spinlock {
	int locked;
	int hcpu;
	char *name;
};
//struct spinlock alloc_lc={0,0,"alloc"};
struct semaphore {
	int value;
	spinlock sem_lk;
	task_t * sem_st[100];
	int top;
	char *name;
};

//>>>>>>> dad0034cd442046d9cc407654dce68cdc0fd783e
#endif
