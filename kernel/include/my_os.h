int printf(const char *mtf,...);
static intptr_t atomic_xchg(volatile intptr_t *addr,intptr_t newval) {
	intptr_t result;
	asm volatile ("lock xchg %0,%1": "+m"(*addr),"=a"(result):"1"(newval):"cc");
	return result;
}
typedef struct _lock{
	char *name;
	int state;
}lc;
lc bas_lc[1000];
int tot=2;
#define allc_lc (bas_lc+1)
#define printf_lk (bas_lc+2)
static void lock(lc * lk) {
	while(atomic_xchg(&lk->state,1));
}
#define newlk(name) {\
	bas_lc[++tot].name=name;\
	return bas_lc+tot;\
}
static void unlock(lc *lk) {
	atomic_xchg(&lk->state,0);
}

