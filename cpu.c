#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"

void cpu_init(struct CPU *cpu) {
    memset(cpu, 0, sizeof(struct CPU));
    cpu->esp.e = MEM_SIZE - 4;
    cpu->eip = 0;
}

uint32_t read32(uint8_t *mem, uint32_t addr){
    return mem[addr] |
        ((uint32_t)mem[addr + 1] << 8) |
        ((uint32_t)mem[addr + 2] << 16) |
        ((uint32_t)mem[addr + 3] << 24);
}


void cpu_step(struct CPU *cpu, uint8_t *memory) {
    uint8_t opcode = memory[cpu->eip];

    switch (opcode) {

        case 0xB8: { // MOV EAX, imm32
            uint32_t imm = read32(memory, cpu->eip + 1);
            cpu->eax.e = imm;
            cpu->eip += 5;
            break;
        }

        case 0xB9: { // MOV ECX, imm32
            uint32_t imm = read32(memory, cpu->eip + 1);
            cpu->ecx.e = imm;
            cpu->eip += 5;
            break;
        }
        
        case 0xF4: {
            printf("Encerrando.\n");
            exit(1);
        }
        
        
        default:
            printf("Opcode desconhecido em EIP=0x%08X: 0x%02X\n",
                   cpu->eip, opcode);
            exit(1);
    }
}

