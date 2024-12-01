#include "../include/print.h"


#define end() asm volatile ( \
    "li x5, 1\n\t"          \
    "csrw 21, x5\n\t"     \
    ::: "x5")

int fibonacci(int n)
{
    int i;
    int t1 = 0, t2 = 1;
    int nextTerm = t1 + t2;
    for (i = 3; i <= n; ++i)
    {
        t1 = t2;
        t2 = nextTerm;
        nextTerm = t1 + t2;
    }
    return nextTerm;
}

int run(){
    return fibonacci(47);
}

void __attribute__((section(".text._start"))) _start(void)
{
    run();
    end();
}