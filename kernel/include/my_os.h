intptr_t atomic_xchg(volatile intptr_t *addr,intptr_t newval) {
	intptr_t result;
	asm volatile ("lock xchg %0,%1": "+m"(*addr),"=a"(result):"1"(newval):"cc");
	return result;
}
typedef struct _lock{
	char *name;
	int state;
}lc;
lc bas_lc[1000];
#define allc_lc (bas_lc+1)
void lock(lc * lk) {
	printf("locked %s\n",lk->name);
	while(atomic_xchg(&lk->state,1));
}
void unlock(lc *lk) {
	printf("unlocked %s\n",lk->name);
	atomic_xchg(&lk->state,0);
}
static int *btmp;
static int mpsize,head,tail;
void  bt_init(intptr_t *addr,int size)	{// each with 128,originally ,then used to point to the linked blocked.(head size+1,tail size+2)
    btmp=addr;
    mpsize=size;
    for(int i=0;i<size;i++)
	   btmp[i]=0;
    mpsize=size;
    head=size+1;
    tail=size+2;
}
