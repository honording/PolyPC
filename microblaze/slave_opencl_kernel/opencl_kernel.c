/*
 * slave_kernel_threads.c
 *
 *  Created on: Apr 8, 2016
 *      Author: hding
 */


#include <fsl.h>
#include <setjmp.h>

jmp_buf buf;

#define	MIG_BASE	(0x60000000)

typedef struct id_struct_s {
	int isIDGet;
	unsigned int id0;
	unsigned int id1;
} id_struct;

id_struct id = {
		.isIDGet	= 0,
		.id0		= 0,
		.id1 		= 0,
};

unsigned int getGlobalID(int d) {
	unsigned int val;
	if (id.isIDGet == 0) {
		getfslx(val, 1, FSL_DEFAULT);
		if (val == 0xFFFFFFFF) {
			//error catch
			longjmp(buf, 0);
		} else {
			id.isIDGet 	= 1;
			id.id0 		= val >> 16;
			id.id1 		= val & 0x0000FFFF;
		}
		return (d == 0)?id.id0:id.id1;
	}
	id.isIDGet 	= 0;
	return (d == 0)?id.id0:id.id1;
}

void kernel(
			int *A,
			int *B,
			int *C) {
	unsigned int id0 = getGlobalID(0);
	unsigned int id1 = getGlobalID(1);
	C[id0 + id1] = A[id0 + id1] + B[id0 + id1];
}

void clean_up() {
	id.isIDGet 	= 0;
	id.id0		= 0;
	id.id1 		= 0;
}

int main() {
	int *arg0 = (int *)MIG_BASE;
	int *arg1 = (int *)MIG_BASE + 1024;
	int *arg2 = (int *)MIG_BASE + 2048;
	while (1) {
		kernel(arg0, arg1, arg2);
	}
	setjmp(buf);
	clean_up();
	return 0;
}
