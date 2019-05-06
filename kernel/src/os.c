#include <common.h>
#include <my_os.h>
#include <klib.h>
int rand();
int printf(const char * tmf,...);
int sprintf(char * g,const char *tmf,...);
static void os_init() {
  pmm->init();
  printf_lk->name="printf";
}

void test_full(){
  //init_lock(&test_lock, 'b');
  //printf("test_full: lock %p\n", &test_lock);
  int *p = NULL;
  int *p_old = NULL;
  int term = 0;
  while((p = pmm->alloc(1000*sizeof(int)))){
    //spin_lock(&test_lock);
 //   printf("\33[1;35mtest_full: I'm at %p, %d\n\33[0m", (uintptr_t)p,_cpu());
    //Assert(test_lock.slock == 1, "test_full: test_lock.slock值为0");
    //spin_unlock(&test_lock);
    for(int i=0;i < 1000;i++){
      //printf("test_full: I'm at %p, %d\n", p, i);
      p[i] = i;
    }
    if(p_old != NULL){
      for(int i=0;i < 1000;i++){
	if(p_old[i]!=i)	{
		printf("test_full: 旧值被改变");
		assert(0);	
	}
      }
    }
    if(p_old != NULL)
      pmm->free(p_old);
    p_old = p;
    term++;
    /*
    if(term >= 5)
      break;
      */
  }
}

void test_big_small(){
  int *p = NULL;
  int *p_old = NULL;
  size_t size[] = {
    100000/sizeof(int)/10*3,
    100000/sizeof(int)/10*1
  };
  int num = sizeof(size)/sizeof(size[0]);
  for(int i=0;;i++){
    if((p = pmm->alloc(size[i%num]*sizeof(int))) == NULL)
      break;
    //printf("test_big_small: start: %d\n", (uintptr_t)p);
    printf("\33[1;35mtest_big_small: I'm at %d, %d, %d\n\33[0m", (uintptr_t)p, i, _cpu());
    for(int j=0;j < size[i%num];j++){
      p[j] = j;
    }
    
    //printf("test_big_small: %d %d\n", (uintptr_t)p, (uintptr_t)p_old);

    if(p_old != NULL){
      for(int j=0;j < size[(i-1)%num];j++){
        if(p_old[j]!=j)	{
		printf("test_bs: 旧值被改变");
		assert(0);	
	}
      }
    }
    if(p_old != NULL)
      pmm->free(p_old);
    p_old = p;
    /*
    if(i >= 10)
      break;
    */
  }
}

static void hello() {
  for (const char *ptr = "Hello from CPU #"; *ptr; ptr++) {
    _putc(*ptr);

  }
  _putc("12345678"[_cpu()]); _putc('\n');
}
void test(){
    int *space[100];
    int num[100];
    int i;
    for(i=0;i<100;++i){
        num[i] = rand()%((1<<5)-1);
        space[i]=pmm->alloc(num[i]);
        for(int j=0;j < num[i]/sizeof(int);j++)
          space[i][j] = j;
    }
    for(i=0;i<1000;++i){
        int temp=rand()%10;
        for(int j=0;j < num[temp]/sizeof(int);j++)
          assert(space[temp][j] == j);
        pmm->free(space[temp]);
        num[temp] = rand()%((1<<5)-1);
        space[temp]=pmm->alloc(num[temp]);
        for(int j=0;j < num[temp]/sizeof(int);j++)
          space[temp][j] = j;
    }
    for(i=0;i<100;++i){
        for(int j=0;j < num[i]/sizeof(int);j++)
          assert(space[i][j] == j);
        pmm->free(space[i]);
    }
    printf("test: I finished\n");
}
/*void test() {
    int* q[1009];
	int top=0;
	for( int i=1;i<=1000;i++){
		int len=rand()%1000*10+100;
		int f=rand()%5;
		if(!top||f)	{
			q[top++]=pmm->alloc(len);
			*q[top-1]=len;
			lock(printf_lk);
				printf("alloc %p with len %d",q[top-1],len);
			unlock(printf_lk);
		}
		else {
			lock(printf_lk);
			printf("\n%d,pop=%p,cpu%d\n",i,q[top-1],_cpu());
			unlock(printf_lk);
			pmm->free(q[top-1]);
			top--;
		}
	}
        while(top){
	  lock(printf_lk);
	  printf("start empty at %p,cpu %d\n",q[top-1],_cpu());
	  unlock(printf_lk);
	  pmm->free(q[--top]);
	}	  
}*/
static void os_run() {
  hello();
//  test_full();
//  test();
  _intr_write(1);
  while (1) {
    _yield();
  }
}

static _Context *os_trap(_Event ev, _Context *context) {
  return context;
}

static void os_on_irq(int seq, int event, handler_t handler) {
}

MODULE_DEF(os) {
  .init   = os_init,
  .run    = os_run,
  .trap   = os_trap,
  .on_irq = os_on_irq,
};
