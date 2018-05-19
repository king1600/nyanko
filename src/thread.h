#ifndef _NK_THREAD_H
#define _NK_THREAD_H

#include "nk.h"
#include "alloc.h"
typedef void* (*nk_thread_func) (void* arg);
#define __nk_inline(type) static NK_INLINE type

#ifdef NK_WINDOWS
    #include <windows.h>
    typedef HANDLE nk_thread_t;
    typedef CRITICAL_SECTION nk_mutex_t;
    typedef CONDITION_VARIABLE nk_cond_t;
    #define nk_thread_yield SwitchToThread
    #define NK_THREAD_FUNC(name, arg) DWORD WINAPI name(PVOID arg)

    // condition variables
    __nk_inline(void) nk_cond_free(nk_cond_t* cond) {}
    __nk_inline(void) nk_cond_init(nk_cond_t* cond) { InitializeConditionVariable(cond); }
    __nk_inline(void) nk_cond_signal(nk_cond_t* cond) { WakeConditionVariable(cond); }
    __nk_inline(void) nk_cond_wait(nk_cond_t* cond, nk_mutex_t* mutex) { SleepConditionVariableCS(cond, mutex, INFINITE); }

    // mutexes
    __nk_inline(void) nk_mutex_free(nk_mutex_t* mutex) {}
    __nk_inline(void) nk_mutex_init(nk_mutex_t* mutex) { InitializeCriticalSection(mutex); }
    __nk_inline(void) nk_mutex_lock(nk_mutex_t* mutex) { EnterCriticalSection(mutex); }
    __nk_inline(void) nk_mutex_unlock(nk_mutex_t* mutex) { LeaveCriticalSection(mutex); }

    // threads
    __nk_inline(void) nk_thread_exit() { ExitThread(NULL); }
    __nk_inline(void) nk_thread_join(nk_thread_t id) { WaitForSingleObject(id, INFINITE); CloseHandle(id); }
    __nk_inline(nk_thread_t) nk_thread_init(nk_thread_func func, void* arg) {
        DWORD id; return CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)func, (LPVOID)arg, 0, &id);
    }

    // misc
    __nk_inline(int) nk_cores() { SYSTEM_INFO info; GetSystemInfo(&info); return (int) info.dwNumberOfProcessors; }

#else
    #include <sched.h>
    #include <pthread.h>
    #include <sys/sysinfo.h>
    typedef pthread_t nk_thread_t;
    typedef pthread_cond_t nk_cond_t;
    typedef pthread_mutex_t nk_mutex_t;
    #define nk_thread_yield sched_yield
    #define NK_THREAD_FUNC(name, arg) void* name(void* arg)

    // condition variables
    __nk_inline(void) nk_cond_free(nk_cond_t* cond) { pthread_cond_destroy(cond); }
    __nk_inline(void) nk_cond_init(nk_cond_t* cond) { pthread_cond_init(cond, NULL); }
    __nk_inline(void) nk_cond_signal(nk_cond_t* cond) { pthread_cond_signal(cond); }
    __nk_inline(void) nk_cond_wait(nk_cond_t* cond, nk_mutex_t* mutex) { pthread_cond_wait(cond, mutex); }

    // mutexes
    __nk_inline(void) nk_mutex_free(nk_mutex_t* mutex) { pthread_mutex_destroy(mutex); }
    __nk_inline(void) nk_mutex_init(nk_mutex_t* mutex) { pthread_mutex_init(mutex, NULL); }
    __nk_inline(void) nk_mutex_lock(nk_mutex_t* mutex) { pthread_mutex_lock(mutex); }
    __nk_inline(void) nk_mutex_unlock(nk_mutex_t* mutex) { pthread_mutex_unlock(mutex); }

    // threads
    __nk_inline(void) nk_thread_exit() { pthread_exit(NULL); }
    __nk_inline(void) nk_thread_join(nk_thread_t id) { pthread_join(id, NULL); }
    __nk_inline(nk_thread_t) nk_thread_init(nk_thread_func func, void* arg) {
        nk_thread_t id; pthread_create(&id, NULL, func, arg); return id;
    }

    // misc
    __nk_inline(int) nk_cores() { return get_nprocs(); }
#endif

#endif // _NK_THREAD_H