int printf(const char *mtf,...);
inline intptr_t atomic_xchg(volatile intptr_t *addr,intptr_t newval) {
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
#define printf_lk (bas_lc+2)
inline void lock(lc * lk) {
	printf("goes lock");
	while(atomic_xchg(&lk->state,1));
}
#define newlk(name) {\
	bas_lc[++tot].name=name;\
	return bas_lc+tot;\
}
inline void unlock(lc *lk) {
	atomic_xchg(&lk->state,0);
}

