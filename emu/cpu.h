#ifndef CPU_H
#define CPU_H

#include <stdint.h>

union Reg32 {
    uint32_t e;
    struct { uint16_t x; };
    struct { uint8_t l, h; };
};

struct Flags { 
    uint8_t CF, ZF, SF, OF; 
};

struct CPU {
    union Reg32 eax, ebx, ecx, edx;
    union Reg32 esi, edi, ebp, esp;
    uint32_t eip;
    struct Flags flags;
};

void cpu_init(struct CPU *cpu, uint32_t mem_size);
void print_state(struct CPU *cpu);
void cpu_step(struct CPU *cpu, uint8_t *memory);



#endif
