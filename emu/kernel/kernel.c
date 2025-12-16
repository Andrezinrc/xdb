#include "kernel.h"
#include <stdio.h>

static int kernel_initialized = 0;

void kernel_init(void) {
    if (kernel_initialized) return;
    
    KDEBUG("Inicializado\n");
    kernel_initialized = 1;
}

static void handle_write(struct CPU *cpu, uint8_t *memory) {
    if (cpu->ebx.e != 1) {
        cpu->eax.e = -1; // Apenas stdout
        return;
    }

    printf("Processo: ");
    printf("\033[32m");
    for (uint32_t i=0;i<cpu->edx.e;i++) {
        putchar(memory[cpu->ecx.e + i]);
    }
    printf("\033[0m\n");
    
    cpu->eax.e = cpu->edx.e;
}

static void handle_exit(struct CPU *cpu, uint8_t *memory) {
    printf("Processo terminou %d\n", cpu->ebx.e);
    memory[0] = 0xFF; // Sinaliza fim para main loop
}

// Manipulador de syscalls - convenção x86-32 (syscall num em EAX)
void kernel_handle_syscall(struct fake_process *proc) {
    if (!kernel_initialized) kernel_init();
	
    struct CPU *cpu = &proc->cpu;
    uint8_t *memory = proc->memory;
    int syscall_num = cpu->eax.e;
    
    KDEBUG("Syscall %d em EIP=0x%08X\n", syscall_num, cpu->eip);
    
    switch (syscall_num) {
        case SYS_EXIT:
            handle_exit(cpu, memory);
            break;            
        case SYS_WRITE:
            handle_write(cpu, memory);
            break;
        case SYS_GETPID:
            cpu->eax.e = proc->pid;
            break;
        default:
            KDEBUG("Syscall %d não implementada\n", syscall_num);
            cpu->eax.e = -1; // ENOSYS
            break;
    }
}
