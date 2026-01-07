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
    static uint32_t sib_used = 0;

    if (mod==0) {
        // [disp32] quando r/m = 5
        if (rm==5) {
            return mem_read32(memory, cpu->eip + 2);
        }
        
        // SIB byte quando r/m = 4
        if (rm == 4) {
            uint8_t sib = mem_read8(memory, cpu->eip + 2 + sib_used);
            sib_used = 1;
            
            uint8_t base = sib & 0x07;
            uint8_t index = (sib >> 3) & 0x07;
            uint8_t scale = (sib >> 6) & 0x03;
            
            if (index==4){
                return *(uint32_t*)get_reg(cpu, base, 32);
            }
            
            printf("SIB complexo não suportado: base=%d, index=%d, scale=%d\n", 
                   base, index, scale);
            exit(1);
        }

        return *(uint32_t*)get_reg(cpu, rm, 32);
    }

    printf("Modo ModRM não suportado (mod=%d, r/m=%d)\n", mod, rm);
    exit(1);
}
