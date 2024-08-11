%define DISPLAY_BUFFER_LEN 128

section .bss
    display_buffer resb DISPLAY_BUFFER_LEN

section .text

global display_int
global plus_int

plus_int:
    mov rax, rdi
    add rax, rsi
    ret
    
display_int:
    push rbp
    mov rbp, rsp

    push r8
    push r9
    push r10

    mov r8, rdi
    lea r9, [display_buffer + DISPLAY_BUFFER_LEN]
    mov r10, 10

    cmp r8, 0
    jge .loop
    imul r8, -1

.loop:
    dec r9

    mov rdx, 0
    mov rax, r8
    div r10
    mov r8, rax
    mov byte [r9], dl
    add byte [r9], '0'

    cmp r8, 0
    jne .loop

    cmp rdi, 0
    jge .write
    dec r9
    mov byte [r9], '-'

.write:
    mov rax, 1
    mov rdi, 1
    mov rsi, r9
    imul r9, -1
    lea r9, [display_buffer + DISPLAY_BUFFER_LEN + r9 + 1]
    mov rdx, r9
    syscall

    pop r10
    pop r9
    pop r8

    mov rax, 0

    mov rsp, rbp
    pop rbp
    ret
