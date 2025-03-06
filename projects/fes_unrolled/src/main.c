#include "../include/measure.h"
#include <../include/print.h>
#include <stdint.h>
#include <stdlib.h>

// Define maximum sizes for arrays
#define MAX_N 5
#define MAX_M 10
#define MAX_SOLS (1 << MAX_N) // Maximum number of solutions (2^MAX_N)


typedef struct {
  int constant;           // 0 or 1
  int linear[MAX_N];      // array of length n (each entry 0 or 1)
  int quad[MAX_N][MAX_N]; // 2D array: for i=0..n-1, quad[i][j]
} Poly;

// Getter functions for the polynomial coefficients.
int get_constant(Poly *f) { return f->constant; }
int get_linear(Poly *f, int k) { return f->linear[k]; }
int get_quad(Poly *f, int k, int j) { return f->quad[k][j]; }

// For bit manipulation
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


// Preloaded polynomials with precomputed random values
void init_preloaded_polys(Poly polys[MAX_M]) {

  // Initialize polynomial 0
  polys[0].constant = 0;

  polys[0].linear[0] = 0;

  polys[0].linear[1] = 0;

  polys[0].linear[2] = 0;

  polys[0].linear[3] = 1;

  polys[0].linear[4] = 0;

  polys[0].quad[0][0] = 0;

  polys[0].quad[0][1] = 0;

  polys[0].quad[0][2] = 0;

  polys[0].quad[0][3] = 0;

  polys[0].quad[0][4] = 0;

  polys[0].quad[1][0] = 0;

  polys[0].quad[1][1] = 0;

  polys[0].quad[1][2] = 0;

  polys[0].quad[1][3] = 0;

  polys[0].quad[1][4] = 0;

  polys[0].quad[2][0] = 1;

  polys[0].quad[2][1] = 1;

  polys[0].quad[2][2] = 0;

  polys[0].quad[2][3] = 0;

  polys[0].quad[2][4] = 0;

  polys[0].quad[3][0] = 0;

  polys[0].quad[3][1] = 0;

  polys[0].quad[3][2] = 0;

  polys[0].quad[3][3] = 0;

  polys[0].quad[3][4] = 0;

  polys[0].quad[4][0] = 0;

  polys[0].quad[4][1] = 0;

  polys[0].quad[4][2] = 0;

  polys[0].quad[4][3] = 1;

  polys[0].quad[4][4] = 0;

  // Initialize polynomial 1
  polys[1].constant = 1;

  polys[1].linear[0] = 0;

  polys[1].linear[1] = 1;

  polys[1].linear[2] = 0;

  polys[1].linear[3] = 0;

  polys[1].linear[4] = 0;

  polys[1].quad[0][0] = 0;

  polys[1].quad[0][1] = 0;

  polys[1].quad[0][2] = 0;

  polys[1].quad[0][3] = 0;

  polys[1].quad[0][4] = 0;

  polys[1].quad[1][0] = 0;

  polys[1].quad[1][1] = 0;

  polys[1].quad[1][2] = 0;

  polys[1].quad[1][3] = 0;

  polys[1].quad[1][4] = 0;

  polys[1].quad[2][0] = 0;

  polys[1].quad[2][1] = 0;

  polys[1].quad[2][2] = 0;

  polys[1].quad[2][3] = 0;

  polys[1].quad[2][4] = 0;

  polys[1].quad[3][0] = 1;

  polys[1].quad[3][1] = 1;

  polys[1].quad[3][2] = 0;

  polys[1].quad[3][3] = 0;

  polys[1].quad[3][4] = 0;

  polys[1].quad[4][0] = 0;

  polys[1].quad[4][1] = 0;

  polys[1].quad[4][2] = 0;

  polys[1].quad[4][3] = 1;

  polys[1].quad[4][4] = 0;

  // Initialize polynomial 2
  polys[2].constant = 1;

  polys[2].linear[0] = 0;

  polys[2].linear[1] = 0;

  polys[2].linear[2] = 0;

  polys[2].linear[3] = 0;

  polys[2].linear[4] = 1;

  polys[2].quad[0][0] = 0;

  polys[2].quad[0][1] = 0;

  polys[2].quad[0][2] = 0;

  polys[2].quad[0][3] = 0;

  polys[2].quad[0][4] = 0;

  polys[2].quad[1][0] = 0;

  polys[2].quad[1][1] = 0;

  polys[2].quad[1][2] = 0;

  polys[2].quad[1][3] = 0;

  polys[2].quad[1][4] = 0;

  polys[2].quad[2][0] = 0;

  polys[2].quad[2][1] = 1;

  polys[2].quad[2][2] = 0;

  polys[2].quad[2][3] = 0;

  polys[2].quad[2][4] = 0;

  polys[2].quad[3][0] = 0;

  polys[2].quad[3][1] = 0;

  polys[2].quad[3][2] = 0;

  polys[2].quad[3][3] = 0;

  polys[2].quad[3][4] = 0;

  polys[2].quad[4][0] = 1;

  polys[2].quad[4][1] = 0;

  polys[2].quad[4][2] = 0;

  polys[2].quad[4][3] = 1;

  polys[2].quad[4][4] = 0;

  // Initialize polynomial 3
  polys[3].constant = 1;

  polys[3].linear[0] = 0;

  polys[3].linear[1] = 1;

  polys[3].linear[2] = 1;

  polys[3].linear[3] = 0;

  polys[3].linear[4] = 0;

  polys[3].quad[0][0] = 0;

  polys[3].quad[0][1] = 0;

  polys[3].quad[0][2] = 0;

  polys[3].quad[0][3] = 0;

  polys[3].quad[0][4] = 0;

  polys[3].quad[1][0] = 1;

  polys[3].quad[1][1] = 0;

  polys[3].quad[1][2] = 0;

  polys[3].quad[1][3] = 0;

  polys[3].quad[1][4] = 0;

  polys[3].quad[2][0] = 0;

  polys[3].quad[2][1] = 0;

  polys[3].quad[2][2] = 0;

  polys[3].quad[2][3] = 0;

  polys[3].quad[2][4] = 0;

  polys[3].quad[3][0] = 1;

  polys[3].quad[3][1] = 1;

  polys[3].quad[3][2] = 1;

  polys[3].quad[3][3] = 0;

  polys[3].quad[3][4] = 0;

  polys[3].quad[4][0] = 0;

  polys[3].quad[4][1] = 1;

  polys[3].quad[4][2] = 0;

  polys[3].quad[4][3] = 1;

  polys[3].quad[4][4] = 0;

  // Initialize polynomial 4
  polys[4].constant = 1;

  polys[4].linear[0] = 0;

  polys[4].linear[1] = 1;

  polys[4].linear[2] = 1;

  polys[4].linear[3] = 1;

  polys[4].linear[4] = 1;

  polys[4].quad[0][0] = 0;

  polys[4].quad[0][1] = 0;

  polys[4].quad[0][2] = 0;

  polys[4].quad[0][3] = 0;

  polys[4].quad[0][4] = 0;

  polys[4].quad[1][0] = 1;

  polys[4].quad[1][1] = 0;

  polys[4].quad[1][2] = 0;

  polys[4].quad[1][3] = 0;

  polys[4].quad[1][4] = 0;

  polys[4].quad[2][0] = 0;

  polys[4].quad[2][1] = 0;

  polys[4].quad[2][2] = 0;

  polys[4].quad[2][3] = 0;

  polys[4].quad[2][4] = 0;

  polys[4].quad[3][0] = 1;

  polys[4].quad[3][1] = 1;

  polys[4].quad[3][2] = 0;

  polys[4].quad[3][3] = 0;

  polys[4].quad[3][4] = 0;

  polys[4].quad[4][0] = 0;

  polys[4].quad[4][1] = 1;

  polys[4].quad[4][2] = 0;

  polys[4].quad[4][3] = 0;

  polys[4].quad[4][4] = 0;

  // Initialize polynomial 5
  polys[5].constant = 0;

  polys[5].linear[0] = 1;

  polys[5].linear[1] = 1;

  polys[5].linear[2] = 1;

  polys[5].linear[3] = 0;

  polys[5].linear[4] = 0;

  polys[5].quad[0][0] = 0;

  polys[5].quad[0][1] = 0;

  polys[5].quad[0][2] = 0;

  polys[5].quad[0][3] = 0;

  polys[5].quad[0][4] = 0;

  polys[5].quad[1][0] = 1;

  polys[5].quad[1][1] = 0;

  polys[5].quad[1][2] = 0;

  polys[5].quad[1][3] = 0;

  polys[5].quad[1][4] = 0;

  polys[5].quad[2][0] = 1;

  polys[5].quad[2][1] = 1;

  polys[5].quad[2][2] = 0;

  polys[5].quad[2][3] = 0;

  polys[5].quad[2][4] = 0;

  polys[5].quad[3][0] = 0;

  polys[5].quad[3][1] = 0;

  polys[5].quad[3][2] = 0;

  polys[5].quad[3][3] = 0;

  polys[5].quad[3][4] = 0;

  polys[5].quad[4][0] = 0;

  polys[5].quad[4][1] = 1;

  polys[5].quad[4][2] = 0;

  polys[5].quad[4][3] = 0;

  polys[5].quad[4][4] = 0;

  // Initialize polynomial 6
  polys[6].constant = 0;

  polys[6].linear[0] = 0;

  polys[6].linear[1] = 0;

  polys[6].linear[2] = 0;

  polys[6].linear[3] = 1;

  polys[6].linear[4] = 0;

  polys[6].quad[0][0] = 0;

  polys[6].quad[0][1] = 0;

  polys[6].quad[0][2] = 0;

  polys[6].quad[0][3] = 0;

  polys[6].quad[0][4] = 0;

  polys[6].quad[1][0] = 0;

  polys[6].quad[1][1] = 0;

  polys[6].quad[1][2] = 0;

  polys[6].quad[1][3] = 0;

  polys[6].quad[1][4] = 0;

  polys[6].quad[2][0] = 1;

  polys[6].quad[2][1] = 1;

  polys[6].quad[2][2] = 0;

  polys[6].quad[2][3] = 0;

  polys[6].quad[2][4] = 0;

  polys[6].quad[3][0] = 1;

  polys[6].quad[3][1] = 1;

  polys[6].quad[3][2] = 1;

  polys[6].quad[3][3] = 0;

  polys[6].quad[3][4] = 0;

  polys[6].quad[4][0] = 1;

  polys[6].quad[4][1] = 0;

  polys[6].quad[4][2] = 1;

  polys[6].quad[4][3] = 1;

  polys[6].quad[4][4] = 0;

  // Initialize polynomial 7
  polys[7].constant = 0;

  polys[7].linear[0] = 0;

  polys[7].linear[1] = 0;

  polys[7].linear[2] = 1;

  polys[7].linear[3] = 1;

  polys[7].linear[4] = 1;

  polys[7].quad[0][0] = 0;

  polys[7].quad[0][1] = 0;

  polys[7].quad[0][2] = 0;

  polys[7].quad[0][3] = 0;

  polys[7].quad[0][4] = 0;

  polys[7].quad[1][0] = 1;

  polys[7].quad[1][1] = 0;

  polys[7].quad[1][2] = 0;

  polys[7].quad[1][3] = 0;

  polys[7].quad[1][4] = 0;

  polys[7].quad[2][0] = 0;

  polys[7].quad[2][1] = 0;

  polys[7].quad[2][2] = 0;

  polys[7].quad[2][3] = 0;

  polys[7].quad[2][4] = 0;

  polys[7].quad[3][0] = 0;

  polys[7].quad[3][1] = 1;

  polys[7].quad[3][2] = 1;

  polys[7].quad[3][3] = 0;

  polys[7].quad[3][4] = 0;

  polys[7].quad[4][0] = 0;

  polys[7].quad[4][1] = 1;

  polys[7].quad[4][2] = 0;

  polys[7].quad[4][3] = 1;

  polys[7].quad[4][4] = 0;

  // Initialize polynomial 8
  polys[8].constant = 1;

  polys[8].linear[0] = 0;

  polys[8].linear[1] = 1;

  polys[8].linear[2] = 0;

  polys[8].linear[3] = 0;

  polys[8].linear[4] = 1;

  polys[8].quad[0][0] = 0;

  polys[8].quad[0][1] = 0;

  polys[8].quad[0][2] = 0;

  polys[8].quad[0][3] = 0;

  polys[8].quad[0][4] = 0;

  polys[8].quad[1][0] = 0;

  polys[8].quad[1][1] = 0;

  polys[8].quad[1][2] = 0;

  polys[8].quad[1][3] = 0;

  polys[8].quad[1][4] = 0;

  polys[8].quad[2][0] = 0;

  polys[8].quad[2][1] = 0;

  polys[8].quad[2][2] = 0;

  polys[8].quad[2][3] = 0;

  polys[8].quad[2][4] = 0;

  polys[8].quad[3][0] = 0;

  polys[8].quad[3][1] = 0;

  polys[8].quad[3][2] = 1;

  polys[8].quad[3][3] = 0;

  polys[8].quad[3][4] = 0;

  polys[8].quad[4][0] = 1;

  polys[8].quad[4][1] = 1;

  polys[8].quad[4][2] = 1;

  polys[8].quad[4][3] = 1;

  polys[8].quad[4][4] = 0;

  // Initialize polynomial 9
  polys[9].constant = 0;

  polys[9].linear[0] = 0;

  polys[9].linear[1] = 1;

  polys[9].linear[2] = 1;

  polys[9].linear[3] = 1;

  polys[9].linear[4] = 0;

  polys[9].quad[0][0] = 0;

  polys[9].quad[0][1] = 0;

  polys[9].quad[0][2] = 0;

  polys[9].quad[0][3] = 0;

  polys[9].quad[0][4] = 0;

  polys[9].quad[1][0] = 1;

  polys[9].quad[1][1] = 0;

  polys[9].quad[1][2] = 0;

  polys[9].quad[1][3] = 0;

  polys[9].quad[1][4] = 0;

  polys[9].quad[2][0] = 1;

  polys[9].quad[2][1] = 0;

  polys[9].quad[2][2] = 0;

  polys[9].quad[2][3] = 0;

  polys[9].quad[2][4] = 0;

  polys[9].quad[3][0] = 0;

  polys[9].quad[3][1] = 1;

  polys[9].quad[3][2] = 0;

  polys[9].quad[3][3] = 0;

  polys[9].quad[3][4] = 0;

  polys[9].quad[4][0] = 1;

  polys[9].quad[4][1] = 0;

  polys[9].quad[4][2] = 0;

  polys[9].quad[4][3] = 1;

  polys[9].quad[4][4] = 0;
}


