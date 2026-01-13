#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

#define MEM_SIZE 0x10000 // 64KB
struct CPU;


uint8_t* mem_create(void);
void mem_destroy(uint8_t *mem);

uint8_t  mem_read8(uint8_t *mem, uint32_t addr);
uint32_t mem_read32(uint8_t *mem, uint32_t addr);


void mem_write8(uint8_t *mem, uint32_t addr, uint8_t val);
void mem_write32(uint8_t *mem, uint32_t addr, uint32_t val);

void push32(uint8_t *mem, struct CPU *cpu, uint32_t val);
uint32_t pop32(uint8_t *mem, struct CPU *cpu);

void call_rel32(uint8_t *mem, struct CPU *cpu, int32_t rel);
void ret(uint8_t *mem, struct CPU *cpu);


#endif
