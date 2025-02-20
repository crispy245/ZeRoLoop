// #include <stdint.h>
// #include "../include/print.h"
// #define KYBER_Q 3329

// // Precomputed zetas (used in the NTT) – note these are arranged in bit‐reversed order.
// const int16_t zetas[128] = {
//   -1044,  -758,  -359, -1517,  1493,  1422,   287,   202,
//    -171,   622,  1577,   182,   962, -1202, -1474,  1468,
//     573, -1325,   264,   383,  -829,  1458, -1602,  -130,
//    -681,  1017,   732,   608, -1542,   411,  -205, -1571,
//    1223,   652,  -552,  1015, -1293,  1491,  -282, -1544,
//     516,    -8,  -320,  -666, -1618, -1162,   126,  1469,
//    -853,   -90,  -271,   830,   107, -1421,  -247,  -951,
//    -398,   961, -1508,  -725,   448, -1065,   677, -1275,
//   -1103,   430,   555,   843, -1251,   871,  1550,   105,
//     422,   587,   177,  -235,  -291,  -460,  1574,  1653,
//    -246,   778,  1159,  -147,  -777,  1483,  -602,  1119,
//    -1590,   644,  -872,   349,   418,   329,  -156,   -75,
//     817,  1097,   603,   610,  1322, -1285, -1465,   384,
//    -1215,  -136,  1218, -1335,  -874,   220, -1187, -1659,
//    -1185, -1530, -1278,   794, -1510,  -854,  -870,   478,
//    -108,  -308,   996,   991,   958, -1460,  1522,  1628
// };

// /*
//  * Barrett Reduction:
//  * Computes x mod mod (here mod = KYBER_Q) using a precomputed mu.
//  */
// uint32_t barrett_reduction(uint32_t x) {
//     const uint32_t mod = KYBER_Q;
//     const uint64_t mu = 1290167ULL;  // Precomputed floor(2^32 / 3329)
//     uint64_t product = (uint64_t)x * mu;
//     uint32_t q = product >> 32;
//     uint32_t r = x - q * mod;
//     if (r >= mod) {
//         r -= mod;
//     }
//     return r;

//     print_str("BARRET REDUCTION DONE! \n");
// }

// /*
//  * Multiplication in Z_q using Barrett reduction.
//  * This function first maps any negative inputs into the range [0, KYBER_Q),
//  * multiplies them, reduces the result modulo KYBER_Q, and then converts the
//  * result back to the symmetric range.
//  */
// static int16_t fqmul(int16_t a, int16_t b) {
//     const int32_t mod = KYBER_Q;
//     int32_t a_mod = a, b_mod = b;
//     if (a_mod < 0) a_mod += mod;
//     if (b_mod < 0) b_mod += mod;
//     uint32_t product = (uint32_t)(a_mod * b_mod);
//     uint32_t reduced = barrett_reduction(product);
//     // Convert to symmetric representation if necessary.
//     if (reduced > mod / 2)
//          return (int16_t)(reduced - mod);
//     else
//          return (int16_t)reduced;
// }

// /*
//  * In-place Number-Theoretic Transform (NTT).
//  * The input array r (of 256 coefficients) is transformed in-place.
//  */
// void ntt(int16_t r[256]) {
//     unsigned int len, start, j, k;
//     int16_t t, zeta;

//     k = 1;
//     for(len = 128; len >= 2; len >>= 1) {
//         for(start = 0; start < 256; start = j + len) {
//             zeta = zetas[k++];
//             for(j = start; j < start + len; j++) {
//                 t = fqmul(zeta, r[j + len]);
//                 r[j + len] = r[j] - t;
//                 r[j] = r[j] + t;
//             }
//         }
//     }
// }

// int main(void) {
//     int16_t poly[256];
//     // Initialize poly with sample data in [0, KYBER_Q).
//     for (int i = 0; i < 256; i++) {
//         poly[i] = i % KYBER_Q;
//     }

//     print_str("Input polynomial:\n");
//     for (int i = 0; i < 256; i++) {
//         print_int(poly[i]);
//         print_str(" ");
//         if ((i + 1) % 16 == 0)
//             print_str("\n");
//     }

//     // Perform the NTT transform.
//     ntt(poly);

//     print_str("\nNTT output:\n");
//     for (int i = 0; i < 256; i++) {
//         print_int(poly[i]);
//         print_str(" ");
//         if ((i + 1) % 16 == 0)
//             print_str("\n");
//     }

//     return 0;
// }


#include "../include/print.h"
#include "../include/plugin_c.h"


int main() {

    int a = 600;
    int b = 600;
    int c = cfu_op0_hw(1,a,b);
    return c;
}

