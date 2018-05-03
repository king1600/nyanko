
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rpmalloc.h"

int main(int argc, char* argv[]) {
    rpmalloc_initialize();
    rpmalloc_thread_initialize();

    char* end = argv[1] + strlen(argv[1]);
    void* p = rpmalloc(strtoul(argv[1], &end, 10));
    printf("%p %lu\n", p, rpmalloc_usable_size(p));
    rpfree(p);

    rpmalloc_thread_finalize();
    rpmalloc_finalize();
}