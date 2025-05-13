#ifndef __FLOAT_H__
#define __FLOAT_H__

#include "assert.h"

typedef int FLOAT;

static inline int F2int(FLOAT a) {
  printf("F2int: %d\n", a);
  FLOAT nosignF = a & ~(1u << 31);
  int nosigni = nosignF >> 16;
  printf("F2int: %d -> %d\n", a, nosigni);
  return (a & (1u << 31)) ? -nosigni : nosigni;
}

static inline FLOAT int2F(int a) {
  printf("int2F: %d\n", a);
  int nosigni = a > 0 ? a : -a;
  FLOAT nosignF = nosigni << 16;
  printf("int2F: %d -> %d\n", a, nosignF);
  return a < 0 ? (1u << 31) | nosignF : nosignF;
}

static inline FLOAT F_mul_int(FLOAT a, int b) {
  printf("F_mul_int: %d * %d\n", a, b);
  return a * int2F(b);
}

static inline FLOAT F_div_int(FLOAT a, int b) {
  printf("F_div_int: %d / %d\n", a, b);
  if(b == 0) {
    assert(0);
  }
  return a / int2F(b);
}

FLOAT f2F(float);
FLOAT F_mul_F(FLOAT, FLOAT);
FLOAT F_div_F(FLOAT, FLOAT);
FLOAT Fabs(FLOAT);
FLOAT Fsqrt(FLOAT);
FLOAT Fpow(FLOAT, FLOAT);

#endif
