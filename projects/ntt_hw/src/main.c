#include "../include/measure.h"
#include "../include/plugin_c.h"
#include "../include/print.h"
#include "stdint.h"

#define QINV -3327 // q^-1 mod 2^16
#define KYBER_Q 3329



const int16_t zetas[128] = {
    -1044, -758,  -359,  -1517, 1493,  1422,  287,   202,  -171,  622,   1577,
    182,   962,   -1202, -1474, 1468,  573,   -1325, 264,  383,   -829,  1458,
    -1602, -130,  -681,  1017,  732,   608,   -1542, 411,  -205,  -1571, 1223,
    652,   -552,  1015,  -1293, 1491,  -282,  -1544, 516,  -8,    -320,  -666,
    -1618, -1162, 126,   1469,  -853,  -90,   -271,  830,  107,   -1421, -247,
    -951,  -398,  961,   -1508, -725,  448,   -1065, 677,  -1275, -1103, 430,
    555,   843,   -1251, 871,   1550,  105,   422,   587,  177,   -235,  -291,
    -460,  1574,  1653,  -246,  778,   1159,  -147,  -777, 1483,  -602,  1119,
    -1590, 644,   -872,  349,   418,   329,   -156,  -75,  817,   1097,  603,
    610,   1322,  -1285, -1465, 384,   -1215, -136,  1218, -1335, -874,  220,
    -1187, -1659, -1185, -1530, -1278, 794,   -1510, -854, -870,  478,   -108,
    -308,  996,   991,   958,   -1460, 1522,  1628};

int16_t montgomery_reduce(int32_t a) {
  int16_t t;

  t = (int16_t)a * QINV;
  t = (a - (int32_t)t * KYBER_Q) >> 16;
  return t;
}

int32_t multiply_hardware(int32_t a, int32_t b){
    return cfu_op0_hw(3, a, b); 
}

static int16_t fqmul(int16_t a, int16_t b) {

  int32_t mul_result = cfu_op0_hw(3, a, b);
  return cfu_op0_hw(2, mul_result, 0);
  //return montgomery_reduce((int32_t)a * b);
}

int16_t barrett_reduce(int16_t a) {
  int16_t t;
  const int16_t v = ((1 << 26) + KYBER_Q / 2) / KYBER_Q;

  t = ((int32_t)v * a + (1 << 25)) >> 26;
  t *= KYBER_Q;
  return a - t;
}

void ntt(int16_t r[256]) {
  unsigned int len, start, j, k;
  int16_t t, zeta;

  k = 1;
  for (len = 128; len >= 2; len >>= 1) {
    for (start = 0; start < 256; start = j + len) {
      zeta = zetas[k++];
      for (j = start; j < start + len; j++) {
        t = fqmul(zeta, r[j + len]);
        r[j + len] = r[j] - t;
        r[j] = r[j] + t;
      }
    }
  }
}

void invntt(int16_t r[256]) {
  unsigned int start, len, j, k;
  int16_t t, zeta;
  const int16_t f = 1441; // mont^2/128

  k = 127;
  for (len = 2; len <= 128; len <<= 1) {
    for (start = 0; start < 256; start = j + len) {
      zeta = zetas[k--];
      for (j = start; j < start + len; j++) {
        t = r[j];
        r[j] = barrett_reduce(t + r[j + len]);
        r[j + len] = r[j + len] - t;
        r[j + len] = fqmul(zeta, r[j + len]);
      }
    }
  }

  for (j = 0; j < 256; j++)
    r[j] = fqmul(r[j], f);
}

void basemul(int16_t r[2], const int16_t a[2], const int16_t b[2],
             int16_t zeta) {
  r[0] = fqmul(a[1], b[1]);
  r[0] = fqmul(r[0], zeta);
  r[0] += fqmul(a[0], b[0]);
  r[1] = fqmul(a[0], b[1]);
  r[1] += fqmul(a[1], b[0]);
}

uint32_t simple_rand() { return cfu_op0_hw(1, 0, 0); }

#define ARRAY_SIZE 256

int main() {
  int16_t poly_a[ARRAY_SIZE]; // Original polynomial
  int16_t poly_b[ARRAY_SIZE]; // Second polynomial for testing
  int16_t ntt_a[ARRAY_SIZE];  // NTT of poly_a

  // Initialize polynomials with random values
  for (int i = 0; i < ARRAY_SIZE; i++) {
    poly_a[i] = simple_rand() % KYBER_Q;  
    poly_b[i] = simple_rand()% KYBER_Q; 
  }

  // Test 1: Roundtrip NTT->INVNTT preservation
  print_str("==== Test 1 =====\n");
  print_str("Original values: ");
  for (int i = 0; i < 5; i++) { // Print first 5 values
    print_int(poly_a[i]);
    print_str(" ");
  }
  print_str("...\n");

  // Apply NTT
  for (int i = 0; i < ARRAY_SIZE; i++) {
    ntt_a[i] = montgomery_reduce((int32_t)poly_a[i] * 65536); // Convert to Montgomery domain
  }





  ACTIVATE_COUNTER(0);
  ntt(ntt_a);
  DEACTIVATE_COUNTER(0);

  print_str("After NTT: ");
  for (int i = 0; i < 5; i++) {
    print_int(ntt_a[i]);
    print_str(" ");
  }
  print_str("...\n");






  //   // Apply inverse NTT
  //   invntt(ntt_a);
  //   DEACTIVATE_COUNTER(0);

  //   //   // Normalize results to the correct range
  //   //   for (int i = 0; i < ARRAY_SIZE; i++) {
  //   //     ntt_a[i] = montgomery_reduce((int32_t)ntt_a[i]);  // Convert out
  //   of
  //   //     Montgomery domain ntt_a[i] = ((ntt_a[i] % KYBER_Q) + KYBER_Q) %
  //   //     KYBER_Q;  // Normalize
  //   //   }

  //     print_str("After INVNTT: ");
  //     for (int i = 0; i < 5; i++) {
  //       print_int(ntt_a[i]);
  //       print_str(" ");
  //     }
  //     print_str("...\n");
  // }

}