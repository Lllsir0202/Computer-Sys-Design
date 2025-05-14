#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  // printf("F_mul_F: a = %d, b = %d\n", a, b);
  // printf("F_mul_F: a * b = %d\n", (int)(a*b >> 16));
  // 会出现溢出的Undefined Behavior
  // 这里的a和b都是32位的整数,所以乘法会溢出
  int sign = 1;
  if((a < 0 && b > 0) || (a > 0 && b < 0)) {
    sign = -1;
  }
  // 取绝对值
  uint32_t ua = a < 0 ? -a : a;
  uint32_t ub = b < 0 ? -b : b;
  
  // 拆分运算
  uint32_t a_high = ua >> 16;
  uint32_t a_low = ua & 0xFFFF;
  uint32_t b_high = ub >> 16;
  uint32_t b_low = ub & 0xFFFF;

  uint32_t result = a_high * b_high;
  uint32_t mid1 = a_high * b_low;
  uint32_t mid2 = a_low * b_high;
  uint32_t mid = (mid1 & 0xFFFF0000) + (mid2 & 0xFFFF0000) + (((mid1 & 0xFFFF) + (mid2 & 0xFFFF)) & 0xFFFF0000);
  result += mid >> 16;
  result += (a_low * b_low) >> 16;
  return sign > 0 ? result : -result;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  // printf("F_div_F: a = %d, b = %d\n", a, b);
  if(b == 0) {
    assert(0);
  }
  
  // 处理符号位
  int sign = 1;
  if ((a < 0 && b > 0) || (a > 0 && b < 0)) {
    sign = -1;
  }
  
  // 取绝对值
  uint32_t ua = a < 0 ? -a : a;
  uint32_t ub = b < 0 ? -b : b;
  
  // 结果的整数部分
  uint32_t int_part = ua / ub;
  
  // 计算余数
  uint32_t remainder = ua % ub;
  
  // 计算小数部分（余数扩大2^16倍再除）
  uint32_t frac_part = 0;
  for (int i = 0; i < 16; i++) {
    remainder <<= 1;
    frac_part <<= 1;
    if (remainder >= ub) {
      remainder -= ub;
      frac_part |= 1;
    }
  }
  
  // 组合整数部分和小数部分
  uint32_t result = (int_part << 16) | frac_part;
  
  // printf("F_div_F: a / b = %d\n", sign > 0 ? result : -result);
  return sign > 0 ? result : -result;
}

// 用于处理float -> FLOAT的转换
// -> 使用float不能这么做，因为union编译后会出现奇奇怪怪的指令
// union {
//   float f;
//   struct {
//     uint32_t fraction : 23;
//     uint32_t exponent : 8;
//     uint32_t sign : 1;
//   } bits;
// } float_union;

// 定义一些宏的位运算来进行处理
#define get_sign(x) (x) >> 31
#define get_exponent(x) ((x) >> 23) & 0xFF
#define get_fraction(x) (x) & 0x7FFFFF
// union {
//   float f;
//   uint32_t i;
// } float_union;

extern void memcpy(void *dest, const void *src, int n);

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
  printf("f2F: a = \n");
  if(a == 0) {
    return 0;
  }
  printf("reach here0\n");
  uint32_t a_int;
  memcpy(&a_int, &a, sizeof(uint32_t));
  printf("reach here1\n");
  // offset表示偏移量,可以通过这里得到整数位
  uint32_t offset = get_exponent(a_int) - 127;
  uint32_t result = get_sign(a_int) << 23 | get_fraction(a_int);
  // 这里的result是offset前的浮点数
  printf("reach here1\n");
  FLOAT res;
  uint32_t swift = offset - 23 + 16;
  if(swift > 0) {
    res = result << swift;
  } else {
    res = result >> -swift;
  }
  printf("reach here2\n");
  if(get_sign(a_int)) {
    res = -res;
  }
  printf("reach here3\n");
  printf("f2F: res = %d\n", res);
  return res;
}

FLOAT Fabs(FLOAT a) {
  // printf("Fabs: a = %d\n", a < 0 ? -a : a);
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
