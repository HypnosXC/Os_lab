
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

