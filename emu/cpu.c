/*
 * Intel 64 and IA-32 Architectures Software Developer's Manual
 * https://cdrdv2-public.intel.com/868139/325383-089-sdm-vol-2abcd.pdf
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "mem.h"
#include "decode.h"
#include "kernel/kernel.h"
#include "dbg.h"

void* get_reg(struct CPU *cpu, int index, int size){
    if(size == 8){
        switch(index){
            case 0: return &cpu->eax.l;  // AL
            case 1: return &cpu->ecx.l;  // CL
            case 2: return &cpu->edx.l;  // DL
            case 3: return &cpu->ebx.l;  // BL
            case 4: return &cpu->eax.h;  // AH
            case 5: return &cpu->ecx.h;  // CH
            case 6: return &cpu->edx.h;  // DH
            case 7: return &cpu->ebx.h;  // BH
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

#define DECODE_MODRM(modrm, mod, reg, rm) \
    do { \
        mod = (modrm) >> 6; \
        reg = ((modrm) >> 3) & 0x07; \
        rm  = (modrm) & 0x07; \
    } while(0) 

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
        uint8_t mod, reg, rm; \
        DECODE_MODRM(modrm, mod, reg, rm); \
        int has_sib = (mod != 3 && rm == 4); \
        if (mod == 0x3){ \
            OP_NAME(cpu, get_reg(cpu, rm, 8), get_reg(cpu, reg, 8), 8); \
        } else { \
            uint32_t mem_addr = modrm_mem_addr(cpu, memory, modrm); \
            uint8_t mem_val = mem_read8(memory, mem_addr); \
            OP_NAME(cpu, &mem_val, get_reg(cpu, reg, 8), 8); \
            mem_write8(memory, mem_addr, mem_val); \
        } \
        cpu->eip += 2 + (has_sib ? 1 : 0) + ((mod == 2 || (mod == 0 && rm == 5)) ? 4 : 0); \
        break; \
    } \
    case base+0x1: { /* reg32, modrm32 */ \
        uint8_t modrm = mem_read8(memory, cpu->eip+1); \
        uint8_t mod, reg, rm; \
        DECODE_MODRM(modrm, mod, reg, rm); \
        int has_sib = (mod != 3 && rm == 4); \
        if (mod == 0x3){ \
            OP_NAME(cpu, get_reg(cpu, rm, 32), get_reg(cpu, reg, 32), 32); \
        } else { \
            uint32_t mem_addr = modrm_mem_addr(cpu, memory, modrm); \
            uint32_t mem_val = mem_read32(memory, mem_addr); \
            OP_NAME(cpu, &mem_val, get_reg(cpu, reg, 32), 32); \
            mem_write32(memory, mem_addr, mem_val); \
        } \
        cpu->eip += 2 + (has_sib ? 1 : 0) + ((mod == 2 || (mod == 0 && rm == 5)) ? 4 : 0); \
        break; \
    } \
    case base+0x2: { /* modrm8, reg8 */ \
        uint8_t modrm = mem_read8(memory, cpu->eip+1); \
        uint8_t mod, reg, rm; \
        DECODE_MODRM(modrm, mod, reg, rm); \
        int has_sib = (mod != 3 && rm == 4); \
        if (mod == 0x3){ \
            OP_NAME(cpu, get_reg(cpu, reg, 8), get_reg(cpu, rm, 8), 8); \
        } else { \
            uint32_t mem_addr = modrm_mem_addr(cpu, memory, modrm); \
            uint8_t mem_val = mem_read8(memory, mem_addr); \
            uint8_t reg_val = *(uint8_t*)get_reg(cpu, reg, 8); \
            OP_NAME(cpu, &reg_val, &mem_val, 8); \
            mem_write8(memory, mem_addr, mem_val); \
        } \
        cpu->eip += 2 + (has_sib ? 1 : 0) + ((mod == 2 || (mod == 0 && rm == 5)) ? 4 : 0); \
        break; \
    } \
    case base+0x3: { /* modrm32, reg32 */ \
        uint8_t modrm = mem_read8(memory, cpu->eip+1); \
        uint8_t mod, reg, rm; \
        DECODE_MODRM(modrm, mod, reg, rm); \
        int has_sib = (mod != 3 && rm == 4); \
        if (mod == 0x3){ \
            OP_NAME(cpu, get_reg(cpu, reg, 32), get_reg(cpu, rm, 32), 32); \
        } else { \
            uint32_t mem_addr = modrm_mem_addr(cpu, memory, modrm); \
            uint32_t mem_val = mem_read32(memory, mem_addr); \
            uint32_t reg_val = *(uint32_t*)get_reg(cpu, reg, 32); \
            OP_NAME(cpu, &reg_val, &mem_val, 32); \
            mem_write32(memory, mem_addr, mem_val); \
        } \
        cpu->eip += 2 + (has_sib ? 1 : 0) + ((mod == 2 || (mod == 0 && rm == 5)) ? 4 : 0); \
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
    
#define HANDLE_MOV(base) \
    case base+0x0: { /* MOV reg8, modrm8 */ \
        uint8_t modrm = mem_read8(memory, cpu->eip+1); \
        uint8_t mod, reg, rm; \
        DECODE_MODRM(modrm, mod, reg, rm); \
        int has_sib = (mod != 3 && rm == 4); \
        if (mod == 0x3){ \
            op_mov(cpu, get_reg(cpu, rm, 8), get_reg(cpu, reg, 8), 8); \
        } else { \
            uint32_t mem_addr = modrm_mem_addr(cpu, memory, modrm); \
            uint8_t reg_val = *(uint8_t*)get_reg(cpu, reg, 8); \
            mem_write8(memory, mem_addr, reg_val); \
        } \
        cpu->eip += 2 + (has_sib ? 1 : 0) + ((mod == 2 || (mod == 0 && rm == 5)) ? 4 : 0); \
        break; \
    } \
    case base+0x1: { /* MOV reg32, modrm32 */ \
        uint8_t modrm = mem_read8(memory, cpu->eip+1); \
        uint8_t mod, reg, rm; \
        DECODE_MODRM(modrm, mod, reg, rm); \
        int has_sib = (mod != 3 && rm == 4); \
        if (mod == 0x3){ \
            op_mov(cpu, get_reg(cpu, rm, 32), get_reg(cpu, reg, 32), 32); \
        } else { \
            uint32_t mem_addr = modrm_mem_addr(cpu, memory, modrm); \
            uint32_t reg_val = *(uint32_t*)get_reg(cpu, reg, 32); \
            mem_write32(memory, mem_addr, reg_val); \
        } \
        cpu->eip += 2 + (has_sib ? 1 : 0) + ((mod == 2 || (mod == 0 && rm == 5)) ? 4 : 0); \
        break; \
    } \
    case base+0x2: { /* MOV modrm8, reg8 */ \
        uint8_t modrm = mem_read8(memory, cpu->eip+1); \
        uint8_t mod, reg, rm; \
        DECODE_MODRM(modrm, mod, reg, rm); \
        int has_sib = (mod != 3 && rm == 4); \
        if (mod == 0x3){ \
            op_mov(cpu, get_reg(cpu, reg, 8), get_reg(cpu, rm, 8), 8); \
        } else { \
            uint32_t mem_addr = modrm_mem_addr(cpu, memory, modrm); \
            uint8_t mem_val = mem_read8(memory, mem_addr); \
            op_mov(cpu, get_reg(cpu, reg, 8), &mem_val, 8); \
        } \
        cpu->eip += 2 + (has_sib ? 1 : 0) + ((mod == 2 || (mod == 0 && rm == 5)) ? 4 : 0); \
        break; \
    } \
    case base+0x3: { /* MOV modrm32, reg32 */ \
        uint8_t modrm = mem_read8(memory, cpu->eip+1); \
        uint8_t mod, reg, rm; \
        DECODE_MODRM(modrm, mod, reg, rm); \
        int has_sib = (mod != 3 && rm == 4); \
        if (mod == 0x3){ \
            op_mov(cpu, get_reg(cpu, reg, 32), get_reg(cpu, rm, 32), 32); \
        } else { \
            uint32_t mem_addr = modrm_mem_addr(cpu, memory, modrm); \
            uint32_t mem_val = mem_read32(memory, mem_addr); \
            op_mov(cpu, get_reg(cpu, reg, 32), &mem_val, 32); \
        } \
        cpu->eip += 2 + (has_sib ? 1 : 0) + ((mod == 2 || (mod == 0 && rm == 5)) ? 4 : 0); \
        break; \
    }
   
#define HANDLE_MOV_REG8_IMM_ALL() \
    case 0xB0: case 0xB1: case 0xB2: case 0xB3: \
    case 0xB4: case 0xB5: case 0xB6: case 0xB7: { \
        uint8_t imm = mem_read8(memory, cpu->eip + 1); \
        int reg = opcode & 7; \
        uint8_t *r = (uint8_t*)get_reg(cpu, reg, 8); \
        if (r) *r = imm; \
        cpu->eip += 2; \
        break; \
    }
 
#define HANDLE_MOV_REG8_MEM_ALL() \
    case 0xA0: case 0xA1: case 0xA2: case 0xA3: \
    case 0xA4: case 0xA5: case 0xA6: case 0xA7: { \
        uint32_t addr = mem_read32(memory, cpu->eip + 1); \
        int reg = opcode & 7; \
        uint8_t *r = (uint8_t*)get_reg(cpu, reg, 8); \
        if (r) *r = mem_read8(memory, addr); \
        cpu->eip += 5; \
        break; \
    }
    
#define HANDLE_MOV_MEM_REG8_ALL() \
    case 0xA8: case 0xA9: case 0xAA: case 0xAB: \
    case 0xAC: case 0xAD: case 0xAE: case 0xAF: { \
        uint32_t addr = mem_read32(memory, cpu->eip + 1); \
        int reg = opcode & 7; \
        uint8_t *r = (uint8_t*)get_reg(cpu, reg, 8); \
        if (r) mem_write8(memory, addr, *r); \
        cpu->eip += 5; \
        break; \
    }
    
#define HANDLE_INCDEC(base, OP) \
    case base+0x0: { cpu->eax.e OP; update_ZF_SF(cpu, cpu->eax.e); cpu->eip += 1; break; } \
    case base+0x1: { cpu->ecx.e OP; update_ZF_SF(cpu, cpu->ecx.e); cpu->eip += 1; break; } \
    case base+0x2: { cpu->edx.e OP; update_ZF_SF(cpu, cpu->edx.e); cpu->eip += 1; break; } \
    case base+0x3: { cpu->ebx.e OP; update_ZF_SF(cpu, cpu->ebx.e); cpu->eip += 1; break; } \
    case base+0x4: { cpu->esp.e OP; update_ZF_SF(cpu, cpu->esp.e); cpu->eip += 1; break; } \
    case base+0x5: { cpu->ebp.e OP; update_ZF_SF(cpu, cpu->ebp.e); cpu->eip += 1; break; } \
    case base+0x6: { cpu->esi.e OP; update_ZF_SF(cpu, cpu->esi.e); cpu->eip += 1; break; } \
    case base+0x7: { cpu->edi.e OP; update_ZF_SF(cpu, cpu->edi.e); cpu->eip += 1; break; }

#define HANDLE_PUSH(base) \
    case base+0x0: push32(memory, cpu, cpu->eax.e); cpu->eip += 1; break; \
    case base+0x1: push32(memory, cpu, cpu->ecx.e); cpu->eip += 1; break; \
    case base+0x2: push32(memory, cpu, cpu->edx.e); cpu->eip += 1; break; \
    case base+0x3: push32(memory, cpu, cpu->ebx.e); cpu->eip += 1; break; \
    case base+0x4: push32(memory, cpu, cpu->esp.e); cpu->eip += 1; break; \
    case base+0x5: push32(memory, cpu, cpu->ebp.e); cpu->eip += 1; break; \
    case base+0x6: push32(memory, cpu, cpu->esi.e); cpu->eip += 1; break; \
    case base+0x7: push32(memory, cpu, cpu->edi.e); cpu->eip += 1; break;

#define HANDLE_POP(base) \
    case base+0x0: cpu->eax.e = pop32(memory, cpu); cpu->eip += 1; break; \
    case base+0x1: cpu->ecx.e = pop32(memory, cpu); cpu->eip += 1; break; \
    case base+0x2: cpu->edx.e = pop32(memory, cpu); cpu->eip += 1; break; \
    case base+0x3: cpu->ebx.e = pop32(memory, cpu); cpu->eip += 1; break; \
    case base+0x4: cpu->esp.e = pop32(memory, cpu); cpu->eip += 1; break; \
    case base+0x5: cpu->ebp.e = pop32(memory, cpu); cpu->eip += 1; break; \
    case base+0x6: cpu->esi.e = pop32(memory, cpu); cpu->eip += 1; break; \
    case base+0x7: cpu->edi.e = pop32(memory, cpu); cpu->eip += 1; break;

#define HANDLE_SUB_REG8_IMM_ALL() \
    case 0x80: case 0x81: case 0x82: case 0x83: \
    case 0x84: case 0x85: case 0x86: case 0x87: { \
        uint8_t imm = mem_read8(memory, cpu->eip + 2); \
        int reg = opcode & 7; \
        uint8_t *r = (uint8_t*)get_reg(cpu, reg, 8); \
        if (r) op_sub(cpu, r, &imm, 8); \
        cpu->eip += 3; \
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

void op_mov(struct CPU *cpu, void *dst, void *src, int size){
    if(size==8) *(uint8_t*)dst = *(uint8_t*)src;
    else *(uint32_t*)dst = *(uint32_t*)src;
}

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
        case 0x0F: {
            uint8_t subop = mem_read8(memory, cpu->eip + 1);
            switch(subop){
                case 0x84: { /* JZ/JE rel32 */
                    int32_t rel = mem_read32(memory, cpu->eip + 2);
                    if(cpu->flags.ZF)
                        cpu->eip += rel + 6;
                    else
                        cpu->eip += 6;
                    break;
                }
                case 0x85: { /* JNZ/JNE rel32 */
                    int32_t rel = mem_read32(memory, cpu->eip + 2);
                    if(!cpu->flags.ZF)
                        cpu->eip += rel + 6;
                    else
                        cpu->eip += 6;
                    break;
                }
                default:
                    printf("Opcode 0F desconhecido: 0x%02X em EIP=0x%08X\n", subop, cpu->eip);
                    exit(1);
            }
            break;
        }

        HANDLE_MOV_IMM32
		
        HANDLE_INCDEC(0x40, ++)  // INC
        HANDLE_INCDEC(0x48, --)  // DEC
        
        /* INC/DEC 8-bit */
        case 0xFE: {
            uint8_t subop = mem_read8(memory, cpu->eip + 1);
            if(subop == 0xC0) { // INC AL
                cpu->eax.l++;
                update_ZF_SF(cpu, cpu->eax.l);
                cpu->eip += 2;
                break;
            }
            if(subop == 0xC8) { // DEC AL
                cpu->eax.l--;
                update_ZF_SF(cpu, cpu->eax.l);
                cpu->eip += 2;
                break;
            }
            printf("Subopcode FE desconhecido: 0x%02X\n", subop);
            exit(1);
        }
     		
        HANDLE_MOV(0x88)
        
        HANDLE_MOV_REG8_IMM_ALL() 
        HANDLE_MOV_REG8_MEM_ALL()
        HANDLE_MOV_MEM_REG8_ALL()
	    
        MAKE_OP(0x00, op_add)
        MAKE_OP(0x18, op_sub)
        MAKE_OP(0x30, op_xor)
        MAKE_OP(0x38, op_cmp)
        MAKE_OP(0x20, op_and)
        MAKE_OP(0x08,  op_or)

        /* NOP */
        case 0x90: cpu->eip+=1; break;
        
        /* JMP rel32 */
        case 0xE9: { 
            int32_t rel = mem_read32(memory, cpu->eip+1); 
            cpu->eip += rel+5; 
            break; 
        }
        
        /* JMP rel8 */
        case 0xEB: { 
            int8_t rel = mem_read8(memory, cpu->eip+1); 
            cpu->eip += rel+2; 
            break; 
        }

        /* JE rel8 */
        case 0x74: { 
            int8_t rel = mem_read8(memory, cpu->eip + 1);
            if(cpu->flags.ZF)
                cpu->eip += rel + 2;
            else
                cpu->eip += 2;
            break;
        }
        
        /* JNE rel8 */
        case 0x75: { 
            int8_t rel = mem_read8(memory, cpu->eip + 1);
            if(!cpu->flags.ZF)
                cpu->eip += rel + 2;
            else
                cpu->eip += 2;
            break;
        } 
        
        HANDLE_PUSH(0x50) // PUSH
        HANDLE_POP(0x58) // POP
        
        HANDLE_SUB_REG8_IMM_ALL() /* SUB reg8, imm8 */
        
        /* SUB AL, imm8 */
        case 0x2C: {
            uint8_t imm = mem_read8(memory, cpu->eip + 1);
            op_sub(cpu, &cpu->eax.l, &imm, 8);
            cpu->eip += 2;
            break;
        }
        
        /* CALL rel32 */
        case 0xE8: { 
            int32_t rel = mem_read32(memory, cpu->eip+1); 
            call_rel32(memory, cpu, rel); 
            break; 
        }
        
        /* RET */
        case 0xC3: ret(memory, cpu); break;

        /* CLC */
        case 0xF8: cpu->flags.CF=0; cpu->eip+=1; break;
        
        /* INT3 */
        case 0xCC: cpu->eip += 1; break;
        
        /* INT */
        case 0xCD: {
            uint8_t num = mem_read8(memory, cpu->eip+1);
            if(num==0x80){
                dbg_trace_syscall(cpu);
                
                if(cpu->eax.e==1 && proc) proc->alive=0;
                else kernel_handle_syscall(proc);
            }
            cpu->eip+=2;
            break;
        }
        
        /* HLT */
        case 0xF4: printf("Encerrando.\n"); exit(0); break;

        default:
            if(opcode==0x00) return;
            printf("Opcode desconhecido em EIP=0x%08X: 0x%02X\n", cpu->eip, opcode);
            exit(1);
    }
}
