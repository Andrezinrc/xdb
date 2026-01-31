#ifndef CPU_OPS_H
#define CPU_OPS_H

#include "cpu.h"

void op_add(struct CPU *cpu, void *dst, void *src, int size);
void op_sub(struct CPU *cpu, void *dst, void *src, int size);
void op_mov(struct CPU *cpu, void *dst, void *src, int size);
void op_xor(struct CPU *cpu, void *dst, void *src, int size);
void op_cmp(struct CPU *cpu, void *dst, void *src, int size);
void op_and(struct CPU *cpu, void *dst, void *src, int size);
void op_or(struct CPU *cpu, void *dst, void *src, int size);

#endif
