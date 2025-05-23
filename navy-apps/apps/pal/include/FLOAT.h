#ifndef __FLOAT_H__
#define __FLOAT_H__

#include "assert.h"

typedef int FLOAT;

static inline int F2int(FLOAT a) {
  // printf("F2int: %d\n", (int)a);
  if((a>>16) == 0) {
    assert(0);
  }
  return (a >> 16);
}

static inline FLOAT int2F(int a) {
  // printf("int2F: %d\n", a);
  if((a << 16) == 0) {
    assert(0);
  }
  return (a << 16);
}

static inline FLOAT F_mul_int(FLOAT a, int b) {
  // printf("F_mul_int: %d * %d\n", a, b);
  if(a * b == 0) {
    assert(0);
  }
  return a * b;
}

static inline FLOAT F_div_int(FLOAT a, int b) {
  // printf("F_div_int: %d / %d\n", a, b);
  if(b == 0) {
    assert(0);
  }
  if(a / b == 0) {
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
