#ifndef _HAPARA_TRACE_H_
#define _HAPARA_TRACE_H_

#define HTRACE_MAGIG                't'
#define HTRACE_MAX                  11
#define HTRACE_TRACE_CLEAR          _IO(HTRACE_MAGIG, 0)
#define HTRACE_TRACE_ALLOC          _IOW(HTRACE_MAGIG, 1, unsigned int *)    // Num of groups
#define HTRACE_TRACE_GETTOTALNUM    _IO(HTRACE_MAGIG, 2)
#define HTRACE_TRACE_GETTOTALSIZE   _IO(HTRACE_MAGIG, 3)
#define HTRACE_TRACE_GETTOTALCON    _IOR(HTRACE_MAGIG, 4, unsigned int *)     // User address
#define HTRACE_TRACE_GETEACHSIZE    _IOW(HTRACE_MAGIG, 5, unsigned int *)     // index
#define HTRACE_TRACE_GETEACHOFF     _IOW(HTRACE_MAGIG, 6, unsigned int *)     // index

#define HTRACE_TIMER_RESET          _IO(HTRACE_MAGIG, 7)
#define HTRACE_TIMER_START          _IO(HTRACE_MAGIG, 8)
#define HTRACE_TIMER_STOP           _IO(HTRACE_MAGIG, 9)
#define HTRACE_TIMER_GETTIME        _IOR(HTRACE_MAGIG, 10, unsigned int *)
        
#endif