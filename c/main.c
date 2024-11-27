// Entry point for freestanding environment
void _start(void) __attribute__((section(".text.boot")));

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
    // Calculate 10th Fibonacci number
    return fibonacci(47);
}

// Data section
static int result = 0;

// Boot entry point
void _start(void)
{
    // Get Fibonacci result
    result = run();
}