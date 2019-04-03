static int *btmp;
static int mpsize;
#define lb(x) (x&(-x))
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
	while(bit_size<mpsize&&btmp[bit_size]) bit_size+=barbit;
	assert(bit_size<=mpsize);
	int ans=bit_size;
	while(bit_size<=mpsize) {
		btmp[bit_size]=1;
		bit_size+=lb(bit_size);
		printf("%d\n",bit_size);
	}
	return ans;
}
void bt_free(intptr_t pos) {
	assert(btmp[pos]==1);
	while(pos<=mpsize) {
		btmp[pos]=0;
		pos+=lb(pos);
	}
}
