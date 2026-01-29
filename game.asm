; teste

BITS 32
org 0x0

section .data
h1 db "tente adivinhar o numero de (0-9)", 10
len_h1 equ $ - h1

msg_errou db "Voce errou!", 10
len_msg_errou equ $ - msg_errou

msg_acertou db "Voce acertou!", 10
len_msg_acertou equ $ - msg_acertou

input_buffer db 0

section .text
global _start
_start:
    mov eax, 4
    mov ebx, 1
    mov ecx, h1
    mov edx, len_h1
    int 0x80

input:
    mov eax, 3
    mov ebx, 0
    mov ecx, input_buffer
    mov edx, 1
    int 0x80

verificar:
    mov al, [input_buffer]
    sub al, 48
    cmp al, 8
    je acertou

errou: 
    mov eax, 4
    mov ebx, 1
    mov ecx, msg_errou
    mov edx, len_msg_errou
    int 0x80
    jmp input

acertou:
    mov eax, 4
    mov ebx, 1
    mov ecx, msg_acertou
    mov edx, len_msg_acertou
    int 0x80
    jmp fim

fim:
    mov eax, 1
    mov ebx, 0
    int 0x80
