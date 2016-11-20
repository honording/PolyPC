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
#include "fsl.h"

#include "../../generic/include/thread_struct.h"
#include "../../generic/CL/cl.h"
#include "../../generic/include/base_addr.h"
#include "../../generic/include/register.h"
#include "../../generic/include/prc_addr_mapping.h"
#include "../../generic/include/hapara_trace_base.h"
//#include "../../generic/include/hw_config.h"

#define STATUS  			(VS_STATUS + SCHEDULER_ICAP)
#define CONTROL 			(VS_CONTROL + SCHEDULER_ICAP)
#define SW_TRIGGER 			(VS_SW_TRIGGER + SCHEDULER_ICAP)
#define TRIGGER0 			(VS_TRIGGER0 + SCHEDULER_ICAP)
#define TRIGGER1 			(VS_TRIGGER1 + SCHEDULER_ICAP)
#define RM_BS_INDEx0 		(VS_RM_BS_INDEx0 + SCHEDULER_ICAP)
#define RM_CONTROL0 		(VS_RM_CONTROL0 + SCHEDULER_ICAP)
#define RM_BS_INDEx1 		(VS_RM_BS_INDEx1 + SCHEDULER_ICAP)
#define RM_CONTROL1 		(VS_RM_CONTROL1 + SCHEDULER_ICAP)
#define BS_ID0 				(VS_BS_ID0 + SCHEDULER_ICAP)
#define BS_ADDRESS0 		(VS_BS_ADDRESS0 + SCHEDULER_ICAP)
#define BS_SIZE0 			(VS_BS_SIZE0 + SCHEDULER_ICAP)
#define BS_ID1 				(VS_BS_ID1 + SCHEDULER_ICAP)
#define BS_ADDRESS1 		(VS_BS_ADDRESS1 + SCHEDULER_ICAP)
#define BS_SIZE1 			(VS_BS_SIZE1 + SCHEDULER_ICAP)


