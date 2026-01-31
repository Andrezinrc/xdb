BITS 32
org 0x0

section .data
msg_jogador1 db "CPU: 1-Pedra 2-Papel 3-Tesoura",10,0
len_msg1    equ $ - msg_jogador1

msg_jogador2 db "Voce: 1-Pedra 2-Papel 3-Tesoura",10,0
len_msg2    equ $ - msg_jogador2

msg_empate   db "Empate!",10,0
len_empate   equ $ - msg_empate

msg_vitoria1 db "CPU venceu!",10,0
len_v1       equ $ - msg_vitoria1

msg_vitoria2 db "Voce venceu!",10,0
len_v2       equ $ - msg_vitoria2

msg_invalido db "Jogada invalida! (1-3)",10,0
len_inv      equ $ - msg_invalido

msg_round    db "Round "
len_round    equ $ - msg_round

round_num    db "0",10,0

msg_result   db "CPU="
cpu_jogada   db "        Voce="
voce_jogada  db "        ",10,0
len_result   equ $ - msg_result

pedra_str   db "Pedra   "
papel_str   db "Papel   "
tesoura_str db "Tesoura "

msg_perdeu   db "CPU GANHOU!",10,0
len_perdeu   equ $ - msg_perdeu

msg_ganhou   db "VOCE GANHOU!",10,0
len_ganhou   equ $ - msg_ganhou

section .bss
jogada1 resb 2
jogada2 resb 2
pontos_j1 resb 1
pontos_j2 resb 1
round resb 1
pontos_bin_j1 resb 1
pontos_bin_j2 resb 1

section .text
global _start
_start:
mov byte [pontos_j1], '0'
mov byte [pontos_j2], '0'
mov byte [pontos_bin_j1], 0
mov byte [pontos_bin_j2], 0
mov byte [round], '1'

jogo_loop:
mov al, [round]
mov [round_num], al

pedir1:
rdtsc
xor eax, edx
and eax, 3
cmp al, 0
je pedir1
add al, '0'
mov [jogada1], al

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
je validar2
cmp al, '2'
je validar2
cmp al, '3'
je validar2

mov eax, 4
mov ebx, 1
mov ecx, msg_invalido
mov edx, len_inv
int 0x80
jmp pedir2

validar2:
mov esi, pedra_str
mov edi, cpu_jogada
mov ecx, 8

mov al, [jogada1]
cmp al, '1'
je copiar_cpu
mov esi, papel_str
cmp al, '2'
je copiar_cpu
mov esi, tesoura_str

copiar_cpu:
movsb
dec ecx
jnz copiar_cpu

mov esi, pedra_str
mov edi, voce_jogada
mov ecx, 8

mov al, [jogada2]
cmp al, '1'
je copiar_voce
mov esi, papel_str
cmp al, '2'
je copiar_voce
mov esi, tesoura_str

copiar_voce:
movsb
dec ecx
jnz copiar_voce

mov eax, 4
mov ebx, 1
mov ecx, msg_result
mov edx, len_result
int 0x80

mov al, [jogada1]
mov bl, [jogada2]
cmp al, bl
je empate

cmp al, '1'
jne caso2
cmp bl, '2'
je vitoria2
jmp vitoria1

caso2:
cmp al, '2'
jne caso3
cmp bl, '3'
je vitoria2
jmp vitoria1

caso3:
cmp bl, '1'
je vitoria2
jmp vitoria1

vitoria1:
mov eax, 4
mov ebx, 1
mov ecx, msg_vitoria1
mov edx, len_v1
int 0x80

mov al, [pontos_bin_j1]
add al, 1
mov [pontos_bin_j1], al
mov al, [pontos_j1]
add al, 1
mov [pontos_j1], al
jmp verificar_campeao

vitoria2:
mov eax, 4
mov ebx, 1
mov ecx, msg_vitoria2
mov edx, len_v2
int 0x80

mov al, [pontos_bin_j2]
add al, 1
mov [pontos_bin_j2], al
mov al, [pontos_j2]
add al, 1
mov [pontos_j2], al

verificar_campeao:
mov al, [pontos_bin_j1]
cmp al, 3
je perdeu

mov al, [pontos_bin_j2]
cmp al, 3
je ganhou

mov al, [round]
add al, 1
mov [round], al
cmp al, '9'
jb jogo_loop

jmp fim

empate:
mov eax, 4
mov ebx, 1
mov ecx, msg_empate
mov edx, len_empate
int 0x80
jmp verificar_campeao

perdeu:
mov eax, 4
mov ebx, 1
mov ecx, msg_perdeu
mov edx, len_perdeu
int 0x80
jmp fim

ganhou:
mov eax, 4
mov ebx, 1
mov ecx, msg_ganhou
mov edx, len_ganhou
int 0x80

fim:
mov eax, 1
mov ebx, 0
int 0x80
