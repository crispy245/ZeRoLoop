#include "../include/measure.h"
#include "../include/plugin_c.h"
#include "../include/print.h"

int main()
{

    int a = 520;
    int b = 420;

    ACTIVATE_COUNTER(0);
    int c = cfu_op0_hw(2, a, b);
    DEACTIVATE_COUNTER(0);

    print_int(c);

    return c;
}