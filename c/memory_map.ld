ENTRY(_start)
MEMORY
{
    /* Expanded memory to accommodate compliance tests */
    rom  (rx) : ORIGIN = 0x00000000, LENGTH = 8K    /* Doubled ROM size */
    ram  (rw) : ORIGIN = 0x00002000, LENGTH = 8K    /* Moved RAM start, doubled size */
}

SECTIONS
{
    /* Keep tohost near start of memory for compliance tests */
    .tohost 0x00002000 : {
        *(.tohost)
    } > ram
    
    .text : {
        KEEP(*(.text.boot))  /* Boot code first */
        *(.text.init)        /* Init code */
        *(.text*)           /* All other code */
        *(.xcpthandler)     /* Exception handler */
        . = ALIGN(4);
    } > rom
    
    .rodata : {
        *(.rodata*)
        . = ALIGN(4);
    } > rom
    
    .data : {
        . = ALIGN(4);
        *(.data*)
        . = ALIGN(4);
    } > ram AT > rom
    
    .bss (NOLOAD) : {
        . = ALIGN(4);
        __bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end = .;
    } > ram
    
    /* Stack section - reduced size to 1KB */
    .stack (NOLOAD) : {
        . = ALIGN(16);
        . = . + 0x400; /* 1KB stack */
        . = ALIGN(16);
        __stack_top = .;
    } > ram

    /* Add a section for _start if it's missing */
    .text.start : {
        /* Define _start symbol if not provided */
        PROVIDE(_start = .);
    } > rom
}

/* Define _start as weak symbol to allow overriding */
PROVIDE(_start = 0x00000000);