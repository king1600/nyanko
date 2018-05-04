
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "atomic.h"
#include "rpmalloc.h"

int main(int argc, char* argv[]) {
    rpmalloc_initialize();
    rpmalloc_thread_initialize();
    nk_queue_t queue;
    nk_queue_init(&queue);

    nk_queue_push(&queue, (void*)5);
    nk_queue_push(&queue, (void*)10);
    nk_queue_push(&queue, (void*)15);

    void* value;
    while ((value = nk_queue_pop(&queue)) != NULL) {
        printf("%p\n", value);
    }

    nk_queue_free(&queue);
    rpmalloc_thread_finalize();
    rpmalloc_finalize();
}