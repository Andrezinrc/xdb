#include "kernel.h"
#include <stdio.h>

static int kernel_inicializado = 0;

void kernel_init(void) {
    if (kernel_inicializado) return;
    
    KDEBUG("Inicializado\n");
    kernel_inicializado = 1;
}

static void handle_write(struct CPU *cpu, uint8_t *memoria) {
    if (cpu->ebx.e != 1) {
        cpu->eax.e = -1;  // Erro
        return;
    }

    printf("Processo: ");
    for (uint32_t i=0;i<cpu->edx.e;i++) {
        putchar(memoria[cpu->ecx.e + i]);
    }
    printf("\n");
    
    cpu->eax.e = cpu->edx.e;
}

static void handle_exit(struct CPU *cpu, uint8_t *memoria) {
    printf("Processo terminou %d\n", cpu->ebx.e);
    memoria[0] = 0xFF;
}

void kernel_handle_syscall(struct CPU *cpu, uint8_t *memory) {
    if (!kernel_inicializado) kernel_init();
    
    int syscall_num = cpu->eax.e;
    
    KDEBUG("Syscall %d em EIP=0x%08X\n", syscall_num, cpu->eip);
    
    switch (syscall_num) {
        case SYS_EXIT:
            handle_exit(cpu, memory);
            break;
            
        case SYS_WRITE:
            handle_write(cpu, memory);
            break;
            
        default:
            KDEBUG("Syscall %d não implementada\n", syscall_num);
            cpu->eax.e = -1;
            break;
    }
}
