.section .text.boot
    .globl _start
    
_start:
    # Set up stack pointer
    la sp, __stack_top

    # Set up global pointer
    .option push
    .option norelax
    la gp, __global_pointer$
    .option pop

    # Clear BSS section
    la t0, __bss_start
    la t1, __bss_end
clear_bss:
    bgeu t0, t1, clear_bss_done
    sw zero, 0(t0)
    addi t0, t0, 4
    j clear_bss
clear_bss_done:

    # Jump to C code
    call main

    # If we somehow return, stop CPU
    li t0, 1            # Using t0 (x5) register
    csrw 21, t0    # Writing to mstatus CSR 
1:  j 1b