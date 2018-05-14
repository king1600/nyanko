#include "alloc.h"
#include "vm.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
    nk_vm_t vm;
    nk_vm_init(&vm, -1, -1);
    nk_vm_this(&vm);
    
    nk_actor_t* a = nk_actor_spawn();

    nk_actor_send(a, NK_VALUE(NK_TYPE_INT, 0));
    nk_actor_send(a, NK_VALUE(NK_TYPE_INT, 1));
    nk_actor_send(a, NK_VALUE(NK_TYPE_INT, 2));

    for (nk_value v = nk_actor_recv(a); !nk_is_null(v); v = nk_actor_recv(a)) {
        printf("%d\n", nk_get_int(v));
    }
    

    nk_actor_free(a);

    nk_vm_free(&vm);
    return 0;
}