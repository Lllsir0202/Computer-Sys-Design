#include "FLOAT.h"
#include <stdint.h>
#include <assert.h>

FLOAT F_mul_F(FLOAT a, FLOAT b) {
  // printf("F_mul_F: a = %d, b = %d\n", a, b);
  // printf("F_mul_F: a * b = %d\n", (int)(a*b >> 16));
  // 会出现溢出的Undefined Behavior
  // 这里的a和b都是32位的整数,所以乘法会溢出
  // 这里采用分段乘法来做处理
  
  int a_hi = a >> 16;
  // a high 16 bits
  int a_lo = a & 0xFFFF;
  // a low 16 bits
  int b_hi = b >> 16;
  // b high 16 bits
  int b_lo = b & 0xFFFF;
  // b low 16 bits

  // 计算
  int high = (a_hi * b_hi) << 16;
  int mid1 = (a_hi * b_lo);
  int mid2 = (a_lo * b_hi);

  return high + mid1 + mid2;
}

FLOAT F_div_F(FLOAT a, FLOAT b) {
  printf("F_div_F: a = %d, b = %d\n", a, b);
  if(b == 0) {
    assert(0);
  }
  return (a << 16) / b;
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
  uint32_t a_int = *(uint32_t*)&a;
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
  printf("Fabs: a = %d\n", a < 0 ? -a : a);
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
