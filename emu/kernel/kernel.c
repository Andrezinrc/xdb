#include "kernel.h"
#include "../mem.h"
#include <stdio.h>
#include <unistd.h>

static int kernel_initialized = 0;

void kernel_init(void) {
    if (kernel_initialized) return;
    KDEBUG("Inicializado\n");
    kernel_initialized = 1;
}

static void handle_write(struct CPU *cpu, uint8_t *memory) {
    if (cpu->ebx.e != 1) return (void)(cpu->eax.e = -1);

    uint32_t start = cpu->ecx.e;
    uint32_t count = cpu->edx.e;

    if (start >= MEM_SIZE || start + count > MEM_SIZE) {
        cpu->eax.e = -1;
        return;
    }

    printf("\033[1;37m");
    for (uint32_t i = 0; i < count; i++)
        putchar(memory[start + i]);
    printf("\033[0m\n");

    cpu->eax.e = count;
}

static void handle_read(struct CPU *cpu, uint8_t *memory, struct fake_process *proc) {
    if (cpu->ebx.e != 0) { cpu->eax.e = -1; return; }

    char *buf = (char*)&memory[cpu->ecx.e];
    size_t max_count = cpu->edx.e;

    printf("\033[1;33m> \033[0m");
    fflush(stdout);

    char line[256];
    if (fgets(line, sizeof(line), stdin) != NULL) {
        buf[0] = line[0];
        cpu->eax.e = 1;
        KDEBUG("\033[90mread() = 1 byte: '%c' (0x%02x)\033[0m\n",
               buf[0], (unsigned char)buf[0]);
    } else {
        cpu->eax.e = 0;
    }
}

static void handle_exit(struct CPU *cpu, uint8_t *memory) {
    printf("Processo terminou %d\n", cpu->ebx.e);
    memory[0] = 0xFF;
}

void kernel_handle_syscall(struct fake_process *proc) {
    if (!kernel_initialized) kernel_init();

    struct CPU *cpu = &proc->cpu;
    uint8_t *memory = proc->memory;
    int syscall_num = cpu->eax.e;

    KDEBUG("Syscall %d em EIP=0x%08X\n", syscall_num, cpu->eip);

    switch (syscall_num) {
        case SYS_EXIT:  handle_exit(cpu, memory); break;
        case SYS_WRITE: handle_write(cpu, memory); break;
        case SYS_READ:  handle_read(cpu, memory, proc); break;
        case SYS_GETPID: cpu->eax.e = proc->pid; break;
        default: cpu->eax.e = -1; KDEBUG("Syscall %d não implementada\n", syscall_num); break;
    }
}
