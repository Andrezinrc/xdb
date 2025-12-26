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
    
    for (int i = 0; i < to_show; i++) {
        printf("%02X ", mem[addr + i]);
    }
    
    // Padding fixo de 20 caracteres no total
    int spaces = 20 - (to_show * 3);
    for (int i = 0; i < spaces; i++) {
        printf(" ");
    }
    
    if (count > max_to_show) {
        printf("...");  // Indica que tem mais bytes
    }
}

void disassemble(uint8_t *memory, uint32_t eip) {
    uint8_t op = memory[eip];
    printf("%08X: ", eip);
    
    int instr_len = 0;

    switch (op) {
        // MOV reg32, imm32
        case 0xB8: case 0xB9: case 0xBA: case 0xBB:
        case 0xBC: case 0xBD: case 0xBE: case 0xBF: {
            instr_len = 5;
            print_bytes(memory, eip, 5);
            printf("    ");
            
            uint8_t reg = op - 0xB8;
            uint32_t imm = mem_read32(memory, eip + 1);
            
            if (imm <= 255 && imm != 0x80 && imm != 0xCD) {
                printf("mov %s, %d\n", reg32_name(reg), imm);
            } else if (imm == 0x80 || imm == 0xCD) {
                printf("mov %s, 0x%02X\n", reg32_name(reg), imm);
            } else {
                printf("mov %s, 0x%08X\n", reg32_name(reg), imm);
            }
            break;
        }

        // ADD r/m32, r32 / SUB r/m32, r32 / CMP r/m32, r32
        case 0x01: case 0x29: case 0x39: {
            instr_len = 2;
            print_bytes(memory, eip, 2);
            printf("    ");
            
            const char *mnemonic = (op == 0x01) ? "add" : 
                                  (op == 0x29) ? "sub" : "cmp";
            
            uint8_t modrm = memory[eip + 1];
            uint8_t reg, rm;
            
            if (modrm_reg_reg(modrm, &reg, &rm)) {
                printf("%s %s, %s\n", mnemonic, reg32_name(rm), reg32_name(reg));
            } else {
                printf("%s mem, %s\n", mnemonic, reg32_name(reg));
            }
            break;
        }

        // XOR r/m32, r32
        case 0x31: {
            instr_len = 2;
            print_bytes(memory, eip, 2);
            printf("    ");
            
            uint8_t modrm = memory[eip + 1];
            uint8_t reg, rm;
            
            if (modrm_reg_reg(modrm, &reg, &rm)) {
                printf("xor %s, %s\n", reg32_name(rm), reg32_name(reg));
            } else {
                printf("xor mem, %s\n", reg32_name(reg));
            }
            break;
        }

        // MOV r/m32, r32
        case 0x89: {
            instr_len = 2;
            print_bytes(memory, eip, 2);
            printf("    ");
            
            uint8_t modrm = memory[eip + 1];
            uint8_t reg, rm;
            
            if (modrm_reg_reg(modrm, &reg, &rm)) {
                printf("mov %s, %s\n", reg32_name(rm), reg32_name(reg));
            } else {
                printf("mov mem, %s\n", reg32_name(reg));
            }
            break;
        }

        // MOV r32, r/m32
        case 0x8B: {
            instr_len = 2;
            print_bytes(memory, eip, 2);
            printf("    ");
            
            uint8_t modrm = memory[eip + 1];
            uint8_t reg, rm;
            
            if (modrm_reg_reg(modrm, &reg, &rm)) {
                printf("mov %s, %s\n", reg32_name(reg), reg32_name(rm));
            } else {
                printf("mov %s, [mem]\n", reg32_name(reg));
            }
            break;
        }

        // ADD EAX, imm32
        case 0x05: {
            instr_len = 5;
            print_bytes(memory, eip, 5);
            printf("    ");
            
            uint32_t imm = mem_read32(memory, eip + 1);
            
            if (imm <= 255) {
                printf("add eax, %d\n", imm);
            } else {
                printf("add eax, 0x%08X\n", imm);
            }
            break;
        }

        // INC reg32 / DEC reg32
        case 0x40: case 0x41: case 0x42: case 0x43:
        case 0x44: case 0x45: case 0x46: case 0x47:
        case 0x48: case 0x49: case 0x4A: case 0x4B:
        case 0x4C: case 0x4D: case 0x4E: case 0x4F: {
            instr_len = 1;
            print_bytes(memory, eip, 1);
            printf("    ");
            
            uint8_t reg = op - (op < 0x48 ? 0x40 : 0x48);
            const char *mnemonic = (op < 0x48) ? "inc" : "dec";
            printf("%s %s\n", mnemonic, reg32_name(reg));
            break;
        }

        // ADD/SUB/CMP r/m32, imm8
        case 0x83: {
            instr_len = 3;
            print_bytes(memory, eip, 3);
            printf("    ");
            
            uint8_t modrm = memory[eip + 1];
            int8_t imm = (int8_t)memory[eip + 2];
            
            uint8_t subop = (modrm >> 3) & 7;
            
            const char *mnemonic = "???";
            switch (subop) {
                case 0: mnemonic = "add"; break;
                case 5: mnemonic = "sub"; break;
                case 7: mnemonic = "cmp"; break;
                default: break;
            }
            
            uint8_t reg, rm;
            if (modrm_reg_reg(modrm, &reg, &rm)) {
                printf("%s %s, %d\n", mnemonic, reg32_name(rm), imm);
            } else {
                printf("%s mem, %d\n", mnemonic, imm);
            }
            break;
        }

        case 0x74: {  // JE/JZ
            instr_len = 2;
            print_bytes(memory, eip, 2);
            printf("    ");
            
            int8_t rel = (int8_t)memory[eip + 1];
            printf("je 0x%08X\n", eip + 2 + rel);
            break;
        }
        case 0x75: {  // JNE/JNZ
            instr_len = 2;
            print_bytes(memory, eip, 2);
            printf("    ");
            
            int8_t rel = (int8_t)memory[eip + 1];
            printf("jne 0x%08X\n", eip + 2 + rel);
            break;
        }
        case 0xEB: {  // JMP curto
            instr_len = 2;
            print_bytes(memory, eip, 2);
            printf("    ");
            
            int8_t rel = (int8_t)memory[eip + 1];
            printf("jmp 0x%08X\n", eip + 2 + rel);
            break;
        }

        case 0xCD: {
            instr_len = 2;
            print_bytes(memory, eip, 2);
            printf("    ");
            
            uint8_t int_num = memory[eip + 1];
            printf("int 0x%02X\n", int_num);
            break;
        }

        case 0xCC: 
            instr_len = 1;
            print_bytes(memory, eip, 1);
            printf("    int3\n"); 
            break;
            
        case 0x90: 
            instr_len = 1;
            print_bytes(memory, eip, 1);
            printf("    nop\n"); 
            break;
            
        case 0xF8: 
            instr_len = 1;
            print_bytes(memory, eip, 1);
            printf("    clc\n"); 
            break;
            
        case 0xF4: 
            instr_len = 1;
            print_bytes(memory, eip, 1);
            printf("    hlt\n"); 
            break;
		
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