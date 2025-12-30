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
    int debug_mode;
};

struct fake_process;

void cpu_init(struct CPU *cpu, uint32_t mem_size);
void cpu_step(struct CPU *cpu, uint8_t *memory, struct fake_process *proc);

void* get_reg(struct CPU *cpu, int index, int size);

void op_add(struct CPU *cpu, void *dst, void *src, int size);
void op_sub(struct CPU *cpu, void *dst, void *src, int size);
void op_mov(struct CPU *cpu, void *dst, void *src, int size);
void op_xor(struct CPU *cpu, void *dst, void *src, int size);
void op_cmp(struct CPU *cpu, void *dst, void *src, int size);

void update_ZF_SF(struct CPU *cpu, uint32_t res);
void update_add_flags(struct CPU *cpu, uint32_t a, uint32_t b, uint32_t res);
void update_sub_flags(struct CPU *cpu, uint32_t a, uint32_t b, uint32_t res);


#endif
