#include "alloc.h"
#include "gc.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
    NK_MEM_INIT();
    NK_MEM_THREAD_INIT();

    nk_gc_t gc;
    nk_gc_init(&gc);

    nk_value v;
    
    for (int i = 0; i < 1024; i++)
        v = nk_gc_alloc(&gc, NK_TYPE_STRING, 12);
        
    char* x = NK_PTR(char*, v);

    memcpy(x, "Hello world", 11);
    printf("%s\n", x);

    nk_gc_free(&gc);

    NK_MEM_THREAD_FREE();
    NK_MEM_FREE();
    
    return 0;
}