#ifndef PROCESS_H
#define PROCESS_H

#include <stdint.h>
#include <sys/types.h>
#include "cpu.h"

struct fake_process {
    pid_t pid;
    struct CPU cpu;
    uint8_t *memory;
    int alive;
    int stopped;
};

#endif
