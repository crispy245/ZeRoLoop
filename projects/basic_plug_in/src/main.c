#include "../include/print.h"
#include "../include/plugin_c.h"


int main() {

    int a = 5;
    int b = 5;
    int c = cfu_op0_hw(1,a,b);
    print_int(c);
    return 0;
}