#ifndef _CL_H_
#define _CL_H_

#include <fsl.h>
#include <pvr.h>
#include <setjmp.h>
#include <xil_io.h>

#include "../include/base_addr.h"
#include "../include/thread_struct.h"
#include "../include/mutex_manager.h"

#define hapara_offsetof(type, member) ((int)&((type *)0)->member)

#define KERNEL_ARGV_BASE    \
        (SLAVE_LOCAL_MEM_BASE + hapara_offsetof(struct hapara_thread_struct, argv))


#define setArgv(num, member, type) \
		type *member = (type *)Xil_In32(KERNEL_ARGV_BASE + (num << 2))

jmp_buf buf;

typedef struct id_struct_s {
    int isIDGet;
    unsigned int id0;
    unsigned int id1;
} id_struct;


id_struct id = {
        .isIDGet    = 0,
        .id0        = 0,
        .id1        = 0,
};

typedef struct hapara_gen_struct {
	unsigned int org;
	unsigned int len;
	unsigned int numOfSlvs;
	volatile unsigned int isFinished;	//1: finish;
} hapara_gen_t;

void lock_mutex(int num) {
	 while (((struct hapara_mutex_pair *)SCHEDULER_MUTEX_MANAGER_BASE + num)->reg1 != 1)
		 ((struct hapara_mutex_pair *)SCHEDULER_MUTEX_MANAGER_BASE + num)->reg0 = 1;
	((struct hapara_mutex_pair *)SCHEDULER_MUTEX_MANAGER_BASE + num)->reg0 = 0;
}

void release_mutex(int num) {
	((struct hapara_mutex_pair *)SCHEDULER_MUTEX_MANAGER_BASE + num)->reg2 = 1;
	while (((struct hapara_mutex_pair *)SCHEDULER_MUTEX_MANAGER_BASE + num)->reg2 != 0);
}

unsigned int getGlobalID(int d) {
    unsigned int val;
    if (id.isIDGet == 0) {
        getfslx(val, 1, FSL_DEFAULT);
        if (val == 0xFFFFFFFF) {
            //error catch
            id.isIDGet = 0;
            longjmp(buf, 1);
        } else {
            id.isIDGet  = 1;
            id.id0      = val >> 16;
            id.id1      = val & 0x0000FFFF;
        }
        return (d == 0)?id.id0:id.id1;
    }
    id.isIDGet = 0;
    return (d == 0)?id.id0:id.id1;
}


void clean_up() {
    id.isIDGet  = 0;
    id.id0      = 0;
    id.id1      = 0;
}

#endif
