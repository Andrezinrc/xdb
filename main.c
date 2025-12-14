#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "cpu.h"
#include "memory.h"
#include "disasm.h"
#include "ptrace-fake.h"
#include "debugger.h"
#include "emu/kernel/kernel.h"

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

    FILE *f = fopen("test_syscall.bin", "rb");
    if(!f) {
       printf("Erro ao abrir arquivo\n");
       return -1;
    }
    fseek(f, 0, SEEK_END);
    long t = ftell(f);
    rewind(f);

    fread(memory, 1, t, f);
    fclose(f);
 
    char *msg = "Ola mundo!";
    memcpy(memory + 0x1000, msg, strlen(msg)); 
    fake_ptrace(PTRACE_ATTACH, 1337, NULL, NULL);
    
    char cmd[64];
    struct Debugger dbg = {0};
    
    while(1){
        fake_ptrace(PTRACE_GETREGS, 1337, NULL, &cpu);
        
         if (!dbg.running) {
            disassemble(memory, cpu.eip);
            printf("EAX=%08X  ECX=%08X  FLAGS: ZF=%d SF=%d CF=%d OF=%d\n",
       cpu.eax.e, cpu.ecx.e, cpu.flags.ZF, cpu.flags.SF, cpu.flags.CF, cpu.flags.OF);
            dbg_prompt(cmd, sizeof(cmd));
            dbg_handle_cmd(&dbg, cmd, &cpu, memory);

            if (dbg.running == -1)
                break;
        }

        int idx = bp_check(&cpu);
        if (idx >= 0) {
            printf("Breakpoint atingido em 0x%X\n", cpu.eip);
            bp_clear(cpu.eip, memory);
            cpu.eip--;
            dbg.running = 0;
        }
        
        fake_ptrace(PTRACE_SINGLESTEP, 1337, NULL, NULL);
        
        if (memory[cpu.eip] == 0xCD && memory[cpu.eip + 1] == 0x80){
            kernel_handle_syscall(&cpu, memory);
            cpu.eip += 2;
            continue;
        } 

        if (memory[0] == 0xFF){
            printf("Processo terminado via syscall exit\n");
            break;
        }
        if(cpu.eip >= MEM_SIZE || memory[cpu.eip] == 0xF4) break; 
    }
    return 0;
}
