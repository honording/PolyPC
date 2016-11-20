#ifndef _LIBTRACE_H_
#define _LIBTRACE_H_

#include "../../../generic/include/hapara_trace.h"
#include "../../../generic/include/hapara_trace_base.h"
#include "../../../generic/include/thread_struct.h"

#define FILEPATH    "/dev/hapara_trace"

void trace_clr();
int trace_alloc(struct hapara_id_pair id);
int trace_alloc_single(unsigned int num);
int trace_gettotalnum();
int trace_gettotalsize();
int trace_gettotalcon(unsigned int *buf);
int trace_geteachsize(unsigned int index);
int trace_geteachoff(unsigned int index);
void hapara_timer_reset();
void hapara_timer_start();
void hapara_timer_stop();
void hapara_timer_gettime(unsigned int *time);

#endif