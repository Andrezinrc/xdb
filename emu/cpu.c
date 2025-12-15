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
#include "memory.h"

uint32_t *get_reg32(struct CPU *cpu, int index) {
    switch(index) {
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
    uint32_t sa = (uint32_t)a;
    uint32_t sb = (uint32_t)b;
    uint32_t sr = (uint32_t)res;
}


void update_sub_flags(struct CPU *cpu, uint32_t a, uint32_t b, uint32_t res) {
    update_ZF_SF(cpu, res);
    cpu->flags.CF = (a < b);
    int32_t sa = (int32_t)a;
    int32_t sb = (int32_t)b;
    int32_t sr = (int32_t)res;
    cpu->flags.OF = (((sa ^ sb) & (sa ^ sr)) < 0);
}


void cpu_step(struct CPU *cpu, uint8_t *memory) {
    uint8_t opcode = mem_read8(memory, cpu->eip);
   
    /* Opcodes MOV r32, imm32 */
    if(opcode >= 0xB8 && opcode <= 0xBF){
        uint32_t imm = mem_read32(memory, cpu->eip + 1);
        uint32_t *reg = get_reg32(cpu, opcode - 0xB8);
        if (reg) { *reg = imm; }
        cpu->eip += 5;
    } else {
        switch (opcode) {
		     case 0x89: { // MOV r/m32, r32
                uint8_t modrm = mem_read8(memory, cpu->eip + 1);
                if(modrm == 0xC1){
                    cpu->ecx.e = cpu->eax.e;
                    cpu->eip += 2;
                    break;
                } else {
                    printf("MOV com modrm nao suportado. %02X\n", modrm);
                    exit(1);
                }
            }
        
            case 0x05: { // ADD EAX, imm32
                uint32_t imm = mem_read32(memory, cpu->eip + 1);
                uint32_t a = cpu->eax.e;
                uint32_t res = a + imm;
                cpu->eax.e = res;
                update_ZF_SF(cpu, res);
                cpu->eip += 5;
                break;
            }
        
            case 0x01: { // ADD r/m32, r32
                uint8_t modrm = mem_read8(memory, cpu->eip + 1);
                if(modrm == 0xC8) {
                    uint32_t a = cpu->eax.e;
                    uint32_t b = cpu->ecx.e;
                    uint32_t res = a + b;
                    update_add_flags(cpu, a, b, res);
                    cpu->eax.e = res;
                    cpu->eip += 2;
                } else {
                    printf("modrm não suportado para opcode 0x01 em EIP=0x%X: 0x%02X\n", cpu->eip, modrm);
                    exit(1);
                }
                break;
            }
        
            case 0x29: { // SUB r/m32, r32
                uint8_t modrm = mem_read8(memory, cpu->eip + 1);
                if(modrm == 0xC8){
                    uint32_t a = cpu->eax.e;
                    uint32_t b = cpu->ecx.e;
                    uint32_t res = a - b;
                    update_sub_flags(cpu, a, b, res);
                    cpu->eax.e = res;
                    cpu->eip += 2;
                } else {
                    printf("modrm não suportado para SUB em EIP=0x%X: 0x%02X\n", cpu->eip, modrm);
                    exit(1);
                }
                break;
            }
        
            case 0x40: { // INC EAX
                uint32_t res = cpu->eax.e + 1;
                update_ZF_SF(cpu, res);
                cpu->eax.e = res;
                cpu->eip += 1;
                break;
            }
        
            case 0x48: { // DEC EAX
                uint32_t res = cpu->eax.e - 1;
                update_ZF_SF(cpu, res);
                cpu->eax.e = res;
                cpu->eip += 1;
                break;
            }
        
        
        
            case 0x39: { // CMP EAX, ECX
                uint8_t modrm = mem_read8(memory, cpu->eip + 1);
                if(modrm == 0xC8){
                    uint32_t a = cpu->eax.e;
                    uint32_t b = cpu->ecx.e;
                    uint32_t res = a - b;
                    update_sub_flags(cpu, a, b, res);
                    cpu->eip += 2;
                } else {
                    printf("modrm não suportado para CMP\n");
                    exit(1);
                }
                break;
            }
        
        
            case 0x74: { // JE/JZ rel8
                uint8_t offset = mem_read8(memory, cpu->eip + 1);
                if(cpu->flags.ZF) cpu->eip += offset + 2;
                else cpu->eip += 2;
                break;
            }
        
            case 0x75: { // JNE/JNZ rel8
                uint8_t offset = mem_read8(memory, cpu->eip + 1);
                if(!cpu->flags.ZF) cpu->eip += offset + 2;
                else cpu->eip += 2;
                break;
            }
        
            case 0x50: { // PUSH EAX
                push32(memory, cpu, cpu->eax.e);
                cpu->eip += 1;
                break;
            }
        
            case 0x58: { // POP EAX
                cpu->eax.e = pop32(memory, cpu);
                cpu->eip += 1;
                break;
            }
        
            case 0xE8: { // CALL rel32
                int32_t rel = mem_read32(memory, cpu->eip + 1);
                call_rel32(memory, cpu, rel);
                break;
            }
        
            case 0xC3: { // RET
                ret(memory, cpu);
                break;
            }
        
            case 0x8B: { // MOV r32, r/m32
                uint8_t modrm = mem_read8(memory, cpu->eip + 1);
                if(modrm == 0xC1){ // mov eax, ecx
                    cpu->eax.e = cpu->ecx.e;
                    cpu->eip += 2;
                } else if(modrm == 0xC8){ // mov ecx, eax
                    cpu->ecx.e = cpu->eax.e;
                    cpu->eip += 2;
                } else {
                    printf("MOV (8B) modrm nao suportado: %02X\n", modrm);
                    exit(1);
                }
                break;
            }
        
        
            case 0x31: { // XOR r/m32, r32
                uint8_t modrm = mem_read8(memory, cpu->eip + 1);
                if (modrm == 0xC0) { // xor eax, eax
                    cpu->eax.e = 0;
                    update_ZF_SF(cpu, 0);
                    cpu->eip += 2;
                } else if(modrm == 0xC9) { // xor ecx, ecx
                    cpu->ecx.e = 0;
                    update_ZF_SF(cpu, 0);
                    cpu->eip += 2;
                } else {
                    printf("XOR nao suportado: %02X\n", modrm);
                    exit(1);
                }
                break;
            }
        
            case 0x83: {
                uint8_t modrm = mem_read8(memory, cpu->eip + 1);
                int8_t imm = mem_read8(memory, cpu->eip + 2);
                switch(modrm){
                    case 0xC0: { // add eax, imm8
                        uint32_t a = cpu->eax.e;
                        uint32_t res = a + imm;
                        update_add_flags(cpu, a, imm, res);
                        cpu->eax.e = res;
                        cpu->eip += 3;
                        break;
                    }
               
                    case 0xE8: { // sub eax, imm8
                        uint32_t a = cpu->eax.e;
                        uint32_t res = a - imm;
                        update_sub_flags(cpu, a, imm, res);
                        cpu->eax.e = res;
                        cpu->eip += 3;
                        break;
                    }
               
               
                    case 0xF8: { // cmp eax, imm8
                        uint32_t a = cpu->eax.e;
                        uint32_t res = a - imm;
                        update_sub_flags(cpu, a, imm, res);
                        cpu->eip += 3;
                        break;
                    }
               
                    default:
                        printf("83 modrm nao suportado: %02X\n", modrm);
                        exit(1);
                }
                break;
            }
        
            case 0xEB: { // JMP rel8
                int8_t rel = mem_read8(memory, cpu->eip + 1);
                cpu->eip += rel + 2;
                break;
            }
        
            case 0x90: { // NOP
                cpu->eip += 1;
                break;
            }
            case 0xF8: { // CLC
                cpu->flags.CF = 0;
                cpu->eip += 1;
                break;
            }
	   

            case 0xCD: // int
                cpu->eip += 2;
                break;
   
            case 0xF4: {
                printf("Encerrando.\n");
                exit(1);
            }
       
       
            default:
                printf("Opcode desconhecido em EIP=0x%08X: 0x%02X\n",
                   cpu->eip, opcode);
                exit(1);
        }
    }
}
