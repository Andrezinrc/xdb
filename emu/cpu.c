#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "cpu.h"
#include "cpu_exec.h"
#include "cpu_ops.h"
#include "cpu_flags.h"
#include "mem.h"
#include "decode.h"
#include "kernel/kernel.h"
#include "dbg.h"

void cpu_init(struct CPU *cpu, uint32_t mem_size) {
    memset(cpu, 0, sizeof(struct CPU));
    cpu->esp.e = mem_size - 4;
    cpu->eip = 0;
}

void cpu_step(struct CPU *cpu, uint8_t *memory, struct fake_process *proc) {
    uint8_t opcode = mem_read8(memory, cpu->eip);
    cpu->last_opcode = opcode;

    switch(opcode){
        case 0x0F: {
            uint8_t subop = mem_read8(memory, cpu->eip + 1);
            switch(subop){
                case 0x31: { /* RDTSC */
                    static uint64_t tsc = 0x12345678;
                    
                    tsc = tsc * 6364136223846793005ULL + 1442695040888963407ULL;
                    
                    struct timespec ts;
                    clock_gettime(CLOCK_MONOTONIC, &ts);
                    tsc ^= ((uint64_t)ts.tv_sec << 32) | ts.tv_nsec;
                    
                    cpu->eax.e = (uint32_t)(tsc & 0xFFFFFFFF);
                    cpu->edx.e = (uint32_t)(tsc >> 32);
                    
                    cpu->eip += 2;
                    break;
                }
                case 0x82: { /* JB rel32 */
                    int32_t rel = mem_read32(memory, cpu->eip + 2);
                    if(cpu->flags.CF)
                        cpu->eip += rel + 6;
                    else
                        cpu->eip += 6;
                    break;
                }
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
                case 0x86: { /* JBE rel32 */
                    int32_t rel = mem_read32(memory, cpu->eip + 2);
                    if (cpu->flags.CF || cpu->flags.ZF)
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
        
        /* MOV reg8, imm8 */
        case 0xB0: case 0xB1: case 0xB2: case 0xB3:
        case 0xB4: case 0xB5: case 0xB6: case 0xB7: {
            uint8_t reg = opcode - 0xB0; 
            
            uint8_t imm = mem_read8(memory, cpu->eip + 1);
            
            uint8_t *reg_ptr = get_reg(cpu, reg, 8);
            *reg_ptr = imm;
            
            cpu->eip += 2;
            break;
        }
		
        /* MOV r/m8, imm8 */
        case 0xC6: {
            uint8_t modrm = mem_read8(memory, cpu->eip + 1);
            uint8_t mod, regop, rm;
            DECODE_MODRM(modrm, mod, regop, rm);
            
            uint8_t imm = mem_read8(memory, cpu->eip + 2);
            
            if (mod == 3) {
                // MOV reg8, imm8
                uint8_t *reg = get_reg(cpu, rm, 8);
                *reg = imm;
            } else if (mod == 0 && rm == 5) {
                // MOV [disp32], imm8
                uint32_t addr = mem_read32(memory, cpu->eip + 2);
                mem_write8(memory, addr, imm);
                cpu->eip += 6;
            } else {
                printf("0xC6 com memoria nao suportado\n");
                exit(1);
            }
            
            if (mod == 3) cpu->eip += 3;
            break;
        }

        HANDLE_INCDEC(0x40, ++)  // INC
        HANDLE_INCDEC(0x48, --)  // DEC
     		
        HANDLE_MOV(0x88)
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
        
        /* JG rel8 */
        case 0x7F: {
            int8_t rel = mem_read8(memory, cpu->eip + 1);
            if (!cpu->flags.ZF && (cpu->flags.SF == cpu->flags.OF))
                cpu->eip += rel + 2;
            else
                cpu->eip += 2;
            break;
        }
        
        /* JA rel8  */
        case 0x77: {
            int8_t rel = mem_read8(memory, cpu->eip + 1);
            if (!cpu->flags.CF && !cpu->flags.ZF)
                cpu->eip += rel + 2;
            else
                cpu->eip += 2;
            break;
        }
        
        HANDLE_PUSH(0x50) // PUSH
        HANDLE_POP(0x58) // POP
        
        /* SUB AL, imm8 */
        case 0x2C: {
            uint8_t imm = mem_read8(memory, cpu->eip + 1);
            op_sub(cpu, &cpu->eax.l, &imm, 8);
            cpu->eip += 2;
            break;
        }

        /* Grp1 Ev, Ib */
        case 0x80: {
            uint8_t modrm = mem_read8(memory, cpu->eip + 1);
            uint8_t mod, regop, rm;
            DECODE_MODRM(modrm, mod, regop, rm);
            
            uint8_t imm = mem_read8(memory, cpu->eip + 2);
        
            if (mod != 3) {
                printf("0x80 com memoria nao suportado\n");
                exit(1);
            }
            
            switch(regop) {
                case 0: op_add(cpu, get_reg(cpu, rm, 8), &imm, 8); break;
                case 1: op_or(cpu, get_reg(cpu, rm, 8), &imm, 8); break;
                case 4: op_and(cpu, get_reg(cpu, rm, 8), &imm, 8); break;
                case 5: op_sub(cpu, get_reg(cpu, rm, 8), &imm, 8); break;
                case 6: op_xor(cpu, get_reg(cpu, rm, 8), &imm, 8); break;
                case 7: op_cmp(cpu, get_reg(cpu, rm, 8), &imm, 8); break;
                default:
                    printf("0x80 subop desconhecido: %d\n", regop);
                    exit(1);
            }
            
            cpu->eip += 3;
            break;
        }
 
        /* Grp1 Ev, Ib */
        case 0x83: {
            uint8_t modrm = mem_read8(memory, cpu->eip + 1);
            uint8_t mod, regop, rm;
            DECODE_MODRM(modrm, mod, regop, rm);
            
            int8_t imm8 = (int8_t)mem_read8(memory, cpu->eip + 2);
            int32_t imm32 = (int32_t)imm8;
            
            if (mod!=3) {
                printf("0x83 com memoria nao suportado\n");
                exit(1);
            }
            
            switch(regop) {
                case 0: /* ADD */
                    op_add(cpu, get_reg(cpu, rm, 32), (uint8_t*)&imm32, 32);
                    break;
                case 1: /* OR */
                    op_or(cpu, get_reg(cpu, rm, 32), (uint8_t*)&imm32, 32);
                    break;
                case 2: /* ADC */
                    printf("0x83/2 ADC not implemented\n");
                    exit(1);
                case 3: /* SBB */
                    printf("0x83/3 SBB not implemented\n");
                    exit(1);
                case 4: /* AND */
                    op_and(cpu, get_reg(cpu, rm, 32), (uint8_t*)&imm32, 32);
                    break;
                case 5: /* SUB */
                    op_sub(cpu, get_reg(cpu, rm, 32), (uint8_t*)&imm32, 32);
                    break;
                case 6: /* XOR */
                    op_xor(cpu, get_reg(cpu, rm, 32), (uint8_t*)&imm32, 32);
                    break;
                case 7: /* CMP */
                    op_cmp(cpu, get_reg(cpu, rm, 32), (uint8_t*)&imm32, 32);
                    break;
                default:
                    printf("0x83 subop desconhecido: %d\n", regop);
                    exit(1);
            }
            
            cpu->eip += 3;
            break;
        }
        
        /* MOV AL, [addr] */
        case 0xA0: {
            uint32_t addr = mem_read32(memory, cpu->eip + 1);
            cpu->eax.l = mem_read8(memory, addr);
            cpu->eip += 5;
            break;
        }
        
        /* MOV [addr], AL */
        case 0xA2: {
            uint32_t addr = mem_read32(memory, cpu->eip + 1);
            mem_write8(memory, addr, cpu->eax.l);
            cpu->eip += 5;
            break;
        }
        
        /* MOVSB */
        case 0xA4: {
            uint8_t src_byte = mem_read8(memory, cpu->esi.e);
            
            mem_write8(memory, cpu->edi.e, src_byte);
            
            if (cpu->flags.DF == 0) {
                cpu->esi.e++;
                cpu->edi.e++;
            } else {
                cpu->esi.e--;
                cpu->edi.e--;
            }
            
            cpu->eip += 1;
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
                if(cpu->debug_mode) dbg_trace_syscall(cpu);
                
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
