#include "../include/print.h"

#define end() asm volatile ( \
    "li x5, 1\n\t" \
    "csrw 0x15, x5\n\t" \
    ::: "x5")

int multiply(int a, int b) {
    int result = 0;
    while (b > 0) {
        if (b & 1) {
            result += a;
        }
        a = a << 1;
        b = b >> 1;
    }
    return result;
}

int square(int x) {
    return multiply(x, x);
}

int run() {
    int x = 3;
    
    // Compute x^2 + x + 1
    int result = square(x);  // x^2
    result += x;            // + x
    result++;              // + 1
    
    return result;
}

void __attribute__((section(".text._start"))) _start(void) {
    asm volatile(
        "li sp, 0x2400"
    );
    
    int result = run();
    result = square(result & 7);  // Keep number bounded with AND 7
    end();
}