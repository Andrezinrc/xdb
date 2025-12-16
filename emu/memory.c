#include "cpu.h"
#include "memory.h"
#include <stdlib.h>
#include <string.h>

uint8_t* mem_create() {
    uint8_t *mem = malloc(MEM_SIZE);
    memset(mem, 0, MEM_SIZE);
    return mem;
}

void mem_destroy(uint8_t *mem) {
    free(mem);
}

uint8_t mem_read8(uint8_t *mem, uint32_t addr) {
    return mem[addr];
}

// Lê 32 bits em little-endian
// Byte menos significativo no endereço mais baixo
uint32_t mem_read32(uint8_t *mem, uint32_t addr) {
    return mem[addr] |
          (mem[addr+1] << 8) |
          (mem[addr+2] << 16) |
          (mem[addr+3] << 24);
}

void mem_write8(uint8_t *mem, uint32_t addr, uint8_t val) {
    mem[addr] = val;
}



// Escreve 32 bits em little-endian
void mem_write32(uint8_t *mem, uint32_t addr, uint32_t val) {
    mem[addr]     =  val        & 0xFF;
    mem[addr + 1] = (val >> 8)  & 0xFF;
    mem[addr + 2] = (val >> 16) & 0xFF;
    mem[addr + 3] = (val >> 24) & 0xFF;
}


// Empilha valor na pilha x86
// Pilha cresce para endereços menores, por isso ESP é decrementado
void push32(uint8_t *mem, struct CPU *cpu, uint32_t val){
    cpu->esp.e -= 4;                   // Move ponteiro da pilha
    mem_write32(mem, cpu->esp.e, val); // Escreve no topo
}


// Desempilha valor da pilha x86
uint32_t pop32(uint8_t *mem, struct CPU *cpu) {
    uint32_t val = mem_read32(mem, cpu->esp.e);
    cpu->esp.e += 4;
    return val;
}


// CALL rel32: salva endereço de retorno e desvia para EIP + rel
void call_rel32(uint8_t *mem, struct CPU *cpu, int32_t rel){
    push32(mem, cpu, cpu->eip + 5); // Salva endereço de retorno
    cpu->eip += rel + 5;            // Salta para destino da chamada
}



// RET: retorna para o endereço no topo da pilha
void ret(uint8_t *mem, struct CPU *cpu) {
    cpu->eip = pop32(mem, cpu);
}
