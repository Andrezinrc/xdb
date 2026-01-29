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

static const char *reg8_name(uint8_t reg) {
    static const char *names[8] = {
        "al", "cl", "dl", "bl",
        "ah", "ch", "dh", "bh"
    };
    return (reg < 8) ? names[reg] : "???";
}

static void print_bytes(uint8_t *mem, uint32_t addr, int count) {
    int max_to_show = 6;
    int to_show = (count > max_to_show) ? max_to_show : count;
    
    for (int i = 0; i < to_show; i++)
        printf("%02X ", mem[addr + i]);
		
    int spaces = 20 - (to_show * 3);
    for (int i = 0; i < spaces; i++)
        printf(" ");
    
    if (count > max_to_show)
        printf("...");
}

static void print_modrm_8(uint8_t *memory, uint32_t eip, const char *mnemonic, bool reverse) {
    uint8_t modrm = memory[eip + 1];
    uint8_t mod, reg, rm;
    DECODE_MODRM(modrm, mod, reg, rm);
    
    int instr_len = 2;
    bool has_disp32 = false;
    uint32_t disp32 = 0;
    
    if (mod == 0 && rm == 5) {
        instr_len = 6;
        has_disp32 = true;
        disp32 = mem_read32(memory, eip + 2);
    }

    print_bytes(memory, eip, instr_len);
    
    if (mod==3) {
        if (!reverse)
            printf("    %s %s, %s\n", mnemonic, reg8_name(rm), reg8_name(reg));
        else
            printf("    %s %s, %s\n", mnemonic, reg8_name(reg), reg8_name(rm));
    } 
    else if (has_disp32) {
        if (!reverse)
            printf("    %s byte [0x%08X], %s\n", mnemonic, disp32, reg8_name(reg));
        else
            printf("    %s %s, byte [0x%08X]\n", mnemonic, reg8_name(reg), disp32);
    }
    else {
        if (!reverse)
            printf("    %s byte [mem], %s\n", mnemonic, reg8_name(reg));
        else
            printf("    %s %s, byte [mem]\n", mnemonic, reg8_name(reg));
    }
}

static void print_modrm_32(uint8_t *memory, uint32_t eip, const char *mnemonic, bool reverse) {
    uint8_t modrm = memory[eip + 1];
    uint8_t mod, reg, rm;
    DECODE_MODRM(modrm, mod, reg, rm);
    
    int instr_len = 2;
    bool has_disp32 = false;
    uint32_t disp32 = 0;
    
    if (mod==0 && rm==5){
        instr_len = 6;
        has_disp32 = true;
        disp32 = mem_read32(memory, eip + 2);
    }
    
    print_bytes(memory, eip, instr_len);
    
    if (mod == 3) {
        if (!reverse)
            printf("    %s %s, %s\n", mnemonic, reg32_name(rm), reg32_name(reg));
        else
            printf("    %s %s, %s\n", mnemonic, reg32_name(reg), reg32_name(rm));
    } 
    else if (has_disp32) {
        if (!reverse)
            printf("    %s dword [0x%08X], %s\n", mnemonic, disp32, reg32_name(reg));
        else
            printf("    %s %s, dword [0x%08X]\n", mnemonic, reg32_name(reg), disp32);
    }
    else {
        if (!reverse)
            printf("    %s dword [mem], %s\n", mnemonic, reg32_name(reg));
        else
            printf("    %s %s, dword [mem]\n", mnemonic, reg32_name(reg));
    }
}

static void print_imm8(uint8_t *memory, uint32_t eip, const char *mnemonic, const char *reg_name) {
    int instr_len = 2;
    print_bytes(memory, eip, instr_len);
    uint8_t imm = memory[eip + 1];
    printf("%s %s, %u\n", mnemonic, reg_name, imm);
}

static void print_imm32(uint8_t *memory, uint32_t eip, const char *mnemonic, const char *reg_name) {
    int instr_len = 5;
    print_bytes(memory, eip, instr_len);
    uint32_t imm = mem_read32(memory, eip + 1);
    if (imm <= 255)
        printf("%s %s, %u\n", mnemonic, reg_name, imm);
    else
        printf("%s %s, 0x%08X\n", mnemonic, reg_name, imm);
}

