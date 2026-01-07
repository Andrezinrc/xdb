/*
 * Intel 64 and IA-32 Architectures Software Developer’s Manual
 * https://cdrdv2-public.intel.com/868139/325383-089-sdm-vol-2abcd.pdf
 */
 

/* Minimal ModRM support: only register-to-register forms (0xC0–0xC7).
 * Memory addressing via ModRM/SIB is intentionally not implemented yet.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "mem.h"
#include "decode.h"
#include "kernel/kernel.h"
#include "dbg.h"

#define HANDLE_MOV_IMM32 \
    case 0xB8: case 0xB9: case 0xBA: case 0xBB: \
    case 0xBC: case 0xBD: case 0xBE: case 0xBF: { \
        uint32_t imm = mem_read32(memory, cpu->eip + 1); \
        uint32_t *reg = get_reg(cpu, opcode - 0xB8, 32); \
        *reg = imm; \
        cpu->eip += 5; \
        break; \
    }
#define MAKE_OP(base, OP_NAME) \
    case base+0x0: { /* reg8, modrm8 */ \
        uint8_t modrm = mem_read8(memory, cpu->eip+1); \
        uint8_t reg, rm; \
        if(!modrm_reg_reg(modrm, &reg, &rm)){ printf("Mem não suportada\n"); exit(1); } \
        OP_NAME(cpu, get_reg(cpu, rm, 8), get_reg(cpu, reg, 8), 8); \
        cpu->eip += 2; \
        break; \
    } \
    case base+0x1: { /* reg32, modrm32 */ \
        uint8_t modrm = mem_read8(memory, cpu->eip+1); \
        uint8_t reg, rm; \
        if(!modrm_reg_reg(modrm, &reg, &rm)){ printf("Mem não suportada\n"); exit(1); } \
        OP_NAME(cpu, get_reg(cpu, rm, 32), get_reg(cpu, reg, 32), 32); \
        cpu->eip += 2; \
        break; \
    } \
    case base+0x2: { /* modrm8, reg8 */ \
        uint8_t modrm = mem_read8(memory, cpu->eip+1); \
        uint8_t reg, rm; \
        if(!modrm_reg_reg(modrm, &reg, &rm)){ printf("Mem não suportada\n"); exit(1); } \
        OP_NAME(cpu, get_reg(cpu, reg, 8), get_reg(cpu, rm, 8), 8); \
        cpu->eip += 2; \
        break; \
    } \
    case base+0x3: { /* modrm32, reg32 */ \
        uint8_t modrm = mem_read8(memory, cpu->eip+1); \
        uint8_t reg, rm; \
        if(!modrm_reg_reg(modrm, &reg, &rm)){ printf("Mem não suportada\n"); exit(1); } \
        OP_NAME(cpu, get_reg(cpu, reg, 32), get_reg(cpu, rm, 32), 32); \
        cpu->eip += 2; \
        break; \
    } \
    case base+0x4: { /* imm8, AL */ \
        uint8_t imm = mem_read8(memory, cpu->eip+1); \
        OP_NAME(cpu, &cpu->eax.l, &imm, 8); \
        cpu->eip += 2; \
        break; \
    } \
    case base+0x5: { /* imm32, EAX */ \
        uint32_t imm = mem_read32(memory, cpu->eip+1); \
        OP_NAME(cpu, &cpu->eax.e, &imm, 32); \
        cpu->eip += 5; \
        break; \
    }

void cpu_init(struct CPU *cpu, uint32_t mem_size) {
    memset(cpu, 0, sizeof(struct CPU));
    cpu->esp.e = mem_size - 4;
    cpu->eip = 0;
}

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

void* get_reg(struct CPU *cpu, int index, int size){
    if(size == 8){
        switch(index){
            case 0: return &cpu->eax.l;  // AL
            case 1: return &cpu->ecx.l;  // CL
            case 2: return &cpu->edx.l;  // DL
            case 3: return &cpu->ebx.l;  // BL
            default: return NULL;
        }
    } else if(size == 32){
        switch(index){
            case 0: return &cpu->eax.e;
            case 1: return &cpu->ecx.e;
            case 2: return &cpu->edx.e;
            case 3: return &cpu->ebx.e;
            case 4: return &cpu->esp.e;
            case 5: return &cpu->ebp.e;
            case 6: return &cpu->esi.e;
            case 7: return &cpu->edi.e;
            default: return NULL;
        }
    }
    return NULL;
}

