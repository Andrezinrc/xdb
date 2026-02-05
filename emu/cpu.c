#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cpu.h"
#include "cpu_exec.h"
#include "cpu_flags.h"
#include "cpu_ops.h"
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

    switch(opcode){
        /* MOV r32, imm32 */
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

        HANDLE_INCDEC(0x40, ++)  // INC
        HANDLE_INCDEC(0x48, --)  // DEC
     		
        HANDLE_MOV(0x88)     /* MOV r/m,r */
        MAKE_OP(0x00, op_add) /* ADD r/m,r / AL/EAX,imm */
        MAKE_OP(0x18, op_sub) /* SUB r/m,r / AL/EAX,imm */
        /* SUB AL, imm8 */
        case 0x2C: {
            uint8_t imm = mem_read8(memory, cpu->eip + 1);
            op_sub(cpu, &cpu->eax.l, &imm, 8);
            cpu->eip += 2;
            break;
        }
        MAKE_OP(0x30, op_xor) /* XOR r/m,r / AL/EAX,imm */
        MAKE_OP(0x38, op_cmp) /* CMP r/m,r / AL/EAX,imm */
        MAKE_OP(0x20, op_and) /* AND r/m,r / AL/EAX,imm */
        MAKE_OP(0x08, op_or)  /* OR  r/m,r / AL/EAX,imm */

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
        
        HANDLE_PUSH(0x50) // PUSH r32
        HANDLE_POP(0x58)  // POP r32
        
        HANDLE_MOV_MOFFS(0xA0, eax, 8, true)   // MOV AL, [addr]
        HANDLE_MOV_MOFFS(0xA1, eax, 32, true)  // MOV EAX, [addr]
        HANDLE_MOV_MOFFS(0xA2, eax, 8, false)  // MOV [addr], AL
        HANDLE_MOV_MOFFS(0xA3, eax, 32, false) // MOV [addr], EAX
 
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
