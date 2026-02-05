#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "cpu.h"
#include "mem.h"
#include "dasm.h"
#include "ptrace-fake.h"
#include "dbg.h"
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

static void debugger_loop(struct fake_process *proc) {
    if(!proc) return;
    char cmd[64];
    struct Debugger dbg = {0};
    fake_ptrace(PTRACE_ATTACH, FAKE_PID, NULL, NULL);
    for(;;) {
        fake_ptrace(PTRACE_GETREGS, FAKE_PID, NULL, &proc->cpu);
        if (!dbg.running){
            disassemble(proc->memory, proc->cpu.eip);
            dbg_prompt(cmd, sizeof(cmd));
            dbg_handle_cmd(&dbg, cmd, &proc->cpu, proc->memory);
            if (dbg.running == -1) break;
        }
        int idx = bp_check(&proc->cpu);
        if (idx >= 0) {
            printf("\033[1;31m=>\033[0m");
            printf("Breakpoint atingido em 0x%X\n", proc->cpu.eip);
            bp_clear(proc->cpu.eip, proc->memory);
            dbg.running = 0;
            continue;
        }
        fake_ptrace(PTRACE_SINGLESTEP, FAKE_PID, NULL, NULL);
        if (proc->cpu.eip >= MEM_SIZE
            || proc->memory[proc->cpu.eip] == 0xF4
            || !proc->alive) break;
    }
}

static void print_banner(const char *prog){
    printf("\033[2J\033[H");
    printf("\033[36mxdb\033[0m\n");
    printf("Commands:\n");
    printf("%s <program.bin>\n", prog);
}

static void print_header(struct fake_process *proc){
    printf("\033[2J\033[H");
    printf("xdb> Entry=0x%08X ESP=0x%08X Memory=%u KB | type 'h' for help\n\n",
           proc->cpu.eip, proc->cpu.esp.e, MEM_SIZE / 1024);
}

int main(int argc, char **argv){
    if (argc<2){
        print_banner(argv[0]);
        return 1;
    }

    const char *bin_path = argv[1];
    uint8_t *memory = mem_create();
    
    if (load_bin(bin_path, memory) < 0) {
        printf("Erro ao abrir arquivo: %s\n", bin_path);
        return 1;
    }
    
    static struct fake_process proc;
    proc.pid = FAKE_PID;
    proc.memory = memory;
    proc.alive = 1;
    fp_register(&proc);
    cpu_init(&proc.cpu, MEM_SIZE);

    print_header(&proc);
    debugger_loop(&proc);
    
    return 0;
}
