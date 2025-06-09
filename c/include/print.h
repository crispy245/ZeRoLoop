#include <stdarg.h>


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

static inline void divide_by_10(int num, int *quotient, int *remainder) {
    int divisor = 10;
    int q = 0;
    int r = num;
    // Subtract larger multiples of 10 at once:
    while (r >= divisor) {
        int temp = divisor;
        int multiple = 1;
        // Double the divisor until it is too high.
        while (r >= (temp + temp)) {
            temp += temp;
            multiple += multiple;
        }
        r -= temp;
        q += multiple;
    }
    *quotient = q;
    *remainder = r;
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
    
    char buffer[32];  // Enough for 32-bit int
    int i = 0;
    
    // Fill buffer in reverse order using our fast division algorithm.
    while (num > 0) {
        int quotient, remainder;
        divide_by_10(num, &quotient, &remainder);
        buffer[i++] = '0' + remainder;
        num = quotient;
    }
    
    // Print digits in the correct order
    while (i > 0) {
        print_char(buffer[--i]);
    }
}


static void printf(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    while (*format != '\0') {
        if (*format == '%') {
            format++;
            
            switch (*format) {
                case 'd': {
                    int val = va_arg(args, int);
                    print_int(val);
                    break;
                }
                case 's': {
                    char* str = va_arg(args, char*);
                    print_str(str);
                    break;
                }
                case 'c': {
                    // Note: char is promoted to int in varargs
                    char c = va_arg(args, int);
                    print_char(c);
                    break;
                }
                case '%': {
                    print_char('%');
                    break;
                }
                default: {
                    // Unsupported format specifier
                    print_char('%');
                    print_char(*format);
                    break;
                }
            }
        } else {
            print_char(*format);
        }
        
        format++;
    }
    
    va_end(args);
}