#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "spinlock.h"

#define TESTALLOCSIZE 9
#define NTEST 1000

struct test{
	char data[TESTALLOCSIZE];
};

void slabtest(){
#if 1
	int i;
	struct test *t[NTEST];

	slabdump();

	for(i=0;i<NTEST;i++){
		t[i] = (struct test *)kmalloc(sizeof(struct test));
	}

	slabdump();
	
	for(i=NTEST-1;i>=0;i--){
		kmfree((char *)t[i], sizeof(struct test));
	}
#endif
	slabdump();
}

int sys_slabtest(){
	slabtest();
	return 0;
}
