#pragma once
#include <stdint.h>
#include <stdbool.h>

#define DECODE_MODRM(modrm, mod, reg, rm) \
    do { \
        mod = (modrm) >> 6; \
        reg = ((modrm) >> 3) & 0x07; \
        rm  = (modrm) & 0x07; \
    } while(0) 

struct CPU;

void* get_reg(struct CPU *cpu, int index, int size);
bool modrm_reg_reg(uint8_t modrm, uint8_t *reg, uint8_t *rm);
uint32_t modrm_mem_addr(struct CPU *cpu, uint8_t *memory, uint8_t modrm);