// Evaluate polynomial f on vector x (of length n). All operations are mod 2.
int eval_poly(Poly *f, int *x, int n) {
  int result = f->constant;
  result ^= f->linear[0] & x[0];
  result ^= f->linear[1] & x[1];
  result ^= f->linear[2] & x[2];
  result ^= f->linear[3] & x[3];
  result ^= f->linear[4] & x[4];
  result ^= f->quad[1][0] & x[1] & x[0];
  result ^= f->quad[2][0] & x[2] & x[0];
  result ^= f->quad[2][1] & x[2] & x[1];
  result ^= f->quad[3][0] & x[3] & x[0];
  result ^= f->quad[3][1] & x[3] & x[1];
  result ^= f->quad[3][2] & x[3] & x[2];
  result ^= f->quad[4][0] & x[4] & x[0];
  result ^= f->quad[4][1] & x[4] & x[1];
  result ^= f->quad[4][2] & x[4] & x[2];
  result ^= f->quad[4][3] & x[4] & x[3];
  return result & 1;
}


typedef struct {
  unsigned int i;
  unsigned int y;
  unsigned int d1[MAX_N];
  unsigned int d2[MAX_N][MAX_N];
} State;

void init_state(State *s, Poly polys[MAX_M], int m, int n) {
  s->i = 0;
  s->y = 0;

  s->d1[0] = 0;
  s->d2[0][0] = 0;
  s->d2[0][1] = 0;
  s->d2[0][2] = 0;
  s->d2[0][3] = 0;
  s->d2[0][4] = 0;
  s->d1[1] = 0;
  s->d2[1][0] = 0;
  s->d2[1][1] = 0;
  s->d2[1][2] = 0;
  s->d2[1][3] = 0;
  s->d2[1][4] = 0;
  s->d1[2] = 0;
  s->d2[2][0] = 0;
  s->d2[2][1] = 0;
  s->d2[2][2] = 0;
  s->d2[2][3] = 0;
  s->d2[2][4] = 0;
  s->d1[3] = 0;
  s->d2[3][0] = 0;
  s->d2[3][1] = 0;
  s->d2[3][2] = 0;
  s->d2[3][3] = 0;
  s->d2[3][4] = 0;
  s->d1[4] = 0;
  s->d2[4][0] = 0;
  s->d2[4][1] = 0;
  s->d2[4][2] = 0;
  s->d2[4][3] = 0;
  s->d2[4][4] = 0;

  // Initialize arrays with polynomial values
  s->y |= ((unsigned int)get_constant(&polys[0])) << 0;
  s->y |= ((unsigned int)get_constant(&polys[1])) << 1;
  s->y |= ((unsigned int)get_constant(&polys[2])) << 2;
  s->y |= ((unsigned int)get_constant(&polys[3])) << 3;
  s->y |= ((unsigned int)get_constant(&polys[4])) << 4;
  s->y |= ((unsigned int)get_constant(&polys[5])) << 5;
  s->y |= ((unsigned int)get_constant(&polys[6])) << 6;
  s->y |= ((unsigned int)get_constant(&polys[7])) << 7;
  s->y |= ((unsigned int)get_constant(&polys[8])) << 8;
  s->y |= ((unsigned int)get_constant(&polys[9])) << 9;
  s->d2[1][0] |= ((unsigned int)get_quad(&polys[0], 1, 0)) << 0;
  s->d2[2][0] |= ((unsigned int)get_quad(&polys[0], 2, 0)) << 0;
  s->d2[2][1] |= ((unsigned int)get_quad(&polys[0], 2, 1)) << 0;
  s->d2[3][0] |= ((unsigned int)get_quad(&polys[0], 3, 0)) << 0;
  s->d2[3][1] |= ((unsigned int)get_quad(&polys[0], 3, 1)) << 0;
  s->d2[3][2] |= ((unsigned int)get_quad(&polys[0], 3, 2)) << 0;
  s->d2[4][0] |= ((unsigned int)get_quad(&polys[0], 4, 0)) << 0;
  s->d2[4][1] |= ((unsigned int)get_quad(&polys[0], 4, 1)) << 0;
  s->d2[4][2] |= ((unsigned int)get_quad(&polys[0], 4, 2)) << 0;
  s->d2[4][3] |= ((unsigned int)get_quad(&polys[0], 4, 3)) << 0;
  s->d2[1][0] |= ((unsigned int)get_quad(&polys[1], 1, 0)) << 1;
  s->d2[2][0] |= ((unsigned int)get_quad(&polys[1], 2, 0)) << 1;
  s->d2[2][1] |= ((unsigned int)get_quad(&polys[1], 2, 1)) << 1;
  s->d2[3][0] |= ((unsigned int)get_quad(&polys[1], 3, 0)) << 1;
  s->d2[3][1] |= ((unsigned int)get_quad(&polys[1], 3, 1)) << 1;
  s->d2[3][2] |= ((unsigned int)get_quad(&polys[1], 3, 2)) << 1;
  s->d2[4][0] |= ((unsigned int)get_quad(&polys[1], 4, 0)) << 1;
  s->d2[4][1] |= ((unsigned int)get_quad(&polys[1], 4, 1)) << 1;
  s->d2[4][2] |= ((unsigned int)get_quad(&polys[1], 4, 2)) << 1;
  s->d2[4][3] |= ((unsigned int)get_quad(&polys[1], 4, 3)) << 1;
  s->d2[1][0] |= ((unsigned int)get_quad(&polys[2], 1, 0)) << 2;
  s->d2[2][0] |= ((unsigned int)get_quad(&polys[2], 2, 0)) << 2;
  s->d2[2][1] |= ((unsigned int)get_quad(&polys[2], 2, 1)) << 2;
  s->d2[3][0] |= ((unsigned int)get_quad(&polys[2], 3, 0)) << 2;
  s->d2[3][1] |= ((unsigned int)get_quad(&polys[2], 3, 1)) << 2;
  s->d2[3][2] |= ((unsigned int)get_quad(&polys[2], 3, 2)) << 2;
  s->d2[4][0] |= ((unsigned int)get_quad(&polys[2], 4, 0)) << 2;
  s->d2[4][1] |= ((unsigned int)get_quad(&polys[2], 4, 1)) << 2;
  s->d2[4][2] |= ((unsigned int)get_quad(&polys[2], 4, 2)) << 2;
  s->d2[4][3] |= ((unsigned int)get_quad(&polys[2], 4, 3)) << 2;
  s->d2[1][0] |= ((unsigned int)get_quad(&polys[3], 1, 0)) << 3;
  s->d2[2][0] |= ((unsigned int)get_quad(&polys[3], 2, 0)) << 3;
  s->d2[2][1] |= ((unsigned int)get_quad(&polys[3], 2, 1)) << 3;
  s->d2[3][0] |= ((unsigned int)get_quad(&polys[3], 3, 0)) << 3;
  s->d2[3][1] |= ((unsigned int)get_quad(&polys[3], 3, 1)) << 3;
  s->d2[3][2] |= ((unsigned int)get_quad(&polys[3], 3, 2)) << 3;
  s->d2[4][0] |= ((unsigned int)get_quad(&polys[3], 4, 0)) << 3;
  s->d2[4][1] |= ((unsigned int)get_quad(&polys[3], 4, 1)) << 3;
  s->d2[4][2] |= ((unsigned int)get_quad(&polys[3], 4, 2)) << 3;
  s->d2[4][3] |= ((unsigned int)get_quad(&polys[3], 4, 3)) << 3;
  s->d2[1][0] |= ((unsigned int)get_quad(&polys[4], 1, 0)) << 4;
  s->d2[2][0] |= ((unsigned int)get_quad(&polys[4], 2, 0)) << 4;
  s->d2[2][1] |= ((unsigned int)get_quad(&polys[4], 2, 1)) << 4;
  s->d2[3][0] |= ((unsigned int)get_quad(&polys[4], 3, 0)) << 4;
  s->d2[3][1] |= ((unsigned int)get_quad(&polys[4], 3, 1)) << 4;
  s->d2[3][2] |= ((unsigned int)get_quad(&polys[4], 3, 2)) << 4;
  s->d2[4][0] |= ((unsigned int)get_quad(&polys[4], 4, 0)) << 4;
  s->d2[4][1] |= ((unsigned int)get_quad(&polys[4], 4, 1)) << 4;
  s->d2[4][2] |= ((unsigned int)get_quad(&polys[4], 4, 2)) << 4;
  s->d2[4][3] |= ((unsigned int)get_quad(&polys[4], 4, 3)) << 4;
  s->d2[1][0] |= ((unsigned int)get_quad(&polys[5], 1, 0)) << 5;
  s->d2[2][0] |= ((unsigned int)get_quad(&polys[5], 2, 0)) << 5;
  s->d2[2][1] |= ((unsigned int)get_quad(&polys[5], 2, 1)) << 5;
  s->d2[3][0] |= ((unsigned int)get_quad(&polys[5], 3, 0)) << 5;
  s->d2[3][1] |= ((unsigned int)get_quad(&polys[5], 3, 1)) << 5;
  s->d2[3][2] |= ((unsigned int)get_quad(&polys[5], 3, 2)) << 5;
  s->d2[4][0] |= ((unsigned int)get_quad(&polys[5], 4, 0)) << 5;
  s->d2[4][1] |= ((unsigned int)get_quad(&polys[5], 4, 1)) << 5;
  s->d2[4][2] |= ((unsigned int)get_quad(&polys[5], 4, 2)) << 5;
  s->d2[4][3] |= ((unsigned int)get_quad(&polys[5], 4, 3)) << 5;
  s->d2[1][0] |= ((unsigned int)get_quad(&polys[6], 1, 0)) << 6;
  s->d2[2][0] |= ((unsigned int)get_quad(&polys[6], 2, 0)) << 6;
  s->d2[2][1] |= ((unsigned int)get_quad(&polys[6], 2, 1)) << 6;
  s->d2[3][0] |= ((unsigned int)get_quad(&polys[6], 3, 0)) << 6;
  s->d2[3][1] |= ((unsigned int)get_quad(&polys[6], 3, 1)) << 6;
  s->d2[3][2] |= ((unsigned int)get_quad(&polys[6], 3, 2)) << 6;
  s->d2[4][0] |= ((unsigned int)get_quad(&polys[6], 4, 0)) << 6;
  s->d2[4][1] |= ((unsigned int)get_quad(&polys[6], 4, 1)) << 6;
  s->d2[4][2] |= ((unsigned int)get_quad(&polys[6], 4, 2)) << 6;
  s->d2[4][3] |= ((unsigned int)get_quad(&polys[6], 4, 3)) << 6;
  s->d2[1][0] |= ((unsigned int)get_quad(&polys[7], 1, 0)) << 7;
  s->d2[2][0] |= ((unsigned int)get_quad(&polys[7], 2, 0)) << 7;
  s->d2[2][1] |= ((unsigned int)get_quad(&polys[7], 2, 1)) << 7;
  s->d2[3][0] |= ((unsigned int)get_quad(&polys[7], 3, 0)) << 7;
  s->d2[3][1] |= ((unsigned int)get_quad(&polys[7], 3, 1)) << 7;
  s->d2[3][2] |= ((unsigned int)get_quad(&polys[7], 3, 2)) << 7;
  s->d2[4][0] |= ((unsigned int)get_quad(&polys[7], 4, 0)) << 7;
  s->d2[4][1] |= ((unsigned int)get_quad(&polys[7], 4, 1)) << 7;
  s->d2[4][2] |= ((unsigned int)get_quad(&polys[7], 4, 2)) << 7;
  s->d2[4][3] |= ((unsigned int)get_quad(&polys[7], 4, 3)) << 7;
  s->d2[1][0] |= ((unsigned int)get_quad(&polys[8], 1, 0)) << 8;
  s->d2[2][0] |= ((unsigned int)get_quad(&polys[8], 2, 0)) << 8;
  s->d2[2][1] |= ((unsigned int)get_quad(&polys[8], 2, 1)) << 8;
  s->d2[3][0] |= ((unsigned int)get_quad(&polys[8], 3, 0)) << 8;
  s->d2[3][1] |= ((unsigned int)get_quad(&polys[8], 3, 1)) << 8;
  s->d2[3][2] |= ((unsigned int)get_quad(&polys[8], 3, 2)) << 8;
  s->d2[4][0] |= ((unsigned int)get_quad(&polys[8], 4, 0)) << 8;
  s->d2[4][1] |= ((unsigned int)get_quad(&polys[8], 4, 1)) << 8;
  s->d2[4][2] |= ((unsigned int)get_quad(&polys[8], 4, 2)) << 8;
  s->d2[4][3] |= ((unsigned int)get_quad(&polys[8], 4, 3)) << 8;
  s->d2[1][0] |= ((unsigned int)get_quad(&polys[9], 1, 0)) << 9;
  s->d2[2][0] |= ((unsigned int)get_quad(&polys[9], 2, 0)) << 9;
  s->d2[2][1] |= ((unsigned int)get_quad(&polys[9], 2, 1)) << 9;
  s->d2[3][0] |= ((unsigned int)get_quad(&polys[9], 3, 0)) << 9;
  s->d2[3][1] |= ((unsigned int)get_quad(&polys[9], 3, 1)) << 9;
  s->d2[3][2] |= ((unsigned int)get_quad(&polys[9], 3, 2)) << 9;
  s->d2[4][0] |= ((unsigned int)get_quad(&polys[9], 4, 0)) << 9;
  s->d2[4][1] |= ((unsigned int)get_quad(&polys[9], 4, 1)) << 9;
  s->d2[4][2] |= ((unsigned int)get_quad(&polys[9], 4, 2)) << 9;
  s->d2[4][3] |= ((unsigned int)get_quad(&polys[9], 4, 3)) << 9;
  s->d1[0] |= ((unsigned int)get_linear(&polys[0], 0)) << 0;
  s->d1[0] |= ((unsigned int)get_linear(&polys[1], 0)) << 1;
  s->d1[0] |= ((unsigned int)get_linear(&polys[2], 0)) << 2;
  s->d1[0] |= ((unsigned int)get_linear(&polys[3], 0)) << 3;
  s->d1[0] |= ((unsigned int)get_linear(&polys[4], 0)) << 4;
  s->d1[0] |= ((unsigned int)get_linear(&polys[5], 0)) << 5;
  s->d1[0] |= ((unsigned int)get_linear(&polys[6], 0)) << 6;
  s->d1[0] |= ((unsigned int)get_linear(&polys[7], 0)) << 7;
  s->d1[0] |= ((unsigned int)get_linear(&polys[8], 0)) << 8;
  s->d1[0] |= ((unsigned int)get_linear(&polys[9], 0)) << 9;
  {
    int bit = (((s->d2[1][0] >> 0) & 1) ^ get_linear(&polys[0], 1));
    s->d1[1] |= ((unsigned int)bit) << 0;
  }
  {
    int bit = (((s->d2[2][1] >> 0) & 1) ^ get_linear(&polys[0], 2));
    s->d1[2] |= ((unsigned int)bit) << 0;
  }
  {
    int bit = (((s->d2[3][2] >> 0) & 1) ^ get_linear(&polys[0], 3));
    s->d1[3] |= ((unsigned int)bit) << 0;
  }
  {
    int bit = (((s->d2[4][3] >> 0) & 1) ^ get_linear(&polys[0], 4));
    s->d1[4] |= ((unsigned int)bit) << 0;
  }
  {
    int bit = (((s->d2[1][0] >> 1) & 1) ^ get_linear(&polys[1], 1));
    s->d1[1] |= ((unsigned int)bit) << 1;
  }
  {
    int bit = (((s->d2[2][1] >> 1) & 1) ^ get_linear(&polys[1], 2));
    s->d1[2] |= ((unsigned int)bit) << 1;
  }
  {
    int bit = (((s->d2[3][2] >> 1) & 1) ^ get_linear(&polys[1], 3));
    s->d1[3] |= ((unsigned int)bit) << 1;
  }
  {
    int bit = (((s->d2[4][3] >> 1) & 1) ^ get_linear(&polys[1], 4));
    s->d1[4] |= ((unsigned int)bit) << 1;
  }
  {
    int bit = (((s->d2[1][0] >> 2) & 1) ^ get_linear(&polys[2], 1));
    s->d1[1] |= ((unsigned int)bit) << 2;
  }
  {
    int bit = (((s->d2[2][1] >> 2) & 1) ^ get_linear(&polys[2], 2));
    s->d1[2] |= ((unsigned int)bit) << 2;
  }
  {
    int bit = (((s->d2[3][2] >> 2) & 1) ^ get_linear(&polys[2], 3));
    s->d1[3] |= ((unsigned int)bit) << 2;
  }
  {
    int bit = (((s->d2[4][3] >> 2) & 1) ^ get_linear(&polys[2], 4));
    s->d1[4] |= ((unsigned int)bit) << 2;
  }
  {
    int bit = (((s->d2[1][0] >> 3) & 1) ^ get_linear(&polys[3], 1));
    s->d1[1] |= ((unsigned int)bit) << 3;
  }
  {
    int bit = (((s->d2[2][1] >> 3) & 1) ^ get_linear(&polys[3], 2));
    s->d1[2] |= ((unsigned int)bit) << 3;
  }
  {
    int bit = (((s->d2[3][2] >> 3) & 1) ^ get_linear(&polys[3], 3));
    s->d1[3] |= ((unsigned int)bit) << 3;
  }
  {
    int bit = (((s->d2[4][3] >> 3) & 1) ^ get_linear(&polys[3], 4));
    s->d1[4] |= ((unsigned int)bit) << 3;
  }
  {
    int bit = (((s->d2[1][0] >> 4) & 1) ^ get_linear(&polys[4], 1));
    s->d1[1] |= ((unsigned int)bit) << 4;
  }
  {
    int bit = (((s->d2[2][1] >> 4) & 1) ^ get_linear(&polys[4], 2));
    s->d1[2] |= ((unsigned int)bit) << 4;
  }
  {
    int bit = (((s->d2[3][2] >> 4) & 1) ^ get_linear(&polys[4], 3));
    s->d1[3] |= ((unsigned int)bit) << 4;
  }
  {
    int bit = (((s->d2[4][3] >> 4) & 1) ^ get_linear(&polys[4], 4));
    s->d1[4] |= ((unsigned int)bit) << 4;
  }
  {
    int bit = (((s->d2[1][0] >> 5) & 1) ^ get_linear(&polys[5], 1));
    s->d1[1] |= ((unsigned int)bit) << 5;
  }
  {
    int bit = (((s->d2[2][1] >> 5) & 1) ^ get_linear(&polys[5], 2));
    s->d1[2] |= ((unsigned int)bit) << 5;
  }
  {
    int bit = (((s->d2[3][2] >> 5) & 1) ^ get_linear(&polys[5], 3));
    s->d1[3] |= ((unsigned int)bit) << 5;
  }
  {
    int bit = (((s->d2[4][3] >> 5) & 1) ^ get_linear(&polys[5], 4));
    s->d1[4] |= ((unsigned int)bit) << 5;
  }
  {
    int bit = (((s->d2[1][0] >> 6) & 1) ^ get_linear(&polys[6], 1));
    s->d1[1] |= ((unsigned int)bit) << 6;
  }
  {
    int bit = (((s->d2[2][1] >> 6) & 1) ^ get_linear(&polys[6], 2));
    s->d1[2] |= ((unsigned int)bit) << 6;
  }
  {
    int bit = (((s->d2[3][2] >> 6) & 1) ^ get_linear(&polys[6], 3));
    s->d1[3] |= ((unsigned int)bit) << 6;
  }
  {
    int bit = (((s->d2[4][3] >> 6) & 1) ^ get_linear(&polys[6], 4));
    s->d1[4] |= ((unsigned int)bit) << 6;
  }
  {
    int bit = (((s->d2[1][0] >> 7) & 1) ^ get_linear(&polys[7], 1));
    s->d1[1] |= ((unsigned int)bit) << 7;
  }
  {
    int bit = (((s->d2[2][1] >> 7) & 1) ^ get_linear(&polys[7], 2));
    s->d1[2] |= ((unsigned int)bit) << 7;
  }
  {
    int bit = (((s->d2[3][2] >> 7) & 1) ^ get_linear(&polys[7], 3));
    s->d1[3] |= ((unsigned int)bit) << 7;
  }
  {
    int bit = (((s->d2[4][3] >> 7) & 1) ^ get_linear(&polys[7], 4));
    s->d1[4] |= ((unsigned int)bit) << 7;
  }
  {
    int bit = (((s->d2[1][0] >> 8) & 1) ^ get_linear(&polys[8], 1));
    s->d1[1] |= ((unsigned int)bit) << 8;
  }
  {
    int bit = (((s->d2[2][1] >> 8) & 1) ^ get_linear(&polys[8], 2));
    s->d1[2] |= ((unsigned int)bit) << 8;
  }
  {
    int bit = (((s->d2[3][2] >> 8) & 1) ^ get_linear(&polys[8], 3));
    s->d1[3] |= ((unsigned int)bit) << 8;
  }
  {
    int bit = (((s->d2[4][3] >> 8) & 1) ^ get_linear(&polys[8], 4));
    s->d1[4] |= ((unsigned int)bit) << 8;
  }
  {
    int bit = (((s->d2[1][0] >> 9) & 1) ^ get_linear(&polys[9], 1));
    s->d1[1] |= ((unsigned int)bit) << 9;
  }
  {
    int bit = (((s->d2[2][1] >> 9) & 1) ^ get_linear(&polys[9], 2));
    s->d1[2] |= ((unsigned int)bit) << 9;
  }
  {
    int bit = (((s->d2[3][2] >> 9) & 1) ^ get_linear(&polys[9], 3));
    s->d1[3] |= ((unsigned int)bit) << 9;
  }
  {
    int bit = (((s->d2[4][3] >> 9) & 1) ^ get_linear(&polys[9], 4));
    s->d1[4] |= ((unsigned int)bit) << 9;
  }
}


