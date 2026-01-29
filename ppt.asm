BITS 32
org 0x0

section .data
msg_jogador1 db "Jogador 1: 1-Pedra 2-Papel 3-Tesoura",10,0
msg_jogador2 db "Jogador 2: 1-Pedra 2-Papel 3-Tesoura",10,0
msg_empate   db "Empate!",10,0
msg_vitoria1 db "Jogador 1 venceu!",10,0
msg_vitoria2 db "Jogador 2 venceu!",10,0
msg_invalido db "Jogada invalida! (1-3)",10,0

len_msg1 equ 37
len_msg2 equ 37
len_empate equ 8
len_v1 equ 18
len_v2 equ 18
len_inv equ 23

section .bss
jogada1 resb 2
jogada2 resb 2

section .text
global _start
_start:

pedir1:
mov eax, 4
mov ebx, 1
mov ecx, msg_jogador1
mov edx, len_msg1
int 0x80

mov eax, 3
mov ebx, 0
mov ecx, jogada1
mov edx, 2
int 0x80

mov al, [jogada1]
cmp al, '1'
je validar1_max
cmp al, '2'
je validar1_max
cmp al, '3'
je validar1_max

invalido1:
mov eax, 4
mov ebx, 1
mov ecx, msg_invalido
mov edx, len_inv
int 0x80
jmp pedir1

validar1_max:
jmp pedir2

pedir2:
mov eax, 4
mov ebx, 1
mov ecx, msg_jogador2
mov edx, len_msg2
int 0x80

mov eax, 3
mov ebx, 0
mov ecx, jogada2
mov edx, 2
int 0x80

mov al, [jogada2]
cmp al, '1'
je validar2_max
cmp al, '2'
je validar2_max
cmp al, '3'
je validar2_max

mov eax, 4
mov ebx, 1
mov ecx, msg_invalido
mov edx, len_inv
int 0x80
jmp pedir2

validar2_max:
mov al, [jogada1]
mov bl, [jogada2]
cmp al, bl
je empate

cmp al, '1'
jne caso2
cmp bl, '3'
je vitoria1
jmp vitoria2

caso2:
cmp al, '2'
jne caso3
cmp bl, '1'
je vitoria1
jmp vitoria2

caso3:
cmp bl, '2'
je vitoria1

vitoria2:
mov ecx, msg_vitoria2
mov edx, len_v2
jmp mostrar

vitoria1:
mov ecx, msg_vitoria1
mov edx, len_v1
jmp mostrar

empate:
mov ecx, msg_empate
mov edx, len_empate

mostrar:
mov eax, 4
mov ebx, 1
int 0x80

mov eax, 1
mov ebx, 0
int 0x80
