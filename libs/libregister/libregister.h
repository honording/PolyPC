#ifndef _LIBREGISTER_H_
#define _LIBREGISTER_H_

#include "../../include/thread_struct.h"

#define FILEPATH    "/dev/hapara_reg"

void libregister_test(void);
void print_struct(struct hapara_thread_struct *sp);

#endif