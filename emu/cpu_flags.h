#ifndef CPU_FLAGS_H
#define CPU_FLAGS_H

#include "cpu.h"

void update_ZF_SF(struct CPU *cpu, uint32_t res);
void update_add_flags(struct CPU* cpu, uint32_t a, uint32_t b, uint32_t res);
void update_sub_flags(struct CPU *cpu, uint32_t a, uint32_t b, uint32_t res);

#endif
