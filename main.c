#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "cpu.h"
#include "memory.h"
#include "disasm.h"
#include "ptrace-fake.h"
#include "debugger.h"
#include "emu/kernel/kernel.h"


#define FAKE_PID 1337


int load_bin(const char *path, uint8_t *memory) {
    FILE *f = fopen(path, "rb");
    if (!f) return -1;

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    fread(memory, 1, size, f);
    fclose(f);

    return 0;
}

static void run_program(struct CPU *cpu, uint8_t *memory) {
    while (1) {

        /* syscall int 0x80 */
        if (memory[cpu->eip] == 0xCD &&
            memory[cpu->eip + 1] == 0x80) {

            kernel_handle_syscall(cpu, memory);
            cpu->eip += 2;
            continue;
        }

        /* syscall exit */
        if (memory[0] == 0xFF) {
            printf("\nProcesso terminado via syscall exit\n");
            break;
        }

        cpu_step(cpu, memory);

        if (cpu->eip >= MEM_SIZE || memory[cpu->eip] == 0xF4)
            break;
    }
}

static void debugger_loop(struct CPU *cpu, uint8_t *memory) {
    char cmd[64];
    struct Debugger dbg = {0};

    fake_ptrace(PTRACE_ATTACH, FAKE_PID, NULL, NULL);

    while (1) {
        fake_ptrace(PTRACE_GETREGS, FAKE_PID, NULL, cpu);

        if (!dbg.running){
            disassemble(memory, cpu->eip);

            printf("EIP=%08X  EAX=%08X  ECX=%08X\n", cpu->eip, cpu->eax.e, cpu->ecx.e);

            dbg_prompt(cmd, sizeof(cmd));
            dbg_handle_cmd(&dbg, cmd, cpu, memory);

            if (dbg.running == -1)
                break;
        }

        int idx = bp_check(cpu);
        if (idx >= 0) {
            printf("Breakpoint atingido em 0x%X\n", cpu->eip);
            bp_clear(cpu->eip, memory);
            cpu->eip--;
            dbg.running = 0;
            continue;
        }

        fake_ptrace(PTRACE_SINGLESTEP, FAKE_PID, NULL, NULL);

        if (memory[cpu->eip] == 0xCD &&
            memory[cpu->eip + 1] == 0x80) {

            kernel_handle_syscall(cpu, memory);
            cpu->eip += 2;
            continue;
        }

        if (memory[0] == 0xFF){
            printf("\nProcesso terminado via syscall exit\n");
            break;
        }

        if (cpu->eip >= MEM_SIZE || memory[cpu->eip] == 0xF4)
            break;
    }
}


int main(int argc, char **argv){
    if (argc<3) {
        printf("Uso:\n");
        printf("  %s debug <program.bin>\n", argv[0]);
        printf("  %s run   <program.bin>\n", argv[0]);
        return 1;
    }

    int debug_mode = 0;

    if (strcmp(argv[1], "debug") == 0){
        debug_mode = 1;
    } else if (strcmp(argv[1], "run") == 0){
        debug_mode = 0;
    } else {
        printf("Modo invalido: %s\n", argv[1]);
        return 1;
    }
    
    const char *bin_path = argv[2];

    struct CPU cpu;
    uint8_t *memory = mem_create();
    cpu_init(&cpu, MEM_SIZE);
    
    if (load_bin(bin_path, memory) < 0) {
        printf("Erro ao abrir arquivo: %s\n", bin_path);
        return 1;
    }
    
       
    static struct fake_process proc;
    proc.pid = FAKE_PID;
    proc.cpu = cpu;
    proc.memory = memory;
    proc.stopped = 0;
    fp_register(&proc);

    
    if (debug_mode){
        printf("[*] Modo DEBUG\n");
        debugger_loop(&cpu, memory);
    } else {
        printf("[*] Modo RUN\n");
        run_program(&cpu, memory);
    }

    return 0;
}
