#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "cpu.h"
#include "mem.h"
#include "dasm.h"
#include "ptrace-fake.h"
#include "dbg.h"
#include "emu/kernel/kernel.h"

#define FAKE_PID 1337
#define OPCODE_FADE 20
#define CLR_LINE "\033[2K"

static uint8_t opcode_heat[256];

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

static void fade_opcodes(void) {
    for (int i=0;i<256;i++) {
        if (opcode_heat[i]>0)
            opcode_heat[i]--;
    }
}

static void draw_opcode_table(void) {
   printf("\033[4;1H");
   for(int i = 0; i < 18; i++)
        printf(CLR_LINE "\n");
    printf("\033[4;1H");
 
   for (int r=0;r<16;r++) {
        for (int c=0;c<16;c++) {
            int op = (r<<4) | c;

            if (opcode_heat[op] > OPCODE_FADE / 2)
                printf("\033[1;32m%02X \033[0m", op);
            else if (opcode_heat[op] > 0)
                printf("\033[32m%02X \033[0m", op);
            else
                printf("\033[90m.. \033[0m");
        }
        printf("\n");
    }
    printf("\n");
}

static void print_banner(const char *prog){
    printf("\033[2J\033[H");
    printf("\033[36mxdb\033[0m\n");
    printf("Commands:\n");
    printf(" %s run   <program.bin>\n", prog);
    printf(" %s debug <program.bin>\n", prog);
}

static void print_header(struct fake_process *proc){
    printf("\033[H");
    printf(CLR_LINE "Stack top: 0x%08X\n", proc->cpu.esp.e);
    printf(CLR_LINE "Entry point: 0x%08X\n\n", proc->cpu.eip);
}

static void print_header_debug(void) {
    printf("\033[2J\033[H");
    printf("Use 'h' for help commands\n\n");
}

static void run_program(struct fake_process *proc){
    if (!proc) return;

    memset(opcode_heat, 0, sizeof(opcode_heat));

    printf("\033[2J");

    for (;;) {
        cpu_step(&proc->cpu, proc->memory, proc);

        opcode_heat[proc->cpu.last_opcode] = OPCODE_FADE;
        fade_opcodes();

        print_header(proc);
        draw_opcode_table();

        printf("\033[30;1H");

        fflush(stdout);
        usleep(20000);

        if (proc->cpu.eip >= MEM_SIZE
            || proc->memory[proc->cpu.eip] == 0xF4
            || !proc->alive) {
            break;
        }
    }
}

static void debugger_loop(struct fake_process *proc) {
    if(!proc) return;

    char cmd[64];
    struct Debugger dbg = {0};

    fake_ptrace(PTRACE_ATTACH, FAKE_PID, NULL, NULL);  
    print_header_debug();
 
    for (;;) {
        fake_ptrace(PTRACE_GETREGS, FAKE_PID, NULL, &proc->cpu);
        
        if (!dbg.running) {
            disassemble(proc->memory, proc->cpu.eip);
            dbg_prompt(cmd, sizeof(cmd));
            dbg_handle_cmd(&dbg, cmd, &proc->cpu, proc->memory);
            if (dbg.running == -1) break;
        }

        int idx = bp_check(&proc->cpu);
        if (idx >= 0) {
            printf("\033[1;31m=>\033[0m Breakpoint em 0x%X\n",
                   proc->cpu.eip);
            bp_clear(proc->cpu.eip, proc->memory);
            dbg.running = 0;
            continue;
        }

        fake_ptrace(PTRACE_SINGLESTEP, FAKE_PID, NULL, NULL);

        if (proc->cpu.eip >= MEM_SIZE
            || proc->memory[proc->cpu.eip] == 0xF4
            || !proc->alive) {
            break;
        }
    }
}

int main(int argc, char **argv){
    if (argc<3){
        print_banner(argv[0]);
        return 1;
    }

    int debug_mode = 0;
    if(strcmp(argv[1], "debug") == 0){
        debug_mode = 1;
    } else if(strcmp(argv[1], "run") == 0) {
        debug_mode = 0;
    } else {
        printf("Modo inválido!\n");
    }
    const char *bin_path = argv[2];

    uint8_t *memory = mem_create();
    if (load_bin(bin_path, memory) < 0) {
        printf("Erro ao abrir arquivo: %s\n", bin_path);
        return 1;
    }

    static struct fake_process proc;
    proc.pid = FAKE_PID;
    proc.memory = memory;
    proc.alive = 1;
    proc.cpu.debug_mode = debug_mode;

    kernel_debug_enabled = debug_mode;

    fp_register(&proc);
    cpu_init(&proc.cpu, MEM_SIZE);

    if(debug_mode) {
        debugger_loop(&proc);
    } else {
        run_program(&proc);
    }
}
