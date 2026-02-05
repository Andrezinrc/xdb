#pragma once

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
 
#define HANDLE_MOV_MOFFS(base, reg_name, size, to_acc) \
    case base: { \
        uint32_t addr = mem_read32(memory, cpu->eip + 1); \
        if(to_acc) { \
            if(size == 8) cpu->reg_name.l = mem_read8(memory, addr); \
            else cpu->reg_name.e = mem_read32(memory, addr); \
        } else { \
            if(size == 8) mem_write8(memory, addr, cpu->reg_name.l); \
            else mem_write32(memory, addr, cpu->reg_name.e); \
        } \
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
