#ifndef _LIBREGISTER_H_
#define _LIBREGISTER_H_

#include "../../../generic/include/thread_struct.h"
#include "../../../generic/include/register.h"

#define FILEPATH    "/dev/hapara_reg"

void libregister_test(void);
void print_struct(struct hapara_thread_struct *sp);

int reg_add(struct hapara_thread_struct *thread_info);
int reg_del(unsigned int off, unsigned int target);

#endif