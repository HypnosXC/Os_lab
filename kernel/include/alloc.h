static int *btmp;
static int mpsize,head,tail;
#define lb(x )(x&(-x))
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
	while(bit_size<=mpsize) {
		btmp[bit_size]=1;
		bit_size+=lb(bit_size);
	}
	return bit_size;
}
void bt_free(intptr_t pos) {
	assert(btmp[pos]==1);
	while(pos<=size) {
		btmp[pos]=0;
		pos+=lb(bit_size);
	}
}