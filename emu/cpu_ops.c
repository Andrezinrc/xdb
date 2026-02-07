#include "cpu_ops.h"
#include "cpu_flags.h"
#include <stdint.h>

// Soma src em dst, atualiza flags
void op_add(struct CPU *cpu, void *dst, void *src, int size){
    if(size==8){
        uint8_t *d=(uint8_t*)dst, *s=(uint8_t*)src;
        uint8_t res = *d + *s;
        cpu->flags.ZF = (res==0);
        cpu->flags.SF = (res>>7)&1;
        cpu->flags.CF = (res < *d);
        cpu->flags.OF = ((*d ^ res) & (*s ^ res)) >> 7;
        *d=res;
    } else {
        uint32_t *d=(uint32_t*)dst, *s=(uint32_t*)src;
        uint32_t res = *d + *s;
        update_add_flags(cpu, *d, *s, res);
        *d=res;
    }
}

// Subtrai src de dst, atualiza flags
void op_sub(struct CPU *cpu, void *dst, void *src, int size){
    if(size==8){
        uint8_t *d=(uint8_t*)dst, *s=(uint8_t*)src;
        uint8_t res = *d - *s;
        cpu->flags.ZF = (res==0);
        cpu->flags.SF = (res>>7)&1;
        cpu->flags.CF = (*d < *s);
        cpu->flags.OF = ((*d ^ *s) & (*d ^ res)) >> 7;
        *d=res;
    } else {
        uint32_t *d=(uint32_t*)dst, *s=(uint32_t*)src;
        uint32_t res = *d - *s;
        update_sub_flags(cpu, *d, *s, res);
        *d=res;
    }
}

// Move src para dst (sem afetar flags)
void op_mov(struct CPU *cpu, void *dst, void *src, int size){
    if(size==8) *(uint8_t*)dst = *(uint8_t*)src;
    else *(uint32_t*)dst = *(uint32_t*)src;
}

// XOR entre dst e src, atualiza ZF e SF
void op_xor(struct CPU *cpu, void *dst, void *src, int size){
    if(size==8){
        *(uint8_t*)dst ^= *(uint8_t*)src;
        cpu->flags.ZF = (*(uint8_t*)dst == 0);
        cpu->flags.SF = (*(uint8_t*)dst >> 7) & 1;
    } else {
        *(uint32_t*)dst ^= *(uint32_t*)src;
        update_ZF_SF(cpu, *(uint32_t*)dst);
    }
    cpu->flags.CF = 0;
    cpu->flags.OF = 0;
}

// Compara dst com src (subtração sem salvar resultado), atualiza flags
void op_cmp(struct CPU *cpu, void *dst, void *src, int size){
    if(size==8){
        uint8_t a = *(uint8_t*)dst;
        uint8_t b = *(uint8_t*)src;
        uint8_t res = a - b;
        cpu->flags.ZF = (res==0);
        cpu->flags.SF = (res>>7)&1;
        cpu->flags.CF = (a < b);
        cpu->flags.OF = ((a ^ b) & (a ^ res)) >> 7;
    } else {
        uint32_t res = *(uint32_t*)dst - *(uint32_t*)src;
        update_sub_flags(cpu, *(uint32_t*)dst, *(uint32_t*)src, res);
    }
}

// AND entre dst e src, atualiza ZF e SF, limpa CF e OF
void op_and(struct CPU *cpu, void *dst, void *src, int size){
    if(size==8) *(uint8_t*)dst &= *(uint8_t*)src;
    else *(uint32_t*)dst &= *(uint32_t*)src;
    update_ZF_SF(cpu, *(uint32_t*)dst);
    cpu->flags.CF = 0;
    cpu->flags.OF = 0;
}

// AND entre dst e src, atualiza ZF e SF, limpa CF e OF
void op_or(struct CPU *cpu, void *dst, void *src, int size){
    if(size==8) *(uint8_t*)dst |= *(uint8_t*)src;
    else *(uint32_t*)dst |= *(uint32_t*)src;
    update_ZF_SF(cpu, *(uint32_t*)dst);
    cpu->flags.CF = 0;
    cpu->flags.OF = 0;
}
