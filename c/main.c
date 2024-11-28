// Entry point for freestanding environment
void *start(void) __attribute__((section(".text.boot")));

#define end() asm volatile ( \
    "li x5, 1\n\t"          \
    "csrw 21, x5\n\t"     \
    ::: "x5")

// Fibonacci calculation
int fibonacci(int n)
{
    int i;
    // initialize first and second terms
    int t1 = 0, t2 = 1;
    // initialize the next term (3rd term)
    int nextTerm = t1 + t2;

    // print 3rd to nth terms
    for (i = 3; i <= n; ++i)
    {
        t1 = t2;
        t2 = nextTerm;
        nextTerm = t1 + t2;
    }
    return nextTerm;
}

// Main computation
int run(void)
{
    // Calculate 47th Fibonacci number
    return fibonacci(47);
}



// Boot entry point
void _start(void)
{
    // Get Fibonacci result
    run();
    end();
    

}