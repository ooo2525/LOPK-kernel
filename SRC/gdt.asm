bits 64

section .text

global gdt_load

gdt_load:
    lgdt [rdi]

    ; Kernel data selector = 0x10
    mov ax, 0x10

    mov ds, ax
    mov es, ax
    mov ss, ax

    ; Reload CS with kernel code selector = 0x08
    push 0x08
    lea rax, [rel .reload_cs]
    push rax
    retfq

.reload_cs:
    ret
