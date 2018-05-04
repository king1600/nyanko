
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "actor.h"
#include "rpmalloc.h"

int main(int argc, char* argv[]) {
    rpmalloc_initialize();
    rpmalloc_thread_initialize();
    
    printf("%lu\n", sizeof(nk_actor_t));

    rpmalloc_thread_finalize();
    rpmalloc_finalize();
}