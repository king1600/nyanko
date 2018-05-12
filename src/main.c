#include "alloc.h"
#include "gc.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
    NK_MEM_INIT();
    NK_MEM_THREAD_INIT();

    NK_MEM_THREAD_FREE();
    NK_MEM_FREE();
    
    return 0;
}