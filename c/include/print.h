#pragma once

// Syscall numbers
#define SYS_PRINT_CHAR 1
#define SYS_EXIT 93 // Similar to Linux

static inline void __attribute__((naked)) print_char(const char c){
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
    // Handle negative numbers
    if (num < 0) {
        print_char('-');
        num = -num;
    }
    
    // Handle special case for 0
    if (num == 0) {
        print_char('0');
        return;
    }
    
    char buffer[12];  // Enough for 32-bit int including sign and null
    int i = 0;
    
    // Convert to characters using repeated subtraction instead of division
    while (num > 0) {
        int digit = 0;
        int temp = num;
        
        while (temp >= 10) {
            temp -= 10;
            digit++;
        }
        
        buffer[i] = '0' + digit;
        num = temp;
        i++;
    }
    
    // Print in reverse order
    while (i > 0) {
        i--;
        print_char(buffer[i]);
    }
}


