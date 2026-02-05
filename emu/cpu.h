#ifndef CPU_H
#define CPU_H

#include <stdint.h>

union Reg32 {
    uint32_t e;       // EAX
    struct { 
        uint16_t x;   // AX
    };
    struct { 
        uint8_t l;    // AL
        uint8_t h;    // AH
    };
};


struct Flags { 
    uint8_t CF; // Carry Flag
    uint8_t ZF; // Zero Flag
    uint8_t SF; // Sign Flag
    uint8_t OF; // Overflow Flag
};

#define STACK_SIZE 0x100 // Tamanho da pilha da CPU

struct CPU {
    union Reg32 eax, ebx, ecx, edx;
    union Reg32 esi, edi, ebp, esp;
    uint32_t eip;
    struct Flags flags;

    uint32_t stack_base;
    uint32_t stack_limit;
};

struct fake_process;

void cpu_init(struct CPU *cpu, uint32_t mem_size);
void cpu_step(struct CPU *cpu, uint8_t *memory, struct fake_process *proc);

#endif
