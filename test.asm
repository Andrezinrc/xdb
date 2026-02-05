BITS 32
org 0x0

start:
    mov eax, 3
    inc eax
    inc eax

    call func

    mov eax, 1
    int 0x80

func:
    push ebx
    mov ebx, eax
    add ebx, 2
    pop ebx
    ret
