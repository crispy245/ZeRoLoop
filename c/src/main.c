#include "../include/print.h"


#define end() asm volatile ( \
    "li x5, 1\n\t"          \
    "csrw 21, x5\n\t"     \
    ::: "x5")


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


void run(){
    return;
}

void __attribute__((section(".text._start"))) _start(void)
{
    run();
    print_str("happy new year");
    end();
}