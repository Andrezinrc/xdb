#include "kernel.h"
#include "syscall.h"

static int kernel_initialized = 0;
void kernel_init(void) {
    if (kernel_initialized) return;
    KDEBUG("Inicializado\n");
    kernel_initialized = 1;
}

void kernel_handle_syscall(struct fake_process *proc) {
    if (!kernel_initialized)
        kernel_init();

    dispatch_syscall(proc);
}
