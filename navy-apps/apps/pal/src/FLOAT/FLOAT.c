#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  return (a * b) >> 16;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  return (a / b) << 16;
}

// 用于处理float -> FLOAT的转换
union {
  float f;
  struct {
    uint32_t fraction : 23;
    uint32_t exponent : 8;
    uint32_t sign : 1;
  } bits;
} float_union;

union {
  FLOAT F;
  struct {
    uint32_t fraction : 16;
    uint32_t interger : 15;
    uint32_t sign : 1;
  } bits;
} FLOAT_union;

FLOAT f2F(float a) {
  /* You should figure out how to convert `a' into FLOAT without
   * introducing x87 floating point instructions. Else you can
   * not run this code in NEMU before implementing x87 floating
   * point instructions, which is contrary to our expectation.
   *
   * Hint: The bit representation of `a' is already on the
   * stack. How do you retrieve it to another variable without
   * performing arithmetic operations on it directly?
   */
  float_union.f = a;
  // offset表示偏移量,可以通过这里得到整数位
  uint8_t offset = float_union.bits.exponent - 127;
  uint32_t result = 1 << 23 | float_union.bits.fraction;
  // 这里的result是offset前的浮点数
  FLOAT_union.F = 0;
  FLOAT_union.bits.sign = float_union.bits.sign;
  FLOAT_union.bits.interger = result >> (23 - offset);
  FLOAT_union.bits.fraction = result << (offset);

  return FLOAT_union.F;
}

FLOAT Fabs(FLOAT a) {
  return a < 0 ? -a : a;
}

/* Functions below are already implemented */

FLOAT Fsqrt(FLOAT x) {
  FLOAT dt, t = int2F(2);

  do {
    dt = F_div_int((F_div_F(x, t) - t), 2);
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}

FLOAT Fpow(FLOAT x, FLOAT y) {
  /* we only compute x^0.333 */
  FLOAT t2, dt, t = int2F(2);

  do {
    t2 = F_mul_F(t, t);
    dt = (F_div_F(x, t2) - t) / 3;
    t += dt;
  } while(Fabs(dt) > f2F(1e-4));

  return t;
}
