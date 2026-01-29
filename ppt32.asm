BITS 32
org 0x0

section .data
msg_jogador1 db "Jogador 1: 1-Pedra 2-Papel 3-Tesoura",10,0
msg_jogador2 db "Jogador 2: 1-Pedra 2-Papel 3-Tesoura",10,0
msg_empate   db "Empate!",10,0
msg_vitoria1 db "Jogador 1 venceu!",10,0
msg_vitoria2 db "Jogador 2 venceu!",10,0
msg_invalido db "Jogada invalida! (1-3)",10,0

msg_round    db "Round "
round_num    db "0",10,0
msg_result   db "J1=0 J2=0",10,0

msg_campeao1 db "Jogador 1 CAMPEAO!",10,0
msg_campeao2 db "Jogador 2 CAMPEAO!",10,0

len_msg1 equ 37
len_msg2 equ 37
len_empate equ 8
len_v1 equ 18
len_v2 equ 18
len_inv equ 23
len_round equ 8
len_result equ 10
len_campeao equ 19

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
je validar1
cmp al, '2'
je validar1
cmp al, '3'
je validar1

invalido1:
mov eax, 4
mov ebx, 1
mov ecx, msg_invalido
mov edx, len_inv
int 0x80
jmp pedir1

validar1:
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
mov al, [jogada1]
mov [msg_result+3], al
mov al, [jogada2]
mov [msg_result+8], al

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
je campeao1

mov al, [pontos_bin_j2]
cmp al, 3
je campeao2

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

campeao1:
mov eax, 4
mov ebx, 1
mov ecx, msg_campeao1
mov edx, len_campeao
int 0x80
jmp fim

campeao2:
mov eax, 4
mov ebx, 1
mov ecx, msg_campeao2
mov edx, len_campeao
int 0x80

fim:
mov eax, 1
mov ebx, 0
int 0x80
