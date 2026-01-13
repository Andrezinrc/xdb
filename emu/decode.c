#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "decode.h"
#include "cpu.h"
#include "mem.h"

/*
 * modrm = 0xD8 (11011000b)
 * mod = 11b (3) -> registrador-registrador
 * reg = 011b (3) -> ebx
 * r/m = 000b (0) -> eax
 * Retorna: true, *reg=3, *rm=0
 */
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