void disassemble(uint8_t *memory, uint32_t eip) {
    uint8_t op = memory[eip];
    printf("%08X: ", eip);
	
    if(op == 0x0F) {
        uint8_t subop = memory[eip + 1];

        switch(subop) {
            case 0x82: { /* JB rel32 */
                int instr_len = 6;
                print_bytes(memory, eip, instr_len);
                int32_t rel = mem_read32(memory, eip + 2);
                printf("    jb 0x%08X\n", eip + instr_len + rel);
                return;
            }
            case 0x84: { // JE rel32
                int instr_len = 6;
                print_bytes(memory, eip, instr_len);
                int32_t rel = mem_read32(memory, eip + 2);
                printf("    je 0x%08X\n", eip + instr_len + rel);
                return;
            }
            case 0x85: { // JNE rel32
                int instr_len = 6;
                print_bytes(memory, eip, instr_len);
                int32_t rel = mem_read32(memory, eip + 2);
                printf("    jne 0x%08X\n", eip + instr_len + rel);
                return;
            }
            case 0x86: { /* JBE rel32 */
                int instr_len = 6;
                print_bytes(memory, eip, instr_len);
                int32_t rel = mem_read32(memory, eip + 2);
                printf("    jbe 0x%08X\n", eip + instr_len + rel);
                return;
            }
            default:
                print_bytes(memory, eip, 2);
                printf("    db 0x0F, 0x%02X\n", subop);
                return;
        }
    }

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
        case 0x00: print_modrm_8(memory, eip, "add", false); break; // ADD r/m8, r8
        case 0x01: print_modrm_32(memory, eip, "add", false); break; // ADD r/m32, r32
        case 0x02: print_modrm_8(memory, eip, "add", true); break; // ADD r8, r/m8
        case 0x03: print_modrm_32(memory, eip, "add", true); break; // ADD r32, r/m32
        case 0x04: print_imm8(memory, eip, "add", "al"); break; // ADD AL, imm8
        case 0x05: print_imm32(memory, eip, "add", "eax"); break; // ADD EAX, imm32

        case 0x88: print_modrm_8(memory, eip, "mov", false); break; // MOV r/m8, r8
        case 0x89: print_modrm_32(memory, eip, "mov", false); break; // MOV r/m32, r32
        case 0x8A: print_modrm_8(memory, eip, "mov", true); break; // MOV r8, r/m8
        case 0x8B: print_modrm_32(memory, eip, "mov", true); break; // MOV r32, r/m32

        case 0x18: print_modrm_8(memory, eip, "sub", false); break; // SUB r/m8, r8
        case 0x19: print_modrm_32(memory, eip, "sub", false); break; // SUB r/m32, r32
        case 0x1A: print_modrm_8(memory, eip, "sub", true); break; // SUB r8, r/m8
        case 0x1B: print_modrm_32(memory, eip, "sub", true); break; // SUB r32, r/m32
        case 0x1C: print_imm8(memory, eip, "sub", "al"); break; // SUB AL, imm8
        case 0x1D: print_imm32(memory, eip, "sub", "eax"); break; // SUB EAX, imm32

        case 0x30: print_modrm_8(memory, eip, "xor", false); break; // XOR r/m8, r8
        case 0x31: print_modrm_32(memory, eip, "xor", false); break; // XOR r/m32, r32
        case 0x32: print_modrm_8(memory, eip, "xor", true); break; // XOR r8, r/m8
        case 0x33: print_modrm_32(memory, eip, "xor", true); break; // XOR r32, r/m32
        case 0x34: print_imm8(memory, eip, "xor", "al"); break; // XOR AL, imm8
        case 0x35: print_imm32(memory, eip, "xor", "eax"); break; // XOR EAX, imm32

        case 0x38: print_modrm_8(memory, eip, "cmp", false); break; // CMP r/m8, r8
        case 0x39: print_modrm_32(memory, eip, "cmp", false); break; // CMP r/m32, r32
        case 0x3A: print_modrm_8(memory, eip, "cmp", true); break; // CMP r8, r/m8
        case 0x3B: print_modrm_32(memory, eip, "cmp", true); break; // CMP r32, r/m32
        case 0x3C: print_imm8(memory, eip, "cmp", "al"); break; // CMP AL, imm8
        case 0x3D: print_imm32(memory, eip, "cmp", "eax"); break; // CMP EAX, imm32
		
        case 0x20: print_modrm_8(memory, eip, "and", false); break; // AND r/m8, r8
        case 0x21: print_modrm_32(memory, eip, "and", false); break; // AND r/m32, r32
        case 0x22: print_modrm_8(memory, eip, "and", true); break; // AND r8, r/m8
        case 0x23: print_modrm_32(memory, eip, "and", true); break; // AND r32, r/m32
        case 0x24: print_imm8(memory, eip, "and", "al"); break; // AND AL, imm8
        case 0x25: print_imm32(memory, eip, "and", "eax"); break; // AND EAX, imm32
		
        case 0x08: print_modrm_8(memory, eip, "or", false); break; // OR r/m8, r8
        case 0x09: print_modrm_32(memory, eip, "or", false); break; // OR r/m32, r32
        case 0x0A: print_modrm_8(memory, eip, "or", true); break; // OR r8, r/m8
        case 0x0B: print_modrm_32(memory, eip, "or", true); break; // OR r32, r/m32
        case 0x0C: print_imm8(memory, eip, "or", "al"); break; // OR AL, imm8
        case 0x0D: print_imm32(memory, eip, "or", "eax"); break; // OR EAX, imm32
        
        /* MOV reg8, imm8 */
        case 0xB0: case 0xB1: case 0xB2: case 0xB3:
        case 0xB4: case 0xB5: case 0xB6: case 0xB7: {
            int instr_len = 2;
            print_bytes(memory, eip, instr_len);
            uint8_t imm = memory[eip + 1];
            int reg = op & 7;
            printf("    mov %s, %u\n", reg8_name(reg), imm);
            break;
        }
        
        /* MOV r/m8, imm8 */
        case 0xC6: {
            uint8_t modrm = memory[eip + 1];
            uint8_t mod, regop, rm;
            DECODE_MODRM(modrm, mod, regop, rm);
            
            uint8_t imm = memory[eip + 2];
            
            int instr_len = 3;
            if (mod == 0 && rm == 5) instr_len = 6;
            
            print_bytes(memory, eip, instr_len);
            
            if (mod == 3) {
                printf("    mov %s, %u\n", reg8_name(rm), imm);
            } else if (mod == 0 && rm == 5) {
                uint32_t addr = mem_read32(memory, eip + 2);
                printf("    mov byte [0x%08X], %u\n", addr, imm);
            } else {
                printf("    mov byte [mem], %u\n", imm);
            }
            break;
        }

        /* SUB AL, imm8 */
        case 0x2C: {
            instr_len = 2;
            print_bytes(memory, eip, instr_len);
            uint8_t imm = memory[eip + 1];
            printf("    sub al, %u\n", imm);
            break;
        }
        
        case 0x80: {
            int instr_len = 3;
            print_bytes(memory, eip, instr_len);
            
            uint8_t modrm = memory[eip + 1];
            uint8_t mod, regop, rm;
            DECODE_MODRM(modrm, mod, regop, rm);
            
            uint8_t imm = memory[eip + 2];
            
            const char *mnemonic = "???";
            switch(regop) {
                case 0: mnemonic = "add"; break;
                case 1: mnemonic = "or";  break;
                case 2: mnemonic = "adc"; break;
                case 3: mnemonic = "sbb"; break;
                case 4: mnemonic = "and"; break;
                case 5: mnemonic = "sub"; break;
                case 6: mnemonic = "xor"; break;
                case 7: mnemonic = "cmp"; break;
            }
            
            if (mod == 3) {
                printf("    %s %s, %u\n", mnemonic, reg8_name(rm), imm);
            } else {
                printf("    %s byte [mem], %u\n", mnemonic, imm);
            }
            break;
        }
 
        /* MOV AL, [addr] */
        case 0xA0: {
            int instr_len = 5;
            print_bytes(memory, eip, instr_len);
            uint32_t addr = mem_read32(memory, eip + 1);
            printf("    mov al, [0x%08X]\n", addr);
            break;
        }
        /* MOV [addr], AL */
        case 0xA2: {
            int instr_len = 5;
            print_bytes(memory, eip, instr_len);
            uint32_t addr = mem_read32(memory, eip + 1);
            printf("    mov [0x%08X], al\n", addr);
            break;
        }
        /* JNE rel8 */
        case 0x75: {
            instr_len = 2;
            print_bytes(memory, eip, instr_len);
            int8_t rel = (int8_t)memory[eip + 1];
            printf("    jne 0x%08X\n", eip + instr_len + rel);
            break;
        }

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

        // JE rel8
        case 0x74: {
            int instr_len = 2;
            print_bytes(memory, eip, instr_len);
            int8_t rel = (int8_t)memory[eip + 1];
            printf("    je 0x%08X\n", eip + instr_len + rel);
            break;
        }

        /* PUSH reg32 */
        case 0x50: print_bytes(memory, eip, 1); printf("    push eax\n"); break;
        case 0x51: print_bytes(memory, eip, 1); printf("    push ecx\n"); break;
        case 0x52: print_bytes(memory, eip, 1); printf("    push edx\n"); break;
        case 0x53: print_bytes(memory, eip, 1); printf("    push ebx\n"); break;
        case 0x54: print_bytes(memory, eip, 1); printf("    push esp\n"); break;
        case 0x55: print_bytes(memory, eip, 1); printf("    push ebp\n"); break;
        case 0x56: print_bytes(memory, eip, 1); printf("    push esi\n"); break;
        case 0x57: print_bytes(memory, eip, 1); printf("    push edi\n"); break;
        
        /* POP reg32 */
        case 0x58: print_bytes(memory, eip, 1); printf("    pop eax\n"); break;
        case 0x59: print_bytes(memory, eip, 1); printf("    pop ecx\n"); break;
        case 0x5A: print_bytes(memory, eip, 1); printf("    pop edx\n"); break;
        case 0x5B: print_bytes(memory, eip, 1); printf("    pop ebx\n"); break;
        case 0x5C: print_bytes(memory, eip, 1); printf("    pop esp\n"); break;
        case 0x5D: print_bytes(memory, eip, 1); printf("    pop ebp\n"); break;
        case 0x5E: print_bytes(memory, eip, 1); printf("    pop esi\n"); break;
        case 0x5F: print_bytes(memory, eip, 1); printf("    pop edi\n"); break;
        
        // CALL rel32
        case 0xE8: {
            instr_len = 5;
            print_bytes(memory, eip, instr_len);
            int32_t rel = mem_read32(memory, eip + 1);
            printf("    call 0x%08X\n", eip + instr_len + rel);
            break;
        }
        
        /* INC reg32 */
        case 0x40: print_bytes(memory, eip, 1); printf("    inc eax\n"); break;
        case 0x41: print_bytes(memory, eip, 1); printf("    inc ecx\n"); break;
        case 0x42: print_bytes(memory, eip, 1); printf("    inc edx\n"); break;
        case 0x43: print_bytes(memory, eip, 1); printf("    inc ebx\n"); break;
        case 0x44: print_bytes(memory, eip, 1); printf("    inc esp\n"); break;
        case 0x45: print_bytes(memory, eip, 1); printf("    inc ebp\n"); break;
        case 0x46: print_bytes(memory, eip, 1); printf("    inc esi\n"); break;
        case 0x47: print_bytes(memory, eip, 1); printf("    inc edi\n"); break;
        
        /* DEC reg32 */
        case 0x48: print_bytes(memory, eip, 1); printf("    dec eax\n"); break;
        case 0x49: print_bytes(memory, eip, 1); printf("    dec ecx\n"); break;
        case 0x4A: print_bytes(memory, eip, 1); printf("    dec edx\n"); break;
        case 0x4B: print_bytes(memory, eip, 1); printf("    dec ebx\n"); break;
        case 0x4C: print_bytes(memory, eip, 1); printf("    dec esp\n"); break;
        case 0x4D: print_bytes(memory, eip, 1); printf("    dec ebp\n"); break;
        case 0x4E: print_bytes(memory, eip, 1); printf("    dec esi\n"); break;
        case 0x4F: print_bytes(memory, eip, 1); printf("    dec edi\n"); break;
        
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
