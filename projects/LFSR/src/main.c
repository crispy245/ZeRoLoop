#include "../include/measure.h"
#include "../include/plugin_c.h"
#include "../include/print.h"
#include <stdint.h>


// Basic LFSR implementation
uint16_t basic_lfsr(uint16_t state) {
    // Feedback polynomial: x^16 + x^14 + x^13 + x^11 + 1
    uint16_t bit = ((state >> 0) ^ (state >> 2) ^ (state >> 3) ^ (state >> 5)) & 1;
    state = (state >> 1) | (bit << 15);
    return state;
}

// First non-linear LFSR: Filter function using multiple taps
uint16_t nonlinear_lfsr1(uint16_t state) {
    // Basic LFSR step
    uint16_t bit = ((state >> 0) ^ (state >> 2) ^ (state >> 3) ^ (state >> 5)) & 1;
    uint16_t next_state = (state >> 1) | (bit << 15);
    
    // Non-linear filter function using multiple taps
    uint8_t x1 = (state >> 1) & 1;
    uint8_t x2 = (state >> 6) & 1;
    uint8_t x3 = (state >> 10) & 1;
    uint8_t output = ((x1 & x2) ^ (x2 & x3) ^ x1 ^ x3) & 1;
    
    // Replace LSB with non-linear output
    return (next_state & 0xFFFE) | output;
}

// Second non-linear LFSR: Self-clocking with irregular stepping
uint16_t nonlinear_lfsr2(uint16_t state) {
    // Clock control bit
    uint8_t clock_bit = ((state >> 8) & 1);
    
    // Apply regular LFSR step
    uint16_t bit = ((state >> 0) ^ (state >> 2) ^ (state >> 3) ^ (state >> 5)) & 1;
    uint16_t next_state = (state >> 1) | (bit << 15);
    
    // Apply second step conditionally based on clock bit
    if (clock_bit) {
        bit = ((next_state >> 0) ^ (next_state >> 2) ^ (next_state >> 3) ^ (next_state >> 5)) & 1;
        next_state = (next_state >> 1) | (bit << 15);
    }
    
    return next_state;
}

int main() {
    uint32_t lfsr = 0xABCDE;

    ACTIVATE_COUNTER(0);
    uint32_t out0 = basic_lfsr(lfsr);
    DEACTIVATE_COUNTER(0);
    printf("Basic LFSR: %d\n", out0);

    ACTIVATE_COUNTER(0);
    uint32_t out1 = nonlinear_lfsr1(lfsr);
    DEACTIVATE_COUNTER(0);
    printf("NonLinear1 LFSR: %d\n", out1);

    ACTIVATE_COUNTER(0);
    uint32_t out2 = nonlinear_lfsr2(lfsr);
    DEACTIVATE_COUNTER(0);
    printf("NonLinear2 LFSR: %d\n", out2);

    return 0;
}