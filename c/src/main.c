#include "../include/print.h"

void *start(void) __attribute__((section(".text.boot")));

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

int run(void)
{
    return fibonacci(5);
}

void _start(void)
{
    run();
    print_char('a');
    print_char('b');
    end();
}