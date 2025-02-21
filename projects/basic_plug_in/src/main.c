#include "../include/print.h"
#include "../include/plugin_c.h"


int main() {

    int a = 2147483648;
    int b = 2147483647;
    int c = cfu_op0_hw(1,a,b);
    return c;
}