.section .text.boot
.globl _start

_start:
    # Initialize stack pointer
    la sp, __stack_top

    # Jump to main function
    call main

    # Exit if main returns
    li a7, 0  # SYS_EXIT
    li a0, 0   # Exit code
    ecall

    # Fallback halt
1:  j 1b