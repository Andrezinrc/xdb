#ifndef PTRACE_FAKE
#define PTRACE_FAKE

#include <stdint.h>
#include <sys/types.h>
#include "cpu.h"
#include "kernel/process.h"

#define MAX_BREAKPOINTS 16

enum {
    PTRACE_ATTACH = 1,
    PTRACE_DETACH,
    PTRACE_CONT,
    PTRACE_SINGLESTEP,
    PTRACE_GETREGS,
    PTRACE_SETREGS,
    PTRACE_PEEKDATA,
    PTRACE_POKEDATA
};

long fake_ptrace(int request, pid_t pid, void *addr, void *data);

struct fake_process *fp_get(pid_t pid);
void fp_register(struct fake_process *p);

struct breakpoint { uint32_t addr; uint8_t orig_byte; int active; };

extern struct breakpoint breakpoints[MAX_BREAKPOINTS];

void bp_set(uint32_t addr, uint8_t *memory);
void bp_clear(uint32_t addr, uint8_t *memory);
int bp_check(struct CPU *cpu);

#endif
