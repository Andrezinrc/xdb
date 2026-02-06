BITS 32
org 0x0

start:
    call recursive_depth
    int 0x80

recursive_depth:
    call level1
    ret

level1:
    call level2
    ret

level2:
    call level3
    ret

level3:
    call level4
    ret

level4:
    call level5
    ret

level5:
    call recursive_depth
    ret
