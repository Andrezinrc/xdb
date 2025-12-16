#include <stdio.h>
#include "ptrace-fake.h"
#include <stddef.h>
#include <string.h>
#include <errno.h>

#include "cpu.h"
#include "memory.h"

struct breakpoint breakpoints[MAX_BREAKPOINTS];
static struct fake_process *proc_table[32];
static int proc_count = 0;

void fp_register(struct fake_process *p){
    if(proc_count < 32) {
        proc_table[proc_count++] = p;
    }
}

struct fake_process *fp_get(pid_t pid) {
    for(int i=0;i<proc_count;i++)
        if(proc_table[i]->pid == pid)
            return proc_table[i];
    return NULL;
}

// Implementação fake do ptrace(2) do Linux
long fake_ptrace(int request, pid_t pid, void *addr, void *data) {
    struct fake_process *p = fp_get(pid);
    if(!p) return -ESRCH;

    switch(request) {

    case PTRACE_ATTACH:
        p->stopped = 1;
        return 0;

    case PTRACE_DETACH:
        p->stopped = 0;
        return 0;

    case PTRACE_SINGLESTEP:
        cpu_step(&p->cpu, p->memory);
        p->stopped = 1;
        return 0;

    case PTRACE_CONT:
        p->stopped = 0;
        return 0;

    case PTRACE_GETREGS:
        memcpy(data, &p->cpu, sizeof(struct CPU));
        return 0;

    case PTRACE_SETREGS:
        memcpy(&p->cpu, data, sizeof(struct CPU));
        return 0;

    case PTRACE_PEEKDATA:
        *(uint32_t*)data =
            *(uint32_t*)(p->memory + (uintptr_t)addr);
        return 0;

    case PTRACE_POKEDATA:
        *(uint32_t*)(p->memory + (uintptr_t)addr) =
            *(uint32_t*)data;
        return 0;
    }

    return -EINVAL;
}

void bp_set(uint32_t addr, uint8_t *memory) {
    for (int i=0;i<MAX_BREAKPOINTS;i++) {
        if (!breakpoints[i].active) {
            breakpoints[i].addr = addr;
            breakpoints[i].orig_byte = memory[addr];
            memory[addr] = 0xCC; // INT3 - breakpoint de software
            breakpoints[i].active = 1;
            printf("Breakpoint definido em 0x%X\n", addr);
            return;
        }
    }
    printf("Sem espaço para mais breakpoints!\n");
}

void bp_clear(uint32_t addr, uint8_t *memory) {
    for (int i=0;i<MAX_BREAKPOINTS;i++) {
        if (breakpoints[i].active && breakpoints[i].addr == addr) {
            memory[addr] = breakpoints[i].orig_byte;
            breakpoints[i].active = 0;
            return;
        }
    }
}

int bp_check(struct CPU *cpu) {
    for (int i=0;i<MAX_BREAKPOINTS;i++) {
        if (breakpoints[i].active && breakpoints[i].addr == cpu->eip) {
            return i;
        }
    }
    return -1;
}
