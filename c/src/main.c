#include "../include/print.h"

#define end() asm volatile ( \
    "li x5, 1\n\t" \
    "csrw 0x15, x5\n\t" \
    ::: "x5")
int run() {

    return 5;
}

extern char __stack_top;

void __attribute__((section(".text._start"))) __attribute__((naked)) _start(void) {

    register char* sp asm("sp") = &__stack_top;
    print_str("hello");
    run();
    end();
}