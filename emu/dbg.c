#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "dbg.h"
#include "mem.h"
#include "ptrace-fake.h"

static uint8_t placeholder_map[MEM_SIZE];

void dbg_prompt(char *buf, size_t size){
    printf("\033[1;32m(dbg)\033[0m ");
    fflush(stdout);
    fgets(buf, size, stdin);
    buf[strcspn(buf, "\n")] = 0;
}

void dbg_help(void){
    printf("\n=== Debugger Commands ===\n");
    printf("s          - Step (execute one instruction)\n");
    printf("c          - Continue execution\n");
    printf("q          - Quit debugger\n");
    printf("r reg val  - Write register value\n");
    printf("w addr val - Write memory (use '??' for placeholders)\n");
    printf("f addr val - Fill placeholders only\n");
    printf("b addr     - Set breakpoint\n");
    printf("d addr     - Delete breakpoint\n");
    printf("x addr len - Examine memory (hex + ASCII)\n");
    printf("p reg      - Print register value (use 'all' for all registers)\n");
    printf("h          - Show this help\n\n");
}

static void dbg_examine(uint32_t addr, uint32_t len) {
    uint32_t i;

    for (i=0;i<len;i+=16){
        printf("\033[96m%08X:\033[97m ", addr + i);

        uint32_t line_vals[16];
        int bytes_in_line = (len - i >= 16) ? 16 : (len - i);

        for (int j=0;j<bytes_in_line;j++) {
            uint32_t val;
            uint32_t cur = addr + i + j;

            fake_ptrace(
                PTRACE_PEEKDATA,
                1337,
                (void*)cur,
                &val
            );

            line_vals[j] = val & 0xFF;

            if (cur < MEM_SIZE && placeholder_map[cur]) {
                printf("\033[93m??\033[97m ");
            } else {
                printf("\033[92m%02X\033[97m ", line_vals[j]);
            }
        }
        
        /* Preenche espaço se a linha tiver menos de 16 bytes */
        for (int j=bytes_in_line;j<16;j++) {
            printf("   ");
        }

        printf("\033[97m |");
        for (int j=0;j<bytes_in_line;j++) {
            uint8_t c = line_vals[j];
            printf("%c", (c >= 32 && c <= 126) ? c : '.');
        }
        printf("|\033[97m\n");
    }
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

    if (cmd[0] == 's'){}
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
        uint32_t val;
        sscanf(cmd, "r %s %x", reg, &val);

        fake_ptrace(PTRACE_GETREGS, 1337, NULL, cpu);

        if (strcmp(reg, "eax") == 0) cpu->eax.e = val;
        else if (strcmp(reg, "ecx") == 0) cpu->ecx.e = val;
        else if (strcmp(reg, "eip") == 0) cpu->eip = val;

        fake_ptrace(PTRACE_SETREGS, 1337, NULL, cpu);
        return;
    }

    if (cmd[0] == 'w') {
        char *saveptr;
        char *token = strtok_r(cmd + 1, " ", &saveptr);
    
        if (!token) {
            printf("Uso: w <addr> <byte1> [byte2 ...]\n");
            return;
        }
    
        uint32_t addr;
        if (sscanf(token, "%x", &addr) != 1) {
            printf("Endereco invalido\n");
            return;
        }
    
        int count = 0;
        int placeholder_count = 0;
        
        while ((token = strtok_r(NULL, " ", &saveptr)) != NULL){
        
            if (strcmp(token, "??")==0) {
                if (addr < MEM_SIZE) {
                    placeholder_map[addr] = 1;
                }
                
                addr++;
                placeholder_count++;
                continue;
            }
        
            uint32_t val;
            if (sscanf(token, "%x", &val) != 1) {
                printf("Valor invalido: %s\n", token);
                break;
            }
        
            if (addr < MEM_SIZE) {
                placeholder_map[addr] = 0;
            }
        
            uint8_t b = val & 0xFF;
            fake_ptrace(PTRACE_POKEDATA, 1337, (void*)addr, &b);
        
            addr++;
            count++;
        }
    
        printf("\033[36m%d byte(s) escritos", count);
        if (placeholder_count > 0) {
            printf(" + %d placeholder(s)", placeholder_count);
        }
        printf("\033[0m\n");
        return;
    }

    if (cmd[0] == 'f') {
        char *saveptr;
        char *token = strtok_r(cmd + 1, " ", &saveptr);
    
        if (!token) {
            printf("Uso: f <addr> <byte1> [byte2 ...]\n");
            return;
        }
    
        uint32_t addr;
        if (sscanf(token, "%x", &addr) != 1) {
            printf("Endereco invalido\n");
            return;
        }
    
        int count = 0;
        
        while ((token = strtok_r(NULL, " ", &saveptr)) != NULL){
        
            if (addr >= MEM_SIZE) {
                printf("Erro: endereco fora da memoria\n");
                break;
            }
        
            if (!placeholder_map[addr]) {
                printf("Erro: 0x%08X nao eh placeholder\n", addr);
                break;
            }
        
            uint32_t val;
            if (sscanf(token, "%x", &val) != 1) {
                printf("Valor invalido: %s\n", token);
                break;
            }
        
            uint8_t b = val & 0xFF;
            fake_ptrace(PTRACE_POKEDATA, 1337, (void*)addr, &b);
            placeholder_map[addr] = 0;
        
            addr++;
            count++;
        }
    
        printf("\033[36m%d placeholder(s) preenchidos\033[0m\n", count);
        return;
    }
    
    if (cmd[0] == 'x') {
        uint32_t addr, len;
        if (sscanf(cmd, "x %x %u", &addr, &len) == 2) {
            dbg_examine(addr, len);
        } else {
            printf("Uso: x <addr> <len>\n");
        }
        return;
    }

    if (tolower(cmd[0]) == 'p') {
        char arg[32];
        sscanf(cmd, "p %s", arg);
    
        if (strcmp(arg, "eax") == 0) printf("EAX = 0x%08X\n", cpu->eax.e);
        else if (strcmp(arg, "ebx") == 0) printf("EBX = 0x%08X\n", cpu->ebx.e);
        else if (strcmp(arg, "ecx") == 0) printf("ECX = 0x%08X\n", cpu->ecx.e);
        else if (strcmp(arg, "edx") == 0) printf("EDX = 0x%08X\n", cpu->edx.e);
        else if (strcmp(arg, "esp") == 0) printf("ESP = 0x%08X\n", cpu->esp.e);
        else if (strcmp(arg, "ebp") == 0) printf("EBP = 0x%08X\n", cpu->ebp.e);
        else if (strcmp(arg, "esi") == 0) printf("ESI = 0x%08X\n", cpu->esi.e);
        else if (strcmp(arg, "edi") == 0) printf("EDI = 0x%08X\n", cpu->edi.e);
        else if (strcmp(arg, "eip") == 0) printf("EIP = 0x%08X\n", cpu->eip);
        else if (strcmp(arg, "flags") == 0) {
            printf("ZF=%d SF=%d CF=%d OF=%d\n",
                cpu->flags.ZF,
                cpu->flags.SF,
                cpu->flags.CF,
                cpu->flags.OF);
        } else if (strcmp(arg, "all") == 0) {
            printf(
                "EAX=0x%08X  EBX=0x%08X  ECX=0x%08X  EDX=0x%08X\n"
                "ESP=0x%08X  EBP=0x%08X  ESI=0x%08X  EDI=0x%08X\n"
                "EIP=0x%08X\n"
                "FLAGS: ZF=%d SF=%d CF=%d OF=%d\n",
                cpu->eax.e, cpu->ebx.e, cpu->ecx.e, cpu->edx.e,
                cpu->esp.e, cpu->ebp.e, cpu->esi.e, cpu->edi.e,
                cpu->eip,
                cpu->flags.ZF, cpu->flags.SF, cpu->flags.CF, cpu->flags.OF
            );
        } else {
            printf("reg deseconhecido: %s\n", arg);
        }
        return;
    }
}

void dbg_trace_syscall(struct CPU *cpu){
    switch (cpu->eax.e){
        case 1:
            printf("[syscall] exit(status=%d)\n", cpu->ebx.e);
            break;
        case 3:
            printf("[syscall] read(fd=%d, buf=0x%X, len=%d)\n",
                   cpu->ebx.e, cpu->ecx.e, cpu->edx.e);
            break;
        case 4:
            printf("[syscall] write(fd=%d, buf=0x%X, len=%d)\n",
                   cpu->ebx.e, cpu->ecx.e, cpu->edx.e);
            break;
        default:
            printf("[syscall] deseconhecido eax=%d\n", cpu->eax.e);
    }
}
