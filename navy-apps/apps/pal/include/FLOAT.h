#ifndef __FLOAT_H__
#define __FLOAT_H__

#include "assert.h"

typedef int FLOAT;

static inline int F2int(FLOAT a) {
  printf("F2int: %d\n", a);
  int sign = a & (1u << 31);
  return sign ? -(-a >> 16) : (a >> 16);
}

static inline FLOAT int2F(int a) {
  printf("int2F: %d\n", a);
  return a < 0 ? -(-a << 16) : (a << 16);
}

static inline FLOAT F_mul_int(FLOAT a, int b) {
  printf("F_mul_int: %d * %d\n", a, b);
  return a * b;
}

static inline FLOAT F_div_int(FLOAT a, int b) {
  printf("F_div_int: %d / %d\n", a, b);
  if(b == 0) {
    assert(0);
  }
  return a / b;
}

FLOAT f2F(float);
FLOAT F_mul_F(FLOAT, FLOAT);
FLOAT F_div_F(FLOAT, FLOAT);
FLOAT Fabs(FLOAT);
FLOAT Fsqrt(FLOAT);
FLOAT Fpow(FLOAT, FLOAT);

#endif
