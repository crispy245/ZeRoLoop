ENTRY(_start)

MEMORY
{
    instruction_mem (rx) : ORIGIN = 0x00000, LENGTH = 128K
    data_mem (rw)       : ORIGIN = 0x20000, LENGTH = 8K  
}

SECTIONS
{
    /* All code goes to instruction memory */
    .text : {
        . = ALIGN(4);
        *(.text._start)     
        *(.text*)           
        . = ALIGN(4);
    } > instruction_mem
    
    /* Force rodata to start at data memory base */
    .rodata ORIGIN(data_mem) : {
        . = ALIGN(4);
        *(.rodata*)         
        . = ALIGN(4);
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