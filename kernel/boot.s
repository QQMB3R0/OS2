MBFLAGS equ 0
MAGIC equ 0x1BADB002
CHECKSUM equ -(MAGIC + MBFLAGS)

section .multiboot
align 4
    dd MAGIC
    dd MBFLAGS
    dd CHECKSUM

section .bss
align 16
stack_bottom:
resb 16384
stack_top:

section .text
extern main
global _start:function (_start.end - _start)

_start:
            mov esp, stack_top
            call main
            cli
.hang:      hlt
            jmp .hang
.end:
