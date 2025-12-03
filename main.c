#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "cpu.h"

int main(){
    
    struct CPU cpu;
    cpu_init(&cpu);
    
    uint8_t memory[MEM_SIZE] = {0};
    memset(memory, 0, sizeof(memory));


    // mov eax, 5
    memory[0] = 0xB8;
    memory[1] = 0x05;
    memory[2] = 0x00;
    memory[3] = 0x00;
    memory[4] = 0x00;
    
    // mov ecx, 3
    memory[5] = 0xB9;
    memory[6] = 0x03;
    memory[7] = 0x00;
    memory[8] = 0x00;
    memory[9] = 0x00;
    
    // HLT
    memory[10] = 0xF4;
    
    

    
    while(1){
        cpu_step(&cpu, memory);

        printf("EIP=%04X EAX=%08X ECX=%08X\n", cpu.eip, cpu.eax.e, cpu.ecx.e);
        if(cpu.eip >= MEM_SIZE) break;
    }

    return 0;
}
