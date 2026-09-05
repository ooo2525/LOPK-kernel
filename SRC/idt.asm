bits 64

section .text

global idt_load
global dummy_interrupt
global dummyhandler
global int50_handler
global PRAX
global PRDI
global PRSI
global PRBX
global PRCX
global PRDX
global keyb_handler

extern int50
extern keybh

idt_load:
    lidt [rdi]
    ret

dummy_interrupt:
    iretq

dummy_exception:
    add rsp, 8
    iretq

int50_handler:
    mov [rel PRAX], rax
    mov [rel PRDI], rdi
    mov [rel PRSI], rsi
    mov [rel PRBX], rbx
    mov [rel PRCX], rcx
    mov [rel PRDX], rdx

    call int50

    mov rax, [rel PRAX]
    mov rdi, [rel PRDI]
    mov rsi, [rel PRSI]
    mov rbx, [rel PRBX]
    mov rcx, [rel PRCX]
    mov rdx, [rel PRDX]

    iretq
    
keyb_handler:
    call keybh
    iretq

section .data

PRAX: dq 0
PRDI: dq 0
PRSI: dq 0
PRBX: dq 0
PRCX: dq 0
PRDX: dq 0
