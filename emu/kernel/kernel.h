#ifndef KERNEL_H
#define KERNEL_H

#include "../cpu.h"

//#define DEBUG_KERNEL

#ifdef DEBUG_KERNEL
    #define KDEBUG(...) printf(__VA_ARGS__)
#else
    #define KDEBUG(...)
#endif

// syscalls
#define SYS_EXIT  1
#define SYS_WRITE 4

void kernel_init(void);
void kernel_handle_syscall(struct CPU *cpu, uint8_t *memory);

#endif
