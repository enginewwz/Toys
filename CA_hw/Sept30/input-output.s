.section .bss
input_char:
    .space 1

.section .text
.global _start
_start:
    # read from stdin
    mov $0, %rax
    mov $0, %rdi
    lea input_char(%rip), %rsi
    mov $1, %rdx
    syscall

    # write to stdout
    mov $1, %rax
    mov $1, %rdi
    lea input_char(%rip), %rsi
    mov $1, %rdx
    syscall

    mov $60, %rax
    xor %rdi, %rdi
    syscall
