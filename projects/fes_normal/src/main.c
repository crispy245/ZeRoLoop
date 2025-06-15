#include "../include/measure.h"
#include <../include/print.h>
#include <../include/plugin_c.h>
#include <stdint.h>
#include <stdlib.h>

// Define maximum sizes for arrays
#define MAX_N 10              // Maximum number of variables
#define MAX_M 10              // Maximum number of polynomials
#define MAX_SOLS (1 << MAX_N) // Maximum number of solutions (2^MAX_N)

/*
Represents a quadratic polynomial over GF(2) with n variables.
The polynomial is of the form:
f(x) = constant + (sum of linear terms) + (sum of quadratic terms)
Linear terms: linear[i] * x_i
Quadratic terms: quad[i][j] * x_i * x_j

Example: Polynomial f(x) = 1 + x_1 + x_2 + x_1 x_2 + x_0 x_2 in 3 variables:
  constant = 1
  linear = [0, 1, 1]  (coefficients of x_0, x_1, x_2)
  quad[1][0] = 0       (coefficient of x_1 x_0)
  quad[2][0] = 1       (coefficient of x_2 x_0)
  quad[2][1] = 1       (coefficient of x_2 x_1)
*/

// Hardware-based random
uint64_t rand_hardware() {
  return cfu_op0_hw(1,0,0);
}


typedef struct {
  int constant;           // 0 or 1
  int linear[MAX_N];      // array of length n (each entry 0 or 1)
  int quad[MAX_N][MAX_N]; // 2D array: for i=0..n-1, quad[i][j]
} Poly;

// Getter functions for the polynomial coefficients.
int get_constant(Poly *f) { return f->constant; }
int get_linear(Poly *f, int k) { return f->linear[k]; }
int get_quad(Poly *f, int k, int j) { return f->quad[k][j]; }

void random_poly(Poly *poly, int n) {
  // Set our constant as 0 or 1
  poly->constant = rand_hardware() % 2;

  // Set our linear terms [0, 1, 0, 1] would be x1 + x3
  for (int i = 0; i < n; i++) {
    poly->linear[i] = rand_hardware() % 2;
  }

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      poly->quad[i][j] = rand_hardware() % 2;
    }
  }

  // Multiplication in GF2 is commutative, meaning (xi⋅xj=xj⋅xi) and
  // (xi​⋅xj​=xj​⋅xi​)
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < i; j++) {
      poly->quad[i][j] = rand_hardware() % 2;
    }
  }
}

typedef struct {
  unsigned int i;
  unsigned int y;
  unsigned int d1[MAX_N];
  unsigned int d2[MAX_N][MAX_N];
} State;

// Fully unrolled, branchless bit1 using de Bruijn multiplication
// https://en.wikipedia.org/wiki/Find_first_set
// http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn
static const int MultiplyDeBruijnBitPosition[32] = {
    0,  1,  28, 2,  29, 14, 24, 3, 30, 22, 20, 15, 25, 17, 4,  8,
    31, 27, 13, 23, 21, 19, 16, 7, 26, 12, 18, 6,  11, 5,  10, 9};

int bit1(unsigned int x) {
  unsigned int v = x;
  // If x is nonzero, compute the index; if zero, r is meaningless.
  unsigned int r = MultiplyDeBruijnBitPosition[((v & -v) * 0x077CB531U) >> 27];
  // Create a mask that is all 1's when x==0.
  unsigned int m = -(!x);
  //  x==0, return -1; otherwise return r. Who needs branches...
  return (int)((r & ~m) | ((unsigned int)-1 & m));
}

int bit2(unsigned int x) { return bit1(x ^ (x & -x)); }

void step(State *s) {
  s->i++;
  int k1 = bit1(s->i);
  int k2 = bit2(s->i);
  int is_k2_none = k2 == -1;

  // If k2 == -1, we add 1 to the index array (This can be done only with a
  // sltiu) After than we make a mask by adding 1 to k2. If k2 happened to be -1
  // then the resulting mask (-1 + 1 = 0) would be zero. Now we flip all the
  // bits and a mask of all 1's should have been created

  s->d1[k1] ^= s->d2[(is_k2_none + k2) & ~(!(k2 + 1))][k1];

  s->y ^= s->d1[k1];
}

void init_state(State *s, Poly polys[MAX_M], int m, int n) {
  s->i = 0;
  s->y = 0;

  // Initialize arrays to zero
  for (int k = 0; k < n; k++) {
    s->d1[k] = 0;
    for (int j = 0; j < n; j++) {
      s->d2[k][j] = 0;
    }
  }

  // We use the getters to not override any actual values
  for (int i = 0; i < m; i++) {
    Poly *f = &polys[i];
    s->y |= ((unsigned int)get_constant(f)) << i;

    for (int k = 0; k < n; k++) {
      for (int j = 0; j < k; j++) {
        s->d2[k][j] |= ((unsigned int)get_quad(f, k, j)) << i;
      }
    }

    s->d1[0] |= ((unsigned int)get_linear(f, 0)) << i;

    for (int k = 1; k < n; k++) {
      int bit = (((s->d2[k][k - 1] >> i) & 1) ^ get_linear(f, k));
      s->d1[k] |= ((unsigned int)bit) << i;
    }
  }
}

// Evaluate polynomial f on vector x (of length n). All operations are mod 2.
int eval_poly(Poly *f, int *x, int n) {
  int result = f->constant;
  for (int i = 0; i < n; i++) {
    if (x[i])
      result ^= f->linear[i];
  }
  for (int i = 0; i < n; i++) {
    for (int j = 0; j < i; j++) {
      if (x[i] && x[j])
        result ^= f->quad[i][j];
    }
  }
  return result & 1;
}

void fes_eval(Poly polys[MAX_M], int m, int n, unsigned int sols[MAX_SOLS],
              int *nsol) {
  int sol_count = 0;
  State s;

  init_state(&s, polys, m, n);

  while (s.i < ((1u << n) - 1)) {
    if (s.y == 0) {
      sols[sol_count++] = s.i ^ (s.i >> 1);
    }
    step(&s);
  }

  *nsol = sol_count;
}

int main(void) {
  int n = N;  // number of variables
  int m = M; // number of polynomials

  // Generate a random "known solution" (an n–bit vector)
  int sol[MAX_N];

  print_str("Known solution:\n");
  for (int i = 0; i < n; i++) {
    sol[i] = rand_hardware() % 2;
    print_int(sol[i]);
  }

  // Create an array of m random quadratic polynomials in n variables.
  // Then "adjust" each so that the known solution is a root i.e. f(sol) = 0.

  /*
    In short, we generate the polynomial 'polys' such that is built from a
    constant term [0 or 1], a series of linear terms (x_a * [0 or 1]) and a
    series of quadratic terms (x_a * x_b * [0 or 1]). 'polys' is composed of n
    variables on m polynomials. After we generate a vector of size n named 'sol'
    reprenting a specific solution to the system.
    'sol' is used to constrained 'poly' such that it evaluates to zero at a
    specific solution
  */

  Poly polys[MAX_M];
  for (int i = 0; i < m; i++) {
    random_poly(&polys[i], n);
    if (eval_poly(&polys[i], sol, n) == 1) {
      // Flip the constant term so that f(sol) becomes 0.
      polys[i].constant ^= 1;
    }
  }

  // // Print the input polynomials.
  // print_str("\nInput polynomials:\n");
  // for (int i = 0; i < m; i++) {
  //   print_str("f");
  //   print_int(i);
  //   print_str(" = ");
  //   print_int(polys[i].constant);

  //   for (int j = 0; j < n; j++) {
  //     if (polys[i].linear[j]) {
  //       print_str(" + x");
  //       print_int(j);
  //     }
  //   }

  //   for (int k = 0; k < n; k++) {
  //     for (int j = 0; j < k; j++) {
  //       if (polys[i].quad[k][j]) {
  //         print_str(" + x");
  //         print_int(k);
  //         print_str("*x");
  //         print_int(j);
  //       }
  //     }
  //   }

  //   print_char('\n');
  // }

  // Evaluate (enumerate) all candidate solutions.
  int sol_count;
  unsigned int sols[MAX_SOLS];

  ACTIVATE_COUNTER(0);
  fes_eval(polys, m, n, sols, &sol_count);
  DEACTIVATE_COUNTER(0);

  // print_str("\nFound solution: ...\n");
  // print_char('\n');
  // // Print the found solutions (each printed as an n–bit binary string).
  // for (int i = 0; i < sol_count; i++) {
  //   unsigned int sol_val = sols[i];
  //   for (int j = 0; j < n; j++) { // Iterate from 0 to n-1 (LSB to MSB)
  //     print_int((sol_val >> j) & 1);
  //     print_char(' ');
  //   }
  //   print_char('\n');
  // }

  return 0;
}
