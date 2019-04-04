static int *btmp;
static int mpsize;
#define BLOCK_SIZE 4096
#define base_sz 16
#define lb(x) (x&(-x))
int rand();
int printf(const char *tmf,...);
void  bt_init(intptr_t *addr,int size)  {// each with 128,originally ,then used to point to the linked blocked.(head size+1,tail size+2)
	btmp=addr;
	mpsize=size;
	for(int i=0;i<size;i++)
		btmp[i]=0;//unused.
	mpsize=size;
}
int bt_alloc(size_t size) {//find a consecutive block with length size
 	int bit_size=1;
	while(bit_size<size) bit_size<<=1;//000..10.. for the original one
	int barbit=bit_size<<1;
	while(bit_size<=mpsize&&btmp[bit_size]) bit_size+=barbit;
	assert(bit_size<=mpsize);
	int ans=bit_size;
	for(int i=lb(bit_size)-1;i>=1;i--)	{ 
		btmp[ans-i]++;
	}
	while(bit_size<=mpsize) { 
		btmp[bit_size]++;
		bit_size+=lb(bit_size);
	}
	return ans;
}
void bt_free(intptr_t pos) {
	if(btmp[pos]==0)	{
		printf("fk wrong pos at %d\n",pos);
		assert(0);
	}
	for(int i=lb(pos)-1;i>=1;i--)	{ 
		btmp[pos-i]--;
	}
	while(pos<=mpsize) { 
		btmp[pos]--;
		pos+=lb(pos);
	}
}
void bt_add(int pos){
	while(pos<=mpsize)	{
		btmp[pos]++;
		pos+=lb(pos);
	}
}
