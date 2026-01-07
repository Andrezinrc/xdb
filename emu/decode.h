#pragma once
#include <stdint.h>
#include <stdbool.h>

struct CPU;

bool modrm_reg_reg(uint8_t modrm, uint8_t *reg, uint8_t *rm);
uint32_t modrm_mem_addr(struct CPU *cpu, uint8_t *memory, uint8_t modrm);
