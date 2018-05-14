#include "alloc.h"
#include "scheduler.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
    NK_MEM_INIT();
    NK_MEM_THREAD_INIT();

    nk_mpmc_queue_t queue;
    nk_mpmc_queue_init(&queue);

    nk_mpmc_queue_push(&queue, (void*)1);
    nk_mpmc_queue_push(&queue, (void*)2);
    nk_mpmc_queue_push(&queue, (void*)3);

    void* data;
    while ((data = nk_mpmc_queue_pop(&queue)) != NULL)
        printf("%p\n", data);

    nk_mpmc_queue_free(&queue);

    NK_MEM_THREAD_FREE();
    NK_MEM_FREE();
    
    return 0;
}