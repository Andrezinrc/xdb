adr86

Emulador educacional de CPU x86 em C.

Compilar

```bash
gcc main.c emu/cpu.c emu/memory.c emu/disasm.c \
emu/ptrace-fake.c emu/debugger.c \
-I emu -o emulator
```

Executar

```bash
./emulator
```

Sobre

Emula uma CPU x86 32-bit com sistema de memória e debugger integrado. Permite modificar registradores e memória durante a execução via comandos interativos.

Comandos do Debugger

· s - executa uma instrução
· c - continua até breakpoint
· r <reg> <val> - modifica registrador
· w <addr> <val> - escreve na memória
· b <addr> - adiciona breakpoint
· d <addr> - remove breakpoint
· q - sai

Projeto para aprendizado de arquitetura de computadores e funcionamento de debuggers.
