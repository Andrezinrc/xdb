TARGET = xdb
CC = gcc
CFLAGS = -Iemu -Wall -Wextra -g

SRCS = main.c \
       emu/cpu.c emu/cpu_flags.c emu/cpu_ops.c emu/mem.c emu/dasm.c emu/ptrace-fake.c emu/dbg.c emu/decode.c \
       emu/kernel/kernel.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

clean:
	rm -f $(TARGET)
