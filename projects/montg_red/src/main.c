#include "../include/measure.h"
#include "../include/plugin_c.h"
#include "../include/print.h"
#include "stdint.h"

#define QINV -3327 // q^-1 mod 2^16
#define KYBER_Q 3329

// Software implementation of Montgomery reduction
int16_t sw_montgomery_reduce(int32_t a) {
  int16_t t;

  t = (int16_t)a * QINV;
  t = (a - (int32_t)t * KYBER_Q) >> 16;
  return t;
}

int main() {
  print_str("Montgomery Reduction Test Suite\n");

  int num_tests = 0;
  int num_passes = 0;

  // Try a variety of values (negatives, small, large, edge cases)
  int32_t test_values[] = {0,
                           1,
                           -1,
                           KYBER_Q,
                           -KYBER_Q,
                           2 * KYBER_Q,
                           -2 * KYBER_Q,
                           3 * KYBER_Q,
                           -3 * KYBER_Q,
                           32767,
                           -32768, // int16_t bounds
                           5000,
                           -5000,
                           10000,
                           -10000,
                           65535,
                           -65535,
                           100000,
                           -100000};

  int num_values = sizeof(test_values) / sizeof(test_values[0]);

  for (int i = 0; i < num_values; i++) {
    int32_t a = test_values[i];
    int16_t sw_mont = sw_montgomery_reduce(a);
    int16_t hw_mont = cfu_op0_hw(2, a, 0);

    print_str("Test ");
    print_int(i);
    print_str(": a = ");
    print_int(a);
    print_str(" | SW = ");
    print_int(sw_mont);
    print_str(", HW = ");
    print_int(hw_mont);

    if (sw_mont == hw_mont) {
      print_str(" [PASS]\n");
      num_passes++;
    } else {
      print_str(" [FAIL]\n");
    }

    num_tests++;
  }

  print_str("\nSummary: ");
  print_int(num_passes);
  print_str("/");
  print_int(num_tests);
  print_str(" tests passed.\n");

  return 0;
}