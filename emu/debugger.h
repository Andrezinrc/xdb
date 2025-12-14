/*
 * Debugger commands:
 * s = step       -> executa uma instrução
 * c = continue   -> executa até parar/encerrar
 * q = quit       -> sai do emulador
 * r <reg> <val>  -> escreve registrador
 * w <addr> <val> -> escreve memória
 * b <addr>       -> set breakpoint (INT3)
 * d <addr>       -> delete breakpoint
 */

#ifndef DEBUGGER_H
#define DEBUGGER_H

#include <stdint.h>
#include "cpu.h"

struct Debugger { int running; };

void dbg_prompt(char *buf, size_t size);
void dbg_handle_cmd(
    struct Debugger *dbg,
    char *cmd,
    struct CPU *cpu,
    uint8_t *memory
);
void dbg_help(void);

#endif
