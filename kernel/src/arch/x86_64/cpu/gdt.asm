[bits 64]

global LoadGDT
LoadGDT:
    lgdt [rdi]
    mov ax, 0x10
    mov es, ax
    mov ds, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Perform a far return with the kernel code segment
    pop rdi
    mov rax, 0x08 ; Kernel code segment
    push rax
    push rdi
    retfq
