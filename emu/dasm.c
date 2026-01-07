#include "dasm.h"
#include "decode.h"
#include "mem.h"
#include <stdio.h>

static const char *reg32_name(uint8_t reg) {
    static const char *names[8] = {
        "eax", "ecx", "edx", "ebx",
        "esp", "ebp", "esi", "edi"
    };
    return (reg < 8) ? names[reg] : "???";
}

static void print_bytes(uint8_t *mem, uint32_t addr, int count) {
    // Imprime até 6 bytes
    int max_to_show = 6;
    int to_show = (count > max_to_show) ? max_to_show : count;
    
    for (int i = 0; i < to_show; i++)
        printf("%02X ", mem[addr + i]);
		
    // Padding fixo de 20 caracteres no total
    int spaces = 20 - (to_show * 3);
    for (int i = 0; i < spaces; i++)
        printf(" ");
    
    if (count > max_to_show)
        printf("..."); // Indica que tem mais bytes
}

static void print_modrm(uint8_t *memory, uint32_t eip, const char *mnemonic) {
    int instr_len = 2;
    print_bytes(memory, eip, instr_len);
    uint8_t modrm = memory[eip + 1];
    uint8_t reg, rm;
    if(modrm_reg_reg(modrm, &reg, &rm))
        printf("%s %s, %s\n", mnemonic, reg32_name(rm), reg32_name(reg));
    else
        printf("%s mem, %s\n", mnemonic, reg32_name(reg));
}

void disassemble(uint8_t *memory, uint32_t eip) {
    uint8_t op = memory[eip];
    printf("%08X: ", eip);
	
    int instr_len = 1;
	
    if(op >= 0xB8 && op <= 0xBF) { // MOV reg32, imm32
        instr_len = 5;
        print_bytes(memory, eip, instr_len);
        uint8_t reg = op - 0xB8;
        uint32_t imm = mem_read32(memory, eip + 1);
        if (imm <= 255)
            printf("    mov %s, %u\n", reg32_name(reg), imm);
        else
            printf("    mov %s, 0x%08X\n", reg32_name(reg), imm);
        return;
    }

    switch(op) {

        // ADD/SUB/XOR/CMP reg-reg via MAKE_OP
        case 0x00: case 0x01: print_modrm(memory, eip, "add"); break;
        case 0x18: case 0x19: print_modrm(memory, eip, "sub"); break;
        case 0x30: case 0x31: print_modrm(memory, eip, "xor"); break;
        case 0x38: case 0x39: print_modrm(memory, eip, "cmp"); break;

        // MOV r/m8, r8 ou r/m32, r32
        case 0x88: print_modrm(memory, eip, "mov"); break;
        case 0x89: print_modrm(memory, eip, "mov"); break;

        // NOP
        case 0x90:
            print_bytes(memory, eip, 1);
            printf("    nop\n");
            break;

        // JMP rel32
        case 0xE9: {
            instr_len = 5;
            print_bytes(memory, eip, instr_len);
            int32_t rel = mem_read32(memory, eip + 1);
            printf("    jmp 0x%08X\n", eip + instr_len + rel);
            break;
        }

        // JMP rel8
        case 0xEB: {
            instr_len = 2;
            print_bytes(memory, eip, instr_len);
            int8_t rel = (int8_t)memory[eip + 1];
            printf("    jmp 0x%08X\n", eip + instr_len + rel);
            break;
        }

        // PUSH/POP EAX
        case 0x50: print_bytes(memory, eip, 1); printf("    push eax\n"); break;
        case 0x58: print_bytes(memory, eip, 1); printf("    pop eax\n"); break;

        // CALL rel32
        case 0xE8: {
            instr_len = 5;
            print_bytes(memory, eip, instr_len);
            int32_t rel = mem_read32(memory, eip + 1);
            printf("    call 0x%08X\n", eip + instr_len + rel);
            break;
        }

        // RET
        case 0xC3: print_bytes(memory, eip, 1); printf("    ret\n"); break;

        // CLC
        case 0xF8: print_bytes(memory, eip, 1); printf("    clc\n"); break;

        // INT3
        case 0xCC: print_bytes(memory, eip, 1); printf("    int3\n"); break;

        // INT n
        case 0xCD: {
            instr_len = 2;
            print_bytes(memory, eip, instr_len);
            uint8_t num = memory[eip + 1];
            printf("    int 0x%02X\n", num);
            break;
        }

        // HLT
        case 0xF4: print_bytes(memory, eip, 1); printf("    hlt\n"); break;

        default:
            if(op == 0x00) {
                return;
            } else {
                instr_len = 1;
                print_bytes(memory, eip, 1);
                printf("    Opcode desconhecido: 0x%02X\n", op);
                break;
            }
    }
}
