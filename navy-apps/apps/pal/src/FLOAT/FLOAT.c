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
  uint32_t sign = float_union.bits.sign;
  uint32_t exponent = float_union.bits.exponent;
  uint32_t fraction = float_union.bits.fraction;
  // offset表示偏移量,可以通过这里得到整数位
  uint8_t offset = exponent - 127;
  uint32_t result = 1 << 23 | fraction;
  // 这里的result是offset前的浮点数
  uint16_t interger = result >> (23 - offset);
  uint16_t fraction2 = result & ((1 << (23 - offset)) - 1);
  FLOAT F;
  F = (sign << 31) | interger << 16 | fraction2;

  return F;
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
