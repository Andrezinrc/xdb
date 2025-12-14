#include <stdio.h>
#include <string.h>
#include "debugger.h"
#include "ptrace-fake.h"

void dbg_prompt(char *buf, size_t size){
    printf("\033[1;32m(dbg)\033[0m ");
    fflush(stdout);
    fgets(buf, size, stdin);
    buf[strcspn(buf, "\n")] = 0;
}

void dbg_help(void){
    printf("\n=== Comandos do Debugger ===\n");
    printf("s          - Step (executa uma instrução)\n");
    printf("c          - Continue\n");
    printf("q          - Quit\n");
    printf("r reg val  - Escreve registrador\n");
    printf("w addr val - Escreve memória\n");
    printf("b addr     - Set breakpoint\n");
    printf("d addr     - Delete breakpoint\n");
    printf("h          - Esta ajuda\n\n");
}

void dbg_handle_cmd(struct Debugger *dbg, char *cmd, struct CPU *cpu, uint8_t *memory){
    if (cmd[0] == 'q') {
        dbg->running = -1;
        return;
    }

    if (cmd[0] == 'c') {
        dbg->running = 1;
        return;
    }

    if (cmd[0] == 's') {/* step */}
    if (cmd[0] == 'h') {
        dbg_help();
        return;
    }

    if (cmd[0] == 'b') {
        uint32_t addr;
        sscanf(cmd, "b %x", &addr);
        bp_set(addr, memory);
        return;
    }

    if (cmd[0] == 'd') {
        uint32_t addr;
        sscanf(cmd, "d %x", &addr);
        bp_clear(addr, memory);
        return;
    }

    if (cmd[0] == 'r') {
        char reg[8];
        int val;
        sscanf(cmd, "r %s %d", reg, &val);

        fake_ptrace(PTRACE_GETREGS, 1337, NULL, cpu);

        if (strcmp(reg, "eax") == 0) cpu->eax.e = val;
        else if (strcmp(reg, "ecx") == 0) cpu->ecx.e = val;

        fake_ptrace(PTRACE_SETREGS, 1337, NULL, cpu);
        return;
    }

    if (cmd[0] == 'w') {
        uint32_t addr, val;
        sscanf(cmd, "w %x %x", &addr, &val);
        fake_ptrace(PTRACE_POKEDATA, 1337, (void*)addr, &val);
        return;
    }
}
