#ifndef KERNEL_H
#define KERNEL_H

#include <stdio.h>
#include "../cpu.h"
#include "process.h"

//#define DEBUG_KERNEL

#ifdef DEBUG_KERNEL
    #define KDEBUG(...) printf(__VA_ARGS__)
#else
    #define KDEBUG(...)
#endif

#define SYS_EXIT    1
#define SYS_WRITE   4
#define SYS_READ    3
#define SYS_GETPID 20

void kernel_init(void);
void kernel_handle_syscall(struct fake_process *proc);

#endif
