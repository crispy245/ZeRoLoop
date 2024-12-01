ENTRY(_start)
MEMORY
{
    instruction_mem (rx) : ORIGIN = 0x00000000, LENGTH = 8K
    data_mem (rw)       : ORIGIN = 0x00000000, LENGTH = 8K  
}

SECTIONS
{
    /* All code goes to instruction memory */
    .text : {
        *(.text._start)     /* Entry point first */
        *(.text*)           /* All other code */
        . = ALIGN(4);
    } > instruction_mem
    
    .rodata : {
        *(.rodata*)
        . = ALIGN(4);
    } > instruction_mem
    
    /* All data goes to data memory */
    .tohost 0x00002000 : {
        *(.tohost)
    } > data_mem
    
    .data : {
        . = ALIGN(4);
        *(.data*)
        . = ALIGN(4);
    } > data_mem
    
    .bss (NOLOAD) : {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > data_mem
    
    .stack (NOLOAD) : {
        . = ALIGN(16);
        . = . + 0x400;      /* 1KB stack */
        . = ALIGN(16);
        __stack_top = .;
    } > data_mem
}