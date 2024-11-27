ENTRY(_start)
MEMORY
{
    rom (rx) : ORIGIN = 0x00001000, LENGTH = 16M
    ram (rw) : ORIGIN = 0x10000000, LENGTH = 32M
}
SECTIONS
{
    .text : {
        KEEP(*(.text.boot))
        *(.text*)
    } > rom
    
    .rodata : {
        *(.rodata*)
    } > rom
    
    .data : {
        *(.data*)
    } > ram AT > rom
    
    .bss (NOLOAD) : {
        *(.bss*)
        *(COMMON)
    } > ram
    
    /* Add stack section */
    .stack (NOLOAD) : {
        . = ALIGN(16);
        . = . + 0x1000; /* 4KB stack */
        . = ALIGN(16);
        __stack_top = .;
    } > ram
}