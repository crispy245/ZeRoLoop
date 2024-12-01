#pragma once

// Syscall numbers
#define SYS_PRINT_CHAR 1
#define SYS_EXIT 93 // Similar to Linux

static inline void print_char(const char c)
{
    register int a0 asm("a0") = c;        // Force a0 allocation
    register int a7 asm("a7") = SYS_PRINT_CHAR;  // Force a7 allocation
    
    asm volatile(
        "ecall"
        : 
        : "r"(a0), "r"(a7)  // Use the allocated registers
        : "memory"
    );
}

static inline void print_str(const char *str) {
    while (1) {
        char c = *str;
        if (c == 0) break;
        print_char(c);
        str++;
    }
}

static inline void exit(int code)
{
    asm volatile(
        "li a7, %1\n\t" // Syscall number for exit
        "mv a0, %0\n\t" // Exit code in a0
        "ecall"
        :
        : "r"(code), "i"(SYS_EXIT)
        : "a0", "a7");
}

