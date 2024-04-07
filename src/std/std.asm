%define DISPLAY_BUFFER_LEN 128

section .bss
    display_buffer resb DISPLAY_BUFFER_LEN

section .text

global displayINT64
global displaySTRING
global plusINT64

plusINT64:
    mov rax, rdi
    add rax, rsi
    ret
    
displayINT64:
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
    mov rax, 1 ; syscall=write
    mov rdi, 1 ; stdout
    mov rsi, r9 ; text
    imul r9, -1
    lea r9, [display_buffer + DISPLAY_BUFFER_LEN + r9 + 1]
    mov rdx, r9 ; length
    syscall

    pop r10
    pop r9
    pop r8

    mov rax, 0

    mov rsp, rbp
    pop rbp
    ret

; expects pointer to string
; TODO: I don't like that we count bytes even if we know the length in advance
displaySTRING:
    push rbp
    mov rbp, rsp

    push r9
    mov r9, 0
.loop:
    cmp BYTE [rdi + r9], 0
    je .write
    inc r9
    jmp .loop

.write:
    mov rax, 1 ; syscall=write
    mov rsi, rdi ; text 
    mov rdi, 1 ; stdout
    mov rdx, r9 ; length
    syscall

    pop r9

    mov rax, 0

    mov rsp, rbp
    pop rbp
    ret
    
