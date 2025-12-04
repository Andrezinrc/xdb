#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "cpu.h"
#include "memory.h"
#include "disasm.h"



int main(){
    
    struct CPU cpu;
    uint8_t *memory = mem_create();
    cpu_init(&cpu, MEM_SIZE);
    

    // mov eax, 5
    memory[0] = 0xB8;
    memory[1] = 0x05;
    memory[2] = 0x00;
    memory[3] = 0x00;
    memory[4] = 0x00;
    
    // mov ecx, 5
    memory[5] = 0xB9;
    memory[6] = 0x05;
    memory[7] = 0x00;
    memory[8] = 0x00;
    memory[9] = 0x00;
    
    // cmp eax, ecx
    memory[10] = 0x39;
    memory[11] = 0xC8;
    
    // inc eax
    memory[12] = 0x40;
    // dec eax
    memory[13] = 0x48;
    
    // add eax, ecx
    memory[14] = 0x01;
    memory[15] = 0xC8;
    
    // sub eax, ecx
    memory[16] = 0x29;
    memory[17] = 0xC8;
    
    // add eax, imm
    memory[18] = 0x05;
    memory[19] = 0x05;
    memory[20] = 0x00;
    memory[21] = 0x00;
    memory[22] = 0x00;
    
     // mov ecx, eax
    memory[23] = 0x89;
    memory[24] = 0xC1;
    
    // cmp eax, ecx
    memory[25] = 0x39;
    memory[26] = 0xC8;
  
    // HLT
    memory[27] = 0xF4;



    
    while(1){
        cpu_step(&cpu, memory);
        disassemble(memory, cpu.eip);
        //print_state(&cpu);

     
        if(cpu.eip >= MEM_SIZE || memory[cpu.eip] == 0xF4) break; 
    }

    return 0;
}

