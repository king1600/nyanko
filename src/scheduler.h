#ifndef _NYANKO_SCHEDULER_H
#define _NYANKO_SCHEDULER_H

#include "nk.h"
#include "atomic.h"

#if defined(NK_WINDOWS)
    #include <windows.h>
    #define nk_yield SwitchToThread
#elif defined(NK_LINUX)
    #include <sched.h>
    #define nk_yield sched_yield
#endif

typedef struct {
    nk_queue_t queue;
} nk_sched_t;

#endif // _NYANKO_SCHEDULER_H