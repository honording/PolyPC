/*
 * g0s0.c
 *
 *  Created on: Apr 8, 2016
 *      Author: hding
 */

#ifndef __USER_PROGRAMS__
#define __USER_PROGRAMS__
#endif

#include <stdint.h>
#include "../../generic/CL/cl.h"
#include "../../generic/include/thread_struct.h"
#include "../../generic/include/base_addr.h"

//typedef void * (*start_thread_t)();

typedef struct {
	// stack pointer
	uint32_t	r1;
	uint32_t	r2;
	// volatile resiters
	uint32_t	r3;
	uint32_t	r4;
	uint32_t	r5;
	uint32_t	r6;
	uint32_t	r7;
	uint32_t	r8;
	uint32_t	r9;
	uint32_t	r10;
	uint32_t	r11;
	uint32_t	r12;
	// dedicated registers
	uint32_t	r13;
	uint32_t	r14;
	uint32_t	r15;
	uint32_t	r16;
	uint32_t	r17;
	uint32_t	r18;
	// non-volatile registers
	uint32_t	r19;
	uint32_t	r20;
	uint32_t	r21;
	uint32_t	r22;
	uint32_t	r23;
	uint32_t	r24;
	uint32_t	r25;
	uint32_t	r26;
	uint32_t	r27;
	uint32_t	r28;
	uint32_t	r29;
	uint32_t	r30;
	uint32_t	r31;
	// special purpose registers
	uint32_t	msr;
	uint32_t	ear;
	uint32_t	esr;
	uint32_t	fsr;

} cpu_context_t;

cpu_context_t __attribute__((section(".hapara.regs"))) cpu_context =
		{
				.r1		= 0,	//0
				.r2 	= 0,	//4
				.r3 	= 0,	//8
				.r4 	= 0,
				.r5 	= 0,
				.r6 	= 0,
				.r7 	= 0,
				.r8 	= 0,
				.r9 	= 0,
				.r10 	= 0,
				.r11 	= 0,
				.r12 	= 0,
				.r13	= 0,
				.r14	= 0,
				.r15	= 0,
				.r16	= 0,
				.r17	= 0,
				.r18	= 0,
				.r19	= 0,
				.r20	= 0,
				.r21	= 0,
				.r22	= 0,
				.r23	= 0,
				.r24	= 0,
				.r25	= 0,
				.r26	= 0,
				.r27	= 0,
				.r28	= 0,
				.r29	= 0,
				.r30	= 0,
				.r31	= 0,	//120
				.msr	= 0,	//124
				.ear	= 0,	//128
				.esr	= 0,	//132
				.fsr	= 0		//136

		};

elf_info_t __attribute__((section(".hapara.elfs"))) elf_info = {
		.main_addr 		= 0,
		.stack_addr 	= 0,
		.thread_size 	= 0
};

int main()
{
	volatile int *trigger = (int *)SCHE_SLAVE_TRIGGER_BASE;
	int mb_num = 0;
	getpvr(1, mb_num);
	struct elf_info_struct *elf_info_r =
			(struct elf_info_struct *)SLAVE_LOCAL_MEM_BASE;
	while (1) {
		if (trigger[mb_num] == 1) {
			trigger[mb_num] = 0;
			elf_info.main_addr		= elf_info_r->main_addr;
			elf_info.stack_addr		= elf_info_r->stack_addr;
//			elf_info.thread_size	= tsp->elf_info.thread_size;
			/* Store CPU Context ------Begin-----*/
			asm("swi r1, r0, .hapara.regs");
			asm("addik r1, r0, .hapara.regs");
			asm("swi r2, r1, 4");
			asm("swi r3, r1, 8");
			asm("swi r4, r1, 12");
			asm("swi r5, r1, 16");
			asm("swi r6, r1, 20");
			asm("swi r7, r1, 24");
			asm("swi r8, r1, 28");
			asm("swi r9, r1, 32");
			asm("swi r10, r1, 36");
			asm("swi r11, r1, 40");
			asm("swi r12, r1, 44");
			asm("swi r13, r1, 48");
			asm("swi r14, r1, 52");
			asm("swi r15, r1, 56");
			asm("swi r16, r1, 60");
			asm("swi r17, r1, 64");
			asm("swi r18, r1, 68");
			asm("swi r19, r1, 72");
			asm("swi r20, r1, 76");
			asm("swi r21, r1, 80");
			asm("swi r22, r1, 84");
			asm("swi r23, r1, 88");
			asm("swi r24, r1, 92");
			asm("swi r25, r1, 96");
			asm("swi r26, r1, 100");
			asm("swi r27, r1, 104");
			asm("swi r28, r1, 108");
			asm("swi r29, r1, 112");
			asm("swi r30, r1, 116");
			asm("swi r31, r1, 120");
			asm("mfs r11, rmsr");
			asm("swi r11, r1, 124");
			asm("mfs r11, rear");
			asm("swi r11, r1, 128");
			asm("mfs r11, resr");
			asm("swi r11, r1, 132");
			asm("mfs r11, rfsr");
			asm("swi r11, r1, 136");
			asm("lwi r11, r1, 40");
			asm("lwi r1, r0, .hapara.regs");
			/* Store CPU Context -----End-----*/

			asm("addik r11, r0, .hapara.elfs");
			asm("lwi r3, r11, 0");
			asm("lwi r1, r11, 4");
			asm("brald r15, r3");
			asm("nop");

			/* Restore CPU Context -----Begin-----*/
			asm("addik r1, r0, .hapara.regs");
			asm("lwi r2, r1, 4");
			asm("lwi r3, r1, 8");
			asm("lwi r4, r1, 12");
			asm("lwi r5, r1, 16");
			asm("lwi r6, r1, 20");
			asm("lwi r7, r1, 24");
			asm("lwi r8, r1, 28");
			asm("lwi r9, r1, 32");
			asm("lwi r10, r1, 36");
			asm("lwi r12, r1, 44");
			asm("lwi r13, r1, 48");
			asm("lwi r14, r1, 52");
			asm("lwi r15, r1, 56");
			asm("lwi r16, r1, 60");
			asm("lwi r17, r1, 64");
			asm("lwi r18, r1, 68");
			asm("lwi r19, r1, 72");
			asm("lwi r20, r1, 76");
			asm("lwi r21, r1, 80");
			asm("lwi r22, r1, 84");
			asm("lwi r23, r1, 88");
			asm("lwi r24, r1, 92");
			asm("lwi r25, r1, 96");
			asm("lwi r26, r1, 100");
			asm("lwi r27, r1, 104");
			asm("lwi r28, r1, 108");
			asm("lwi r29, r1, 112");
			asm("lwi r30, r1, 116");
			asm("lwi r31, r1, 120");
			asm("lwi r11, r1, 124");
			asm("mts rmsr, r11");
			asm("lwi r11, r1, 128");
			asm("mts rear, r11");
			asm("lwi r11, r1, 132");
			asm("mts resr, r11");
			asm("lwi r11, r1, 136");
			asm("mts rfsr, r11");
			asm("lwi r11, r1, 40");
			asm("lwi r1, r0, .hapara.regs");
			/* Restore CPU Context -----End-----*/
		}
	}
	return 0;
}
