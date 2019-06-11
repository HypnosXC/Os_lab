//spin_lock.c
//void spin_init(struct spinlock *lk,char *name);
//void spin_lock(struct spinlock *lk);
//void spin_unlock(struct spinlock *lk);
task_t * current_task();
int cnt_cpu();
void new_block(inode_t *inode);
void basic_read(inode_t *inode,off_t offset,char *buf,size_t size);
void basic_write(inode_t *inode,off_t offset,const char *buf,size_t size);
