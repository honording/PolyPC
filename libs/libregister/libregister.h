#ifndef _LIBREGISTER_H_
#define _LIBREGISTER_H_

#include "../../../generic/include/thread_struct.h"
#include "../../../generic/include/register.h"

#define FILEPATH    "/dev/hapara_reg"

void libregister_test(void);
void print_struct(struct hapara_thread_struct *sp);

int read_struct(struct hapara_thread_struct *thread_info, unsigned int offset);
//do not implenment minus offset.

void set_struct(struct hapara_thread_struct *thread_info,
                unsigned int valid,
                unsigned int priority,
                unsigned int type,
                unsigned int next,
                unsigned int tid,
                unsigned int id0,
                unsigned int id1);


int reg_add(struct hapara_thread_struct *thread_info);
int reg_del(unsigned int off, unsigned int target);
void reg_clr();

#endif
