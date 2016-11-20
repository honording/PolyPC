#ifndef _MODTRACE_H_
#define _MODTRACE_H_

#include "../../../generic/include/debug.h"
#include "../../../generic/include/base_addr.h"
#include "../../../generic/include/hapara_trace.h"
#include "../../../generic/include/hapara_trace_base.h"

struct hapara_trace {
    struct cdev cdev;
    void *mmio_timer;
    void *mmio_trace;
};

#define MODULE_NAME     "hapara_trace"

#define TIMER_BASE      ARM_TIMER_BASE
#define TIMER_SPAN      0x10000          //64KByte

#define TRACE_BASE      ARM_TRACE_BASE
#define TRACE_SPAN      0x10000          //64KByte

#define MAX_TRACE_SLOT  4096


#endif
