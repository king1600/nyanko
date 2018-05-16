#include "alloc.h"
#include "vm.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char* argv[]) {
    nk_vm_t vm;
    nk_vm_init(&vm, 0, 0);

    nk_vm_this(&vm);

    nk_vm_free(&vm);
    return 0;
}