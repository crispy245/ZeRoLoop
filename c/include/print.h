#pragma once

#define PRINT_CHAR_ADDR 0xF00

static inline void print_char(char c) {
    asm volatile (
        "mv x6, %0\n\t"        // Move character into x6
        "li x5, %1\n\t"        // Load address into x5
        "sb x6, 0(x5)\n\t"     // Store byte: x6 to address in x5
        :
        : "r" (c), "i" (PRINT_CHAR_ADDR)
        : "x5", "x6", "memory"
    );
}

// Function to print a string
static inline void print_str(const char* str) {
    while (*str) {
        print_char(*str++);
    }
}

// Function to print a number without using division/modulo
static void print_num(int num) {
    char buffer[32];
    int pos = 0;
    
    // Handle negative numbers
    if (num < 0) {
        print_char('-');
        // Handle INT_MIN case
        if (num == -2147483648) {
            print_str("2147483648");
            return;
        }
        num = -num;
    }
    
    // Handle zero case
    if (num == 0) {
        print_char('0');
        return;
    }
    
    // Convert to string using bit operations
    while (num > 0) {
        int digit = 0;
        int temp = num;
        
        // Subtract in a loop instead of using division/modulo
        while (temp >= 10) {
            temp -= 10;
            digit++;
        }
        
        buffer[pos++] = '0' + temp;
        
        // Update num by subtracting
        num = digit;
    }
    
    // Print in reverse order
    while (pos > 0) {
        print_char(buffer[--pos]);
    }
}
