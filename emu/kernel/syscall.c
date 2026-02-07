#include "syscall.h"
#include "kernel.h"
#include "../mem.h"
#include <stdio.h>

static void sys_write(struct fake_process *proc) {
    struct CPU *cpu = &proc->cpu;
    uint8_t *memory = proc->memory;

    if (cpu->ebx.e != 1) { 
        cpu->eax.e = -1; 
        return; 
    }

    uint32_t start = cpu->ecx.e;
    uint32_t count = cpu->edx.e;

    if (start + count > MEM_SIZE) {
        cpu->eax.e = -1;
        return;
    }

    for (uint32_t i = 0; i < count; i++)
        putchar(memory[start + i]);

    cpu->eax.e = count;
}

static void sys_read(struct fake_process *proc) {
    struct CPU *cpu = &proc->cpu;
    uint8_t *memory = proc->memory;

    if (cpu->ebx.e != 0) { 
        cpu->eax.e = -1; 
        return; 
    }

    char *buf = (char*)&memory[cpu->ecx.e];

    int c = getchar();
    if (c == EOF) {
        cpu->eax.e = 0;
        return;
    }

    buf[0] = (char)c;
    cpu->eax.e = 1;
}

static void sys_exit(struct fake_process *proc) {
    printf("Processo terminou %d\n", proc->cpu.ebx.e);
    proc->alive = 0;
}

void dispatch_syscall(struct fake_process *proc) {
    struct CPU *cpu = &proc->cpu;

    KDEBUG("Syscall %d em EIP=0x%08X\n", cpu->eax.e, cpu->eip);

    switch (cpu->eax.e) {
        case SYS_EXIT:   sys_exit(proc); break;
        case SYS_WRITE:  sys_write(proc); break;
        case SYS_READ:   sys_read(proc); break;
        case SYS_GETPID: cpu->eax.e = proc->pid; break;
        default:
            cpu->eax.e = -1;
            KDEBUG("Syscall %d não implementada\n", cpu->eax.e);
            break;
    }
}
