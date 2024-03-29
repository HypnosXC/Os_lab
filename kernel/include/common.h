#ifndef __COMMON_H__
#define __COMMON_H__

#include <kernel.h>
#include <nanos.h>
#include <def.h>
//<<<<<<< HEAD
//=======
//#define spin_lock(c) kmt->spin_lock(c)
//#define spin_unlock(c) kmt->spin_unlock(c)
//#define spin_init(c,name) kmt->spin_init(c,name)
#define BLOCK_SIZE (off_t)2048
#define INODE_MAP_ENTRY (off_t)0
#define DATA_MAP_ENTRY (off_t)2048
#define INODE_ENTRY (off_t)4096
#define DATA_ENTRY (off_t)(4096+2048*sizeof(inode_t))
#define FLSYS_NUM  10
struct task {
	char name[30];
	file_t *flides[100];
	inode_t * preloc;
	char loc[120];
	_Context *context;
	_Area stack;
	int state;//running or some 
	int park;//sleep or not
}tasks[32]; 
struct spinlock {
	int ff;
	int locked;
	int hcpu;
	char name[30];
	int bf;
};
//struct spinlock alloc_lc={0,0,"alloc"};
typedef struct tasklist {
	task_t * tk;
	struct tasklist * next;
}tasklist_t;
struct semaphore {
	int value;
	spinlock_t sem_lk;
	tasklist_t* tk_list;
	char name[30];
};

//>>>>>>> dad0034cd442046d9cc407654dce68cdc0fd783e
#endif
