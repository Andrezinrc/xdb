#include "decoder.h"

/*
 * modrm = 0xD8 (11011000b)
 * mod = 11b (3) -> registrador-registrador
 * reg = 011b (3) -> ebx
 * r/m = 000b (0) -> eax
 * Retorna: true, *reg=3, *rm=0
 */
bool modrm_reg_reg(uint8_t modrm, uint8_t *reg, uint8_t *rm) {
    if ((modrm >> 6) != 3) return false;
    *reg = (modrm >> 3) & 7;
    *rm  = modrm & 7;
    return true;
}
