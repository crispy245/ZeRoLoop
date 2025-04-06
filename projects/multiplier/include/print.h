#pragma once

// Syscall numbers
#define SYS_PRINT_CHAR 1
#define SYS_EXIT 93 // Similar to Linux

static void print_char(const char c){
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

static inline void print_int(int num) {
    // Handle negative numbers.
    if (num < 0) {
        print_char('-');
        num = -num;
    }
    
    // Base case: if num is a single digit, print it directly.
    if (num < 10) {
        print_char('0' + num);
        return;
    }
    
    int quotient = 0;
    int temp = num;
    while (temp >= 10) {
        temp -= 10;
        quotient++;
    }

    print_int(quotient);
    print_char('0' + temp);
}

