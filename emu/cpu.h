#ifndef CPU_H
#define CPU_H

#include <stdint.h>

union Reg32 {
    uint32_t e;
    struct { uint16_t x; };
    struct { uint8_t l, h; };
};

struct Flags { 
    uint8_t CF, ZF, SF, OF, DF; 
};

struct CPU {
    union Reg32 eax, ebx, ecx, edx;
    union Reg32 esi, edi, ebp, esp;
    uint32_t eip;
    struct Flags flags;
    int debug_mode;
    uint8_t last_opcode;
};

struct fake_process;

void cpu_init(struct CPU *cpu, uint32_t mem_size);
void cpu_step(struct CPU *cpu, uint8_t *memory, struct fake_process *proc);

#endif