void step(State *s) {
  s->i++;
  int k1 = bit1(s->i);
  int k2 = bit2(s->i);
  int is_k2_none = k2 == -1;

  // Branchless update to d1[k1]
  s->d1[k1] ^= s->d2[(is_k2_none + k2) & ~(!(k2 + 1))][k1];
  s->y ^= s->d1[k1];
}


void fes_eval(Poly polys[MAX_M], int m, int n, unsigned int sols[MAX_SOLS], int *nsol) {
  int sol_count = 0;
  State s;

  init_state(&s, polys, m, n);

  // Step 1
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 2
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 3
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 4
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 5
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 6
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 7
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 8
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 9
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 10
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 11
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 12
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 13
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 14
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 15
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 16
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 17
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 18
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 19
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 20
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 21
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 22
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 23
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 24
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 25
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 26
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 27
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 28
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 29
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 30
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  // Step 31
  {
      volatile unsigned int mask = -(s.y == 0);
      volatile unsigned int gray_code = (s.i ^ (s.i >> 1));
      sols[sol_count] = (gray_code & mask) | (sols[sol_count] & ~mask);
      sol_count += (s.y == 0);
      step(&s);
  }
  *nsol = sol_count;
}


int main(void) {
  int n = 5;  // number of variables
  int m = 10; // number of polynomials

  // Precomputed known solution
  int sol[MAX_N] = {1, 1, 1, 0, 0};

  print_str("Known solution:\n");
  for (int i = 0; i < n; i++) {{
    print_int(sol[i]);
  }}

  // Create polynomial system with preloaded values
  Poly polys[MAX_M];
  init_preloaded_polys(polys);

  // Print the input polynomials.
  print_str("\nInput polynomials:\n");
  for (int i = 0; i < m; i++) {{
    print_str("f");
    print_int(i);
    print_str(" = ");
    print_int(polys[i].constant);

    for (int j = 0; j < n; j++) {{
      if (polys[i].linear[j]) {{
        print_str(" + x");
        print_int(j);
      }}
    }}

    for (int k = 0; k < n; k++) {{
      for (int j = 0; j < k; j++) {{
        if (polys[i].quad[k][j]) {{
          print_str(" + x");
          print_int(k);
          print_str("*x");
          print_int(j);
        }}
      }}
    }}

    print_char('\n');
  }}

  // Evaluate (enumerate) all candidate solutions.
  int sol_count;
  unsigned int sols[MAX_SOLS];

  ACTIVATE_COUNTER(0);
  fes_eval(polys, m, n, sols, &sol_count);
  DEACTIVATE_COUNTER(0);

  print_str("\nFound solution: ...\n");
  print_char('\n');
  // Print the found solutions (each printed as an n–bit binary string).
  for (int i = 0; i < sol_count; i++) {{
    unsigned int sol_val = sols[i];
    for (int j = 0; j < n; j++) {{ // Iterate from 0 to n-1 (LSB to MSB)
      print_int((sol_val >> j) & 1);
      print_char(' ');
    }}
    print_char('\n');
  }}
  return 0;
}
