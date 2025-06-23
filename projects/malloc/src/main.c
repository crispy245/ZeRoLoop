#include "../include/measure.h"
#include "../include/plugin_c.h"
#include "../include/print.h"
#include <stdlib.h>


int main() {
    int *arr;
    int n = 1000;

    arr = (int*)malloc(sizeof(int)*n);

    for(int i = 0; i < 1000; i++){
        arr[i] = i;
        print_int(arr[i]);
        print_char(' ');
    }
}

