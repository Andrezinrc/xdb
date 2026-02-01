#ifndef KERNEL_H
#define KERNEL_H

#include "../cpu.h"
#include "../process.h"

#define DEBUG_KERNEL
#ifdef DEBUG_KERNEL

extern int kernel_debug_enabled;

#define KDEBUG(...) \
    do { if (kernel_debug_enabled) printf(__VA_ARGS__); } while (0)
#endif

#define MAX_PROCS 32

#define SYS_EXIT  1
#define SYS_WRITE 4

//#define READ_SECRET
//#define READ_NONBLOCKING
#define SYS_READ  3

#define SYS_GETPID 20

void kernel_init(void);
void kernel_handle_syscall(struct fake_process *proc);

#endif