void op_add(struct CPU *cpu, void *dst, void *src, int size){
    if(size==8){
        uint8_t *d=(uint8_t*)dst, *s=(uint8_t*)src;
        uint8_t res = *d + *s;
        cpu->flags.ZF = (res==0);
        cpu->flags.SF = (res>>7)&1;
        *d=res;
    } else {
        uint32_t *d=(uint32_t*)dst, *s=(uint32_t*)src;
        uint32_t res = *d + *s;
        update_add_flags(cpu, *d, *s, res);
        *d=res;
    }
}

void op_sub(struct CPU *cpu, void *dst, void *src, int size){
    if(size==8){
        uint8_t *d=(uint8_t*)dst, *s=(uint8_t*)src;
        uint8_t res = *d - *s;
        cpu->flags.ZF = (res==0);
        cpu->flags.SF = (res>>7)&1;
        *d=res;
    } else {
        uint32_t *d=(uint32_t*)dst, *s=(uint32_t*)src;
        uint32_t res = *d - *s;
        update_sub_flags(cpu, *d, *s, res);
        *d=res;
    }
}

void op_mov(struct CPU *cpu, void *dst, void *src, int size){
    if(size==8) *(uint8_t*)dst = *(uint8_t*)src;
    else *(uint32_t*)dst = *(uint32_t*)src;
}

void op_xor(struct CPU *cpu, void *dst, void *src, int size){
    if(size==8) *(uint8_t*)dst ^= *(uint8_t*)src;
    else *(uint32_t*)dst ^= *(uint32_t*)src;
    update_ZF_SF(cpu, *(uint32_t*)dst);
}

void op_cmp(struct CPU *cpu, void *dst, void *src, int size){
    if(size==8){
        uint8_t res = *(uint8_t*)dst - *(uint8_t*)src;
        cpu->flags.ZF = (res==0);
        cpu->flags.SF = (res>>7)&1;
    } else {
        uint32_t res = *(uint32_t*)dst - *(uint32_t*)src;
        update_sub_flags(cpu, *(uint32_t*)dst, *(uint32_t*)src, res);
    }
}

void op_and(struct CPU *cpu, void *dst, void *src, int size){
    if(size==8) *(uint8_t*)dst &= *(uint8_t*)src;
    else *(uint32_t*)dst &= *(uint32_t*)src;
    update_ZF_SF(cpu, *(uint32_t*)dst);
    cpu->flags.CF = 0;
    cpu->flags.OF = 0;
}

void op_or(struct CPU *cpu, void *dst, void *src, int size){
    if(size==8) *(uint8_t*)dst |= *(uint8_t*)src;
    else *(uint32_t*)dst |= *(uint32_t*)src;
    update_ZF_SF(cpu, *(uint32_t*)dst);
    cpu->flags.CF = 0;
    cpu->flags.OF = 0;
}

void cpu_step(struct CPU *cpu, uint8_t *memory, struct fake_process *proc) {
    uint8_t opcode = mem_read8(memory, cpu->eip);

    switch(opcode){
        HANDLE_MOV_IMM32
        MAKE_OP(0x00, op_add)
        MAKE_OP(0x88, op_mov)
        MAKE_OP(0x18, op_sub)
        MAKE_OP(0x30, op_xor)
        MAKE_OP(0x38, op_cmp)
        MAKE_OP(0x20, op_and)
        MAKE_OP(0x08,  op_or)

        case 0x90: cpu->eip+=1; break;
        case 0xE9: { int32_t rel = mem_read32(memory, cpu->eip+1); cpu->eip += rel+5; break; }
        case 0xEB: { int8_t rel = mem_read8(memory, cpu->eip+1); cpu->eip += rel+2; break; }

        case 0x50: push32(memory, cpu, cpu->eax.e); cpu->eip+=1; break;
        case 0x58: cpu->eax.e = pop32(memory, cpu); cpu->eip+=1; break;

        case 0xE8: { int32_t rel = mem_read32(memory, cpu->eip+1); call_rel32(memory, cpu, rel); break; }
        case 0xC3: ret(memory, cpu); break;

        case 0xF8: cpu->flags.CF=0; cpu->eip+=1; break;
        case 0xCC: cpu->eip += 1; break;
        case 0xCD: {
            uint8_t num = mem_read8(memory, cpu->eip+1);
            if(num==0x80){
                if(cpu->debug_mode) dbg_trace_syscall(cpu);
                if(cpu->eax.e==1 && proc) proc->alive=0;
                else kernel_handle_syscall(proc);
            }
            cpu->eip+=2;
            break;
        }
        case 0xF4: printf("Encerrando.\n"); exit(0); break;

        default:
            if(opcode==0x00) return;
            printf("Opcode desconhecido em EIP=0x%08X: 0x%02X\n", cpu->eip, opcode);
            exit(1);
    }
}
