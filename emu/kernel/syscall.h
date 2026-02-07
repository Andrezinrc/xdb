#ifndef SYSCALL_H
#define SYSCALL_H

#include "process.h"

void dispatch_syscall(struct fake_process *proc);

#endif
