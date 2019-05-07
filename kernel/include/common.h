#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <nanos.h>
#include <def.h>
//<<<<<<< HEAD
//=======

struct task {};
struct spinlock {
	int locked;
	int hcpu;
	char *name;
};
//struct spinlock alloc_lc={0,0,"alloc"};
struct semaphore {};

//>>>>>>> dad0034cd442046d9cc407654dce68cdc0fd783e
#endif
