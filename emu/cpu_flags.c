#include "cpu_flags.h"
#include <stdint.h>

void update_ZF_SF(struct CPU *cpu, uint32_t res){
    cpu->flags.ZF = (res == 0);
    cpu->flags.SF = (res >> 31) & 1;
}

void update_add_flags(struct CPU* cpu, uint32_t a, uint32_t b, uint32_t res){
    update_ZF_SF(cpu, res);
    cpu->flags.CF = (res < a);
    cpu->flags.OF = ((a ^ res) & (b ^ res)) >> 31;
}

void update_sub_flags(struct CPU *cpu, uint32_t a, uint32_t b, uint32_t res) {
    update_ZF_SF(cpu, res);
    cpu->flags.CF = (a < b);
    cpu->flags.OF = (((int32_t)a ^ (int32_t)b) & ((int32_t)a ^ (int32_t)res)) >> 31;
}