int main() {
	unsigned int pvr1 = 0;
	unsigned int pvr2 = 0;
	getpvr(0, pvr1);
	getpvr(1, pvr2);
	unsigned int num_of_slave	= pvr2 >> 16;
	unsigned int num_of_mb_slave = pvr2 & 0x0000FFFF;
	unsigned int num_of_hw_slave = num_of_slave - num_of_mb_slave;
	unsigned int group_index = pvr1 & 0x000000FF;
	unsigned int pr_offset = group_index * num_of_slave;
	int i;


	struct hapara_thread_struct *hapara_thread;
	struct hapara_thread_struct *hapara_thread_curr;
	struct hapara_trace_struct *trace_curr;
	struct hapara_thread_struct *hapara_thread_base =
			(struct hapara_thread_struct *)SCHEDULER_HTDT_BASE;
	struct hapara_gen_struct *hapara_gen =
			(struct hapara_gen_struct *)SCHEDULER_ID_GENERATOR_BASE;
	struct elf_info_struct *elf_info =
			(struct elf_info_struct *)SCHEDULER_LOCAL_MEM_BASE;
	struct hapara_timer_struct *timer =
			(struct hapara_timer_struct *)SCHEDULER_TIMER_BASE;
	struct hapara_trace_struct *trace = 
			(struct hapara_trace_struct *)SCHEDULER_TRACE_BASE;
	int *trigger = (int *)SCHE_SLAVE_TRIGGER_BASE;
	elf_info->elf_magic = 0;
	while (1) {
		volatile int max_priority = -1;
		volatile int htdt_off = -1;
		volatile int max_priority_magic = -1;
		volatile int htdt_off_magic = -1;
		struct hapara_id_pair cur_group_id = {
				.id0 = 0,
				.id1 = 0,
		};
		struct hapara_id_pair group_num = {
				.id0 = 0,
				.id1 = 0,
		};
		lock_mutex(REG_MUTEX, group_index + 1);
		for_each_valid(hapara_thread, SCHEDULER_HTDT_BASE)
		{
			if (hapara_thread->isValid == 1 &&
				hapara_thread->priority > max_priority) {
				htdt_off = hapara_thread - hapara_thread_base;
				max_priority = hapara_thread->priority;
				if (elf_info->elf_magic == hapara_thread->elf_info.elf_magic) {
					max_priority_magic = max_priority;
					htdt_off_magic = htdt_off;
				}
			}
		}
		if (max_priority == -1) {
			release_mutex(REG_MUTEX, group_index + 1);
			continue;
		}
		if (max_priority_magic == max_priority) {
			hapara_thread_curr = hapara_thread_base + htdt_off_magic;
		} else {
			hapara_thread_curr = hapara_thread_base + htdt_off;
		}
		group_num.id0 = hapara_thread_curr->group_num.id0;
		cur_group_id.id0 = hapara_thread_curr->cur_group_id.id0;
		if (cur_group_id.id0 >= group_num.id0) {
			hapara_thread_curr->isValid = 0;
			release_mutex(REG_MUTEX, group_index + 1);
			continue;
		}
		cur_group_id.id1 = hapara_thread_curr->cur_group_id.id1;
		group_num.id1 = hapara_thread_curr->group_num.id1;

		hapara_thread_curr->cur_group_id.id0 += (cur_group_id.id1 + 1) / group_num.id1;
		hapara_thread_curr->cur_group_id.id1 = (cur_group_id.id1 + 1) % group_num.id1;
		release_mutex(REG_MUTEX, group_index + 1);
		struct hapara_id_pair group_size = {
				.id0 = hapara_thread_curr->group_size.id0,
				.id1 = hapara_thread_curr->group_size.id1,
		};
		// Get timer information
		unsigned int trace_off = hapara_thread_curr->trace_ram_off + 
								 (cur_group_id.id0 * group_num.id1 + cur_group_id.id1);
		trace_curr = trace + trace_off;
		trace_curr->group_index = group_index;
		trace_curr->before_loading = timer->tcr;

		hapara_gen->org = ((cur_group_id.id0 * group_size.id0) << 16) | (cur_group_id.id1 * group_size.id1);
		hapara_gen->numOfSlvs = num_of_slave;
		hapara_gen->len = (group_size.id0 << 16) | (group_size.id1);
		memcpy((unsigned int *)SCHE_SLAVE_ARGV_BASE, (unsigned int *)(&(hapara_thread_curr->argv[0])), ARGC * sizeof(unsigned int));
		if (hapara_thread_curr->elf_info.elf_magic != elf_info->elf_magic) {
			elf_info->elf_magic = hapara_thread_curr->elf_info.elf_magic;
			if (num_of_mb_slave != 0) {
				elf_info->main_addr = hapara_thread_curr->elf_info.main_addr;
				elf_info->stack_addr = hapara_thread_curr->elf_info.stack_addr;
				Xil_Out32((SCHEDULER_DMA_BASE + 0x18), hapara_thread_curr->elf_info.ddr_addr);	//source
				Xil_Out32((SCHEDULER_DMA_BASE + 0x20), DMA_INST_MEM_BASE);						//destination
				Xil_Out32((SCHEDULER_DMA_BASE + 0x28), hapara_thread_curr->elf_info.DMA_size);	//bytes to transfer
				// while((Xil_In32((SCHEDULER_DMA_BASE + 0x4)) & 0x00000002) == 0x00000000);		//not idle: bit == 0
			}
			int pr_size = hapara_thread_curr->pr_info.each_size;
			if (pr_size != -1) {
				for (i = 0; i < num_of_hw_slave; i++) {
					int Status;
					unsigned int VSM_OFFSET = (pr_offset + i) << 7;
					//  xil_printf("Putting the PRC core's Math RP in Shutdown mode\n\r");
					Xil_Out32(VSM_OFFSET | CONTROL, 0);
					//  xil_printf("Waiting for the shutdown to occur\r\n");
					while(!(Xil_In32(VSM_OFFSET | STATUS) & 0x80));
					//  xil_printf("Initializing RM bitstream address and size registers for Current PR RM\r\n");
					unsigned int curr_pr_offset = hapara_thread_curr->pr_info.ddr_addr + (pr_offset + i) * pr_size;
					Xil_Out32(VSM_OFFSET | BS_ADDRESS0, curr_pr_offset);
					Xil_Out32(VSM_OFFSET | BS_SIZE0, pr_size);
					//  xil_print("Initializing RM trigger ID registers for Current RM\r\n");
					Xil_Out32(VSM_OFFSET | TRIGGER0, 0);
					//  xil_print("Initializing RM index and control registers for Current RM.\r\n");
					Xil_Out32(VSM_OFFSET | RM_BS_INDEx0, 0);
					//  Reset Active low mode for 8 clock cycles.
					Xil_Out32(VSM_OFFSET | RM_CONTROL0, 0xF0);
					//  xil_print("Putting the PRC core's Math RP in Restart with Status mode\n\r");
					Xil_Out32(VSM_OFFSET | CONTROL, 2);
					//  xil_printf("Generating software trigger for VADD reconfiguration\r\n");
					Status = Xil_In32(VSM_OFFSET | SW_TRIGGER);
					if(!(Status & 0x8000)) {
					//  xil_printf("Starting VADD Reconfiguration\n\r");
						Xil_Out32(VSM_OFFSET | SW_TRIGGER, 0);
					}
					while((Xil_In32(VSM_OFFSET | STATUS) & 0x07) != 7);
				}				
			}


			// Wait for ELF and PR BIN files to finish transferring
			if (num_of_mb_slave != 0) {
				while((Xil_In32((SCHEDULER_DMA_BASE + 0x4)) & 0x00000002) == 0x00000000);		//not idle: bit == 0
			}
		}
		trace_curr->before_triggle = timer->tcr;
		for (i = 0; i < num_of_slave; i++) {
			trigger[i] = 1;
		}
		// while (hapara_gen->isFinished != 1);
		volatile unsigned int rel = 0;
		putfslx(1, 0, FSL_DEFAULT);
		getfslx(rel, 0, FSL_DEFAULT);
		trace_curr->after_finish = timer->tcr;
	}
	return 0;
}

