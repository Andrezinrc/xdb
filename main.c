#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "cpu.h"
#include "memory.h"
#include "disasm.h"
#include "ptrace-fake.h"
#include "debugger.h"


int main(){
    struct CPU cpu;
    uint8_t *memory = mem_create();
    cpu_init(&cpu, MEM_SIZE);
    
    
    static struct fake_process proc;
    proc.pid = 1337;
    proc.cpu = cpu;
    proc.memory = memory;
    proc.stopped = 0;
    fp_register(&proc);

    FILE *f = fopen("test.bin", "rb");
    if(!f) {
       printf("Erro ao abrir arquivo\n");
       return -1;
    }
    fseek(f, 0, SEEK_END);
    long t = ftell(f);
    rewind(f);

    fread(memory, 1, t, f);
    fclose(f);
 
 
    fake_ptrace(PTRACE_ATTACH, 1337, NULL, NULL);
    
    char cmd[64];
    struct Debugger dbg = {0};
    
    while(1){
        fake_ptrace(PTRACE_GETREGS, 1337, NULL, &cpu);
        
         if (!dbg.running) {
            disassemble(memory, cpu.eip);
            dbg_prompt(cmd, sizeof(cmd));
            dbg_handle_cmd(&dbg, cmd, &cpu, memory);

            if (dbg.running == -1)
                break;
        }

        // Checa se hitou breakpoint
        int idx = bp_check(&cpu);
        if (idx >= 0) {
            printf("Breakpoint atingido em 0x%X\n", cpu.eip);
            bp_clear(cpu.eip, memory);
            dbg.running = 0;
        }
        
         fake_ptrace(PTRACE_SINGLESTEP, 1337, NULL, NULL);
         
         if(cpu.eip >= MEM_SIZE || memory[cpu.eip] == 0xF4) break; 
    }
    return 0;
}
