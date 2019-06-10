#ifndef __KERNEL_H__
#define __KERNEL_H__

#include <am.h>
#include <nanos.h>

#define MODULE(name) \
  mod_##name##_t; \
  extern mod_##name##_t *name
#define MODULE_DEF(name) \
  extern mod_##name##_t __##name##_obj; \
  mod_##name##_t *name = &__##name##_obj; \
  mod_##name##_t __##name##_obj = 

typedef _Context *(*handler_t)(_Event, _Context *);
typedef struct {
  void (*init)();
  void (*run)();
  _Context *(*trap)(_Event ev, _Context *context);
  void (*on_irq)(int seq, int event, handler_t handler);
} MODULE(os);

typedef struct {
  void (*init)();
  void *(*alloc)(size_t size);
  void (*free)(void *ptr);
} MODULE(pmm);

typedef struct task task_t;
typedef struct spinlock spinlock_t;
typedef struct inode inode_t;
typedef struct semaphore sem_t;
typedef struct filesystem filesystem_t;
typedef struct {
  void (*init)();
  int (*create)(task_t *task, const char *name, void (*entry)(void *arg), void *arg);
  void (*teardown)(task_t *task);
  void (*spin_init)(spinlock_t *lk, const char *name);
  void (*spin_lock)(spinlock_t *lk);
  void (*spin_unlock)(spinlock_t *lk);
  void (*sem_init)(sem_t *sem, const char *name, int value);
  void (*sem_wait)(sem_t *sem);
  void (*sem_signal)(sem_t *sem);
} MODULE(kmt);
typedef file {
	int  refcnt;
	inode_t *inode;
	uint64_t offset;
}file_t;
typedef struct device device_t;
typedef struct devops {
  int (*init)(device_t *dev);
  ssize_t (*read)(device_t *dev, off_t offset, void *buf, size_t count);
  ssize_t (*write)(device_t *dev, off_t offset, const void *buf, size_t count);
} devops_t;
typedef struct {
  void (*init)();
} MODULE(dev);
typedef struct fsops {
	void (*init)(filesystem_t *fs,const char *name,device_t *dev);
	inode_t *(*lookup)(filesystem_t *fs,const char *path,int flags);
	int (*close)(inode_t *inode);
}fsops_t;
struct filesystem {
	fsops_t *ops;
	device_t *dev;
};
typedef struct inodeops{
    int (*open)(file_t *file, int flags);
  	int (*close)(file_t *file);
 	ssize_t (*read)(file_t *file, char *buf, size_t size);
 	ssize_t (*write)(file_t *file, const char *buf, size_t size);
  	off_t (*lseek)(file_t *file, off_t offset, int whence);
 	int (*mkdir)(const char *name);
 	int (*rmdir)(const char *name);
  	int (*link)(const char *name, inode_t *inode);
  	int (*unlink)(const char *name);
  	// 你可以自己设计readdir的功能
}inodeops_t;
typedef struct inode {
  int refcnt;
  void *ptr;
  filesystem_t *fs;
  inodeops_t *ops; 
};
typedef struct {
	void (*init)();
	int (*access)(const char * path,int mode);
	int (*mount)(const char *path,filsystem_t *fs);
	int (*unmont)(const char *path);
	int (*mkdir)(const char *path);
	int (*rmdir)(const char *path);
	int (*link)(const char *oldpath,const char* newpath);
	int (*unlink)(const char *path);
	int (*open)(const char *path,int flags);
	ssize_t (*read)(int fd,void *buf,size_t nbyte);
	ssize_t (*write)(int fd,void *buf,size_t nbyte);
	off_t (*lssek)(int fd,off_t offset,int whence);
	int (*close)(int fd);
} MODULE(vfs);
#endif
