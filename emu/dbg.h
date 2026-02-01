#pragma once
#include <stdio.h>
#include <stdint.h>
#include "cpu.h"
#include "kernel/kernel.h"

struct Debugger { int running; };

void dbg_prompt(char *buf, size_t size);
void dbg_handle_cmd(
    struct Debugger *dbg,
    char *cmd,
    struct CPU *cpu,
    uint8_t *memory
);
void dbg_help(void);
void dbg_trace_syscall(struct CPU *cpu);
