bits 64

_start:
    mov rax, 0
    mov rsi, 0xFFFFFFFF
    int 0x50

.hang:
    cli
    hlt
    jmp .hang
    