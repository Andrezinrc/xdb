#include "kernel.h"
#include "../mem.h"
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

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
    for (uint32_t i=0;i<cpu->edx.e;i++) {
        putchar(memory[cpu->ecx.e + i]);
    }
    printf("\033[0m\n");
    
    cpu->eax.e = count;
}

static void handle_read(struct CPU *cpu, uint8_t *memory, struct fake_process *proc) {
    int fd = cpu->ebx.e;
    char *buf = (char*)&memory[cpu->ecx.e];
    size_t max_count = cpu->edx.e;

    if (fd != 0) {
        cpu->eax.e = -1;
        return;
    }

    printf("\033[1;33m> \033[0m");
    fflush(stdout);

#ifdef READ_NONBLOCKING
    struct termios oldt, newt;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);

    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    int c = getchar();
    if (c == EOF) {
        cpu->eax.e = 0;
    } else {
        buf[0] = (char)c;
        cpu->eax.e = 1;
        KDEBUG("\033[90mread() = 1 byte: '%c' (0x%02x)\033[0m\n",
               buf[0], (unsigned char)buf[0]);
    }

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, flags);

#else
    if(max_count == 1) {
        char line[256];
        if(fgets(line, sizeof(line), stdin) != NULL) {
            buf[0] = line[0];
            cpu->eax.e = 1;

            KDEBUG("\033[90mread() = 1 byte: '%c' (0x%02x)\033[0m\n",
                   buf[0], (unsigned char)buf[0]);
        } else {
            cpu->eax.e = 0;
        }
    } else {
        size_t bytes_read = 0;
        int c;
        while(bytes_read < max_count) {
            c = getchar();
            if(c == EOF) break;

            buf[bytes_read++] = (char)c;
            if(c == '\n') break;
        }
        cpu->eax.e = (uint32_t)bytes_read;
        KDEBUG("\033[90mread() = %u bytes\033[0m\n", cpu->eax.e);
    }
#endif
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
        case SYS_EXIT:
            handle_exit(cpu, memory);
            break;            
        case SYS_WRITE:
            handle_write(cpu, memory);
            break;
		 case SYS_READ:
		     handle_read(cpu, memory, proc);
            break;
        case SYS_GETPID:
            cpu->eax.e = proc->pid;
            break;
        default:
            KDEBUG("Syscall %d não implementada\n", syscall_num);
            cpu->eax.e = -1;
            break;
    }
}
