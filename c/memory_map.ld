ENTRY(_start)
MEMORY
{
    rom  (rx) : ORIGIN = 0x000, LENGTH = 4K     /* Full instruction_memory */
    ram  (rw) : ORIGIN = 0x000, LENGTH = 3840   /* 3.75K for actual data */
    io   (rw) : ORIGIN = 0xF00, LENGTH = 256    /* Last 256 bytes reserved for I/O */
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
    
    /* Stack section */
    .stack (NOLOAD) : {
        . = ALIGN(16);
        . = . + 0x200; /* 512 bytes stack */
        . = ALIGN(16);
        __stack_top = .;
    } > ram

    /* Prevent any data from being placed in the I/O region */
    .io (NOLOAD) : {
        KEEP(*(.io))
    } > io
}