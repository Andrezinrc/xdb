#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "decode.h"
#include "cpu.h"
#include "mem.h"

void* get_reg(struct CPU *cpu, int index, int size){
    if(size == 8){
        switch(index){
            case 0: return &cpu->eax.l;  // AL
            case 1: return &cpu->ecx.l;  // CL
            case 2: return &cpu->edx.l;  // DL
            case 3: return &cpu->ebx.l;  // BL
            case 4: return &cpu->eax.h;  // AH
            case 5: return &cpu->ecx.h;  // CH
            case 6: return &cpu->edx.h;  // DH
            case 7: return &cpu->ebx.h;  // BH
            default: return NULL;
        }
    } else if(size == 32){
        switch(index){
            case 0: return &cpu->eax.e;
            case 1: return &cpu->ecx.e;
            case 2: return &cpu->edx.e;
            case 3: return &cpu->ebx.e;
            case 4: return &cpu->esp.e;
            case 5: return &cpu->ebp.e;
            case 6: return &cpu->esi.e;
            case 7: return &cpu->edi.e;
            default: return NULL;
        }
    }
    return NULL;
}

bool modrm_reg_reg(uint8_t modrm, uint8_t *reg, uint8_t *rm) {
    if ((modrm >> 6) != 3) return false;
    *reg = (modrm >> 3) & 7;
    *rm  = modrm & 7;
    return true;
}

uint32_t modrm_mem_addr(struct CPU *cpu, uint8_t *memory, uint8_t modrm) {
    uint8_t mod = modrm >> 6;
    uint8_t rm  = modrm & 7;
    
    if (mod == 3) {
        printf("Erro: ModRM mod=3 é registrador, não memória\n");
        exit(1);
    }
    
    if (mod == 0) {
        if (rm == 5) {
            return mem_read32(memory, cpu->eip + 2);
        }
        return *(uint32_t*)get_reg(cpu, rm, 32);
    }

    if (mod == 2) {
        uint32_t base = *(uint32_t*)get_reg(cpu, rm, 32);
        uint32_t disp = mem_read32(memory, cpu->eip + 2);
        return base + disp;
    }

    printf("Modo ModRM não suportado (mod=%d, r/m=%d)\n", mod, rm);
    exit(1);
}
