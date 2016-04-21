/*
 * scheduler.c
 *
 *  Created on: Apr 15, 2016
 *      Author: hding
 */


#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif

#include <stdint.h>
//#include <xaxicdma.h>
#include <xstatus.h>
#include <xil_io.h>
#include <string.h>

#include "../../generic/include/thread_struct.h"
#include "../../generic/CL/cl.h"
#include "../../generic/include/base_addr.h"
#include "../../generic/include/register.h"
#include "../../generic/include/hw_config.h"



int main() {
	struct hapara_thread_struct *hapara_thread;
	struct hapara_thread_struct *hapara_thread_curr;
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
				max_priority = hapara_thread->priority;
			}
		}
		if (max_priority == -1) {
			release_mutex(REG_MUTEX);
			continue;
		}
		hapara_thread_curr = hapara_thread_base + htdt_off;
		group_num.id0 = hapara_thread_curr->group_num.id0;
		cur_group_id.id0 = hapara_thread_curr->cur_group_id.id0;
		if (cur_group_id.id0 >= group_num.id0) {
			hapara_thread_curr->isValid = 0;
			release_mutex(REG_MUTEX);
			continue;
		}
		cur_group_id.id1 = hapara_thread_curr->cur_group_id.id1;
		group_num.id1 = hapara_thread_curr->group_num.id1;

		hapara_thread_curr->cur_group_id.id0 += (cur_group_id.id1 + 1) / group_num.id1;
		hapara_thread_curr->cur_group_id.id1 = (cur_group_id.id1 + 1) % group_num.id1;
		release_mutex(REG_MUTEX);
		struct hapara_id_pair group_size = {
				.id0 = hapara_thread_curr->group_size.id0,
				.id1 = hapara_thread_curr->group_size.id1,
		};
		hapara_gen->org = ((cur_group_id.id0 * group_size.id0) << 16) | (cur_group_id.id1 * group_size.id1);
		hapara_gen->numOfSlvs = NUM_SLAVES;
//		hapara_gen->numOfSlvs = 1;
		hapara_gen->len = (group_size.id0 << 16) | (group_size.id1);
		if (hapara_thread_curr->elf_info.elf_magic != elf_info->elf_magic) {
			elf_info->elf_magic = hapara_thread_curr->elf_info.elf_magic;
			elf_info->main_addr = hapara_thread_curr->elf_info.main_addr;
			elf_info->stack_addr = hapara_thread_curr->elf_info.stack_addr;
			memcpy((char *)SCHE_SLAVE_ARGV_BASE, (char *)(&(hapara_thread_curr->argv[0])), ARGC);
			Xil_Out32((SCHEDULER_DMA_BASE + 0x18), hapara_thread_curr->elf_info.ddr_addr);	//source
			Xil_Out32((SCHEDULER_DMA_BASE + 0x20), DMA_INST_MEM_BASE);								//destination
			Xil_Out32((SCHEDULER_DMA_BASE + 0x28), hapara_thread_curr->elf_info.DMA_size);	//bytes to transfer
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