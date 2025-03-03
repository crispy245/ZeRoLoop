#include "../include/measure.h"
#include "../include/plugin_c.h"
#include "../include/print.h"

int main()
{

    int a = 420;
    int b = 650;

    // Force register allocation for precise measurment
    register int reg_a asm("a0") = a;
    register int reg_b asm("a1") = b;

    ACTIVATE_COUNTER(0);
    int c = cfu_op0_hw(1, reg_a, reg_b);
    DEACTIVATE_COUNTER(0);

    ACTIVATE_COUNTER(0);
    c = a + b; // this will yield 0 gates because the compiler will simply optimize it
    DEACTIVATE_COUNTER(0);

    return c;
}