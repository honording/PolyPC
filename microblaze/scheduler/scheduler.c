/*
 * scheduler.c
 *
 *  Created on: Apr 8, 2016
 *      Author: hding
 */



/*
#define CONTROL_BASE_ADDR		(0xC0004000)
#define HAPARA_GENERATOR_BASE	(0x44A00000)
#define HAPARA_MUTEX_MANA_BASE	(0x40010000)
#define HAPARA_HTDT_BASE		(0x40000000)
#define HAPARA_DMA_BASE			(0x44A10000)
#define HAPARA_DDR_BASE			(0x60000000)

#define HAPARA_MIG_CDMA_BASE	(0x60000000)
#define HAPARA_INST_CDMA_BASE	(0xC2000000)
#define HAPARA_DATA_CDMA_BASE	(0xC0000000)

//#define DMAAPI

typedef struct {
	unsigned int main_addr;
	unsigned int stack_addr;
	unsigned int thread_size;
} elf_info_t;

typedef struct {
	volatile unsigned int trigger0;
	volatile unsigned int trigger1;
	volatile unsigned int trigger2;
	volatile unsigned int trigger3;
} thread_control_t;

struct hapara_id_pair {
    uint32_t id0;
    uint32_t id1;
};

struct hapara_thread_struct {
    uint8_t valid;                       //1: valid; 0: invalid
    uint8_t type;
    uint8_t priority;
    uint8_t next;
    uint8_t tid;
    struct hapara_id_pair group_id;
    elf_info_t	elf_info;
}__attribute__((aligned(4)));



typedef struct {
	unsigned int org;
	unsigned int len;
	unsigned int numOfSlvs;
	volatile unsigned int isFinished;	//1: finish;
} hapara_gen_t;

struct hapara_mutex_pair
{
	volatile uint32_t reg0;
	volatile uint32_t reg1;
	volatile uint32_t reg2;
};

void lock_mutex(int num) {
	 while (((struct hapara_mutex_pair *)HAPARA_MUTEX_MANA_BASE + num)->reg1 != 1)
		 ((struct hapara_mutex_pair *)HAPARA_MUTEX_MANA_BASE + num)->reg0 = 1;
	((struct hapara_mutex_pair *)HAPARA_MUTEX_MANA_BASE + num)->reg0 = 0;
}

void release_mutex(int num) {
	((struct hapara_mutex_pair *)HAPARA_MUTEX_MANA_BASE + num)->reg2 = 1;
	while (((struct hapara_mutex_pair *)HAPARA_MUTEX_MANA_BASE + num)->reg2 != 0);
}

int main() {
	//initial state;
	//finish signal is 0 at the beginning to be fixed;

	hapara_gen_t *hapara_gen = (hapara_gen_t *)HAPARA_GENERATOR_BASE;
	thread_control_t *thread_control =
			(thread_control_t *)CONTROL_BASE_ADDR;
	elf_info_t *elf_info_p =
			(elf_info_t *)(CONTROL_BASE_ADDR + sizeof(thread_control_t));
	struct hapara_thread_struct *hapara_thread = (struct hapara_thread_struct *)HAPARA_HTDT_BASE;

	hapara_thread->valid = 1;
	hapara_thread->priority = 10;
	hapara_thread->type = 1;
	hapara_thread->next = 0;
	hapara_thread->tid = 100;
	hapara_thread->group_id.id0 = 0;
	hapara_thread->group_id.id1 = 0;
//	hapara_thread->elf_info.main_addr = 0xC0000568;
	hapara_thread->elf_info.main_addr = 0xC0008000;
	hapara_thread->elf_info.stack_addr = 0xC0009408;
	hapara_thread->elf_info.thread_size = 0xB80;

//	int * ddr = (int *)0x60002000;
//	int i = 0;
//	for (i = 0; i < 16; i++) {
////		putnum(0xDEADBEEF);
//		ddr[i] = 0;
//	}

	int numOfGroup 	= 4;
	int groupLength = 4;
	int gid0 = 0;
	int gid1 = 0;

	int isDMA = 0;
#ifdef DMAAPI
	int status;
	XAxiCdma CDMA;
	XAxiCdma_Config *CDMAPtr;
	CDMAPtr = XAxiCdma_LookupConfig(0);
	if (!CDMAPtr)
		return XST_FAILURE;
	status = XAxiCdma_CfgInitialize(&CDMA,CDMAPtr,CDMAPtr->BaseAddress);
	if (status != XST_SUCCESS)
		return XST_FAILURE;
	XAxiCdma_IntrDisable(&CDMA, XAXICDMA_XR_IRQ_ALL_MASK);
#endif

	while (hapara_gen->isFinished != 0) ;
	while (1) {
		lock_mutex(0);
		//begin to access HTDT
		if (hapara_thread->type == 1) {
			elf_info_p->main_addr 	= hapara_thread->elf_info.main_addr;
			elf_info_p->stack_addr 	= hapara_thread->elf_info.stack_addr;
			elf_info_p->thread_size = hapara_thread->elf_info.thread_size;
			gid0 = hapara_thread->group_id.id0;
			gid1 = hapara_thread->group_id.id1;
			if (gid1 >= numOfGroup) {
				hapara_thread->type = 0;
				isDMA = 0;
				release_mutex(0);
				break;
			}
			hapara_thread->group_id.id1++;
		} else {
			release_mutex(0);
			continue;
		}
		release_mutex(0);
		//FIXME here, this can only work for single HTDT entry;

		//begin to initialize id_generator
		//gid0 = gid0 * groupLength;
		gid1 = gid1 * groupLength;
		hapara_gen->org 		= (gid0 << 16) | gid1;
		hapara_gen->numOfSlvs 	= 4;
		hapara_gen->len			= 0x00010004;
		gid0 = 0;
		gid1 = 0;
		if (isDMA == 0) {
			isDMA = 1;
			//begin to DMA ELF files from DDR to local memory
#ifdef DMAAPI
			status = XAxiCdma_SimpleTransfer(&CDMA,
											 HAPARA_MIG_CDMA_BASE + 3072 * 4, //src
											 HAPARA_INST_CDMA_BASE,			  //dst
											 elf_info_p->thread_size,
											 NULL,
											 NULL);
			if (status != XST_SUCCESS)
				return 1;
			while (XAxiCdma_IsBusy(&CDMA)); //wait for transfer is done
#else
			Xil_Out32((HAPARA_DMA_BASE + 0x18), 0x60003000);				//source
			Xil_Out32((HAPARA_DMA_BASE + 0x20), HAPARA_INST_CDMA_BASE);		//destination
			Xil_Out32((HAPARA_DMA_BASE + 0x28), elf_info_p->thread_size);	//bytes to transfer
			while((Xil_In32((HAPARA_DMA_BASE + 0x4)) & 0x00000002) == 0x00000000);	//not idle: bit == 0
#endif
		}
		//begin to start slaves to run
		thread_control->trigger0 = 1;
		thread_control->trigger1 = 1;
		thread_control->trigger2 = 1;
		thread_control->trigger3 = 1;
		while (hapara_gen->isFinished != 1);
	}

	return 0;
}
*/

#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif

#include <stdint.h>
//#include <xaxicdma.h>
#include <xstatus.h>
#include <xil_io.h>

#include "../../generic/include/thread_struct.h"
#include "../../generic/CL/cl.h"
#include "../../generic/include/base_addr.h"
#include "../../generic/include/register.h"
#include "../../generic/include/hw_config.h"



int main() {
	struct hapara_thread_struct *hapara_thread;
	struct hapara_thread_struct *hapara_thread_base =
			(struct hapara_thread_struct *)SCHEDULER_HTDT_BASE;
	struct hapara_gen_struct *hapara_gen =
			(struct hapara_gen_struct *)SCHEDULER_ID_GENERATOR_BASE;
	struct elf_info_struct *elf_info =
			(struct elf_info_struct *)SCHEDULER_LOCAL_MEM_BASE;
	int *trigger = (int *)SCHE_SLAVE_TRIGGER_BASE;
	while (1) {
		int max_priority = -1;
		int htdt_off = -1;
		struct hapara_id_pair cur_group_id = {
				.id0 = 0,
				.id1 = 0,
		};
		struct hapara_id_pair group_num = {
				.id0 = 0,
				.id1 = 0,
		};
		lock_mutex(REG_MUTEX);
		for_each_valid(hapara_thread, SCHEDULER_HTDT_BASE)
		{
			if (hapara_thread->isValid == 1 &&
				hapara_thread->priority > max_priority) {
				htdt_off = hapara_thread - hapara_thread_base;
			}
		}
		if (max_priority == -1) {
			release_mutex(REG_MUTEX);
			continue;
		}
		cur_group_id.id0 = (hapara_thread + htdt_off)->cur_group_id.id0;
		cur_group_id.id1 = (hapara_thread + htdt_off)->cur_group_id.id1;
		group_num.id0 = (hapara_thread + htdt_off)->group_num.id0;
		group_num.id1 = (hapara_thread + htdt_off)->group_num.id1;
		if (cur_group_id.id0 >= group_num.id0) {
			(hapara_thread + htdt_off)->isValid = 0;
			release_mutex(REG_MUTEX);
			continue;
		}
		cur_group_id.id0 += (cur_group_id.id0 + 1) / group_num.id0;
		cur_group_id.id1 = (cur_group_id.id0 + 1) % group_num.id0;
		(hapara_thread + htdt_off)->cur_group_id.id0 = cur_group_id.id0;
	    (hapara_thread + htdt_off)->cur_group_id.id1 = cur_group_id.id1;
		release_mutex(REG_MUTEX);
		struct hapara_id_pair group_size = {
				.id0 = (hapara_thread + htdt_off)->group_size.id0,
				.id1 = (hapara_thread + htdt_off)->group_size.id1,
		};
		hapara_gen->org = ((cur_group_id.id0 * group_size.id0) << 16) | (cur_group_id.id1 * group_size.id1);
		hapara_gen->numOfSlvs = NUM_SLAVES;
		hapara_gen->len = (group_size.id0 << 16) | (group_size.id1);
		if ((hapara_thread + htdt_off)->elf_info.elf_magic != elf_info->elf_magic) {
			elf_info->main_addr = (hapara_thread + htdt_off)->elf_info.main_addr;
			elf_info->stack_addr = (hapara_thread + htdt_off)->elf_info.stack_addr;
			Xil_Out32((SCHEDULER_DMA_BASE + 0x18), (hapara_thread + htdt_off)->elf_info.ddr_addr);	//source
			Xil_Out32((SCHEDULER_DMA_BASE + 0x20), DMA_INST_MEM_BASE);								//destination
			Xil_Out32((SCHEDULER_DMA_BASE + 0x28), (hapara_thread + htdt_off)->elf_info.DMA_size);	//bytes to transfer
			while((Xil_In32((SCHEDULER_DMA_BASE + 0x4)) & 0x00000002) == 0x00000000);				//not idle: bit == 0
		}
		int i;
		for (i = 0; i < NUM_SLAVES; i++) {
			trigger[i] = 1;
		}
		while (hapara_gen->isFinished != 1);
	}
	return 0;
}
