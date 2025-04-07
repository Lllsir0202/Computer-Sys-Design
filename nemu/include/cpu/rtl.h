#ifndef __RTL_H__
#define __RTL_H__

#include "nemu.h"

extern rtlreg_t t0, t1, t2, t3;
extern const rtlreg_t tzero;

/* RTL basic instructions */

// static inline void rtl_li(rtlreg_t* dest, uint32_t imm) {
//   *dest = imm;
// }
inline void rtl_li(rtlreg_t* dest, uint32_t imm) {
  *dest = imm;
}

#define c_add(a, b) ((a) + (b))
#define c_sub(a, b) ((a) - (b))
#define c_and(a, b) ((a) & (b))
#define c_or(a, b)  ((a) | (b))
#define c_xor(a, b) ((a) ^ (b))
#define c_shl(a, b) ((a) << (b))
#define c_shr(a, b) ((a) >> (b))
#define c_sar(a, b) ((int32_t)(a) >> (b))
#define c_slt(a, b) ((int32_t)(a) < (int32_t)(b))
#define c_sltu(a, b) ((a) < (b))

#define make_rtl_arith_logic(name) \
  static inline void concat(rtl_, name) (rtlreg_t* dest, const rtlreg_t* src1, const rtlreg_t* src2) { \
    *dest = concat(c_, name) (*src1, *src2); \
  } \
  static inline void concat3(rtl_, name, i) (rtlreg_t* dest, const rtlreg_t* src1, int imm) { \
    *dest = concat(c_, name) (*src1, imm); \
  }


make_rtl_arith_logic(add)
make_rtl_arith_logic(sub)
make_rtl_arith_logic(and)
make_rtl_arith_logic(or)
make_rtl_arith_logic(xor)
make_rtl_arith_logic(shl)
make_rtl_arith_logic(shr)
make_rtl_arith_logic(sar)
make_rtl_arith_logic(slt)
make_rtl_arith_logic(sltu)

static inline void rtl_mul(rtlreg_t* dest_hi, rtlreg_t* dest_lo, const rtlreg_t* src1, const rtlreg_t* src2) {
  asm volatile("mul %3" : "=d"(*dest_hi), "=a"(*dest_lo) : "a"(*src1), "r"(*src2));
}

static inline void rtl_imul(rtlreg_t* dest_hi, rtlreg_t* dest_lo, const rtlreg_t* src1, const rtlreg_t* src2) {
  asm volatile("imul %3" : "=d"(*dest_hi), "=a"(*dest_lo) : "a"(*src1), "r"(*src2));
}

static inline void rtl_div(rtlreg_t* q, rtlreg_t* r, const rtlreg_t* src1_hi, const rtlreg_t* src1_lo, const rtlreg_t* src2) {
  asm volatile("div %4" : "=a"(*q), "=d"(*r) : "d"(*src1_hi), "a"(*src1_lo), "r"(*src2));
}

static inline void rtl_idiv(rtlreg_t* q, rtlreg_t* r, const rtlreg_t* src1_hi, const rtlreg_t* src1_lo, const rtlreg_t* src2) {
  asm volatile("idiv %4" : "=a"(*q), "=d"(*r) : "d"(*src1_hi), "a"(*src1_lo), "r"(*src2));
}

static inline void rtl_lm(rtlreg_t *dest, const rtlreg_t* addr, int len) {
  *dest = vaddr_read(*addr, len);
}

static inline void rtl_sm(rtlreg_t* addr, int len, const rtlreg_t* src1) {
  vaddr_write(*addr, len, *src1);
}

static inline void rtl_lr_b(rtlreg_t* dest, int r) {
  *dest = reg_b(r);
}

static inline void rtl_lr_w(rtlreg_t* dest, int r) {
  *dest = reg_w(r);
}

static inline void rtl_lr_l(rtlreg_t* dest, int r) {
  *dest = reg_l(r);
}

static inline void rtl_sr_b(int r, const rtlreg_t* src1) {
  reg_b(r) = *src1;
}

static inline void rtl_sr_w(int r, const rtlreg_t* src1) {
  reg_w(r) = *src1;
}

static inline void rtl_sr_l(int r, const rtlreg_t* src1) {
  reg_l(r) = *src1;
}

/* RTL psuedo instructions */

static inline void rtl_lr(rtlreg_t* dest, int r, int width) {
  switch (width) {
    case 4: rtl_lr_l(dest, r); return;
    case 1: rtl_lr_b(dest, r); return;
    case 2: rtl_lr_w(dest, r); return;
    default: assert(0);
  }
}

static inline void rtl_sr(int r, int width, const rtlreg_t* src1) {
  switch (width) {
    case 4: rtl_sr_l(r, src1); return;
    case 1: rtl_sr_b(r, src1); return;
    case 2: rtl_sr_w(r, src1); return;
    default: assert(0);
  }
}

static inline void c_set_CF(const rtlreg_t* src);
static inline void c_set_OF(const rtlreg_t* src);
static inline void c_set_ZF(const rtlreg_t* src);
static inline void c_set_SF(const rtlreg_t* src);

static inline void c_get_CF(rtlreg_t* dest);
static inline void c_get_OF(rtlreg_t* dest);
static inline void c_get_ZF(rtlreg_t* dest);
static inline void c_get_SF(rtlreg_t* dest);

#define make_rtl_setget_eflags(f) \
  static inline void concat(rtl_set_, f) (const rtlreg_t* src) { \
    concat(c_set_, f) (src); \
  } \
  static inline void concat(rtl_get_, f) (rtlreg_t* dest) { \
    concat(c_get_, f) (dest); \
  }

make_rtl_setget_eflags(CF)
make_rtl_setget_eflags(OF)
make_rtl_setget_eflags(ZF)
make_rtl_setget_eflags(SF)

// Add: used to set/get eflags
static inline void c_set_CF(const rtlreg_t* src){
  cpu.EFLAGS.CF = (*src == 0) ? 0 : 1;
}

static inline void c_set_OF(const rtlreg_t* src){
  cpu.EFLAGS.OF = (*src == 0) ? 0 : 1;
}

// Not used -> mainly used in set 0
static inline void c_set_ZF(const rtlreg_t* src){
  cpu.EFLAGS.ZF = (*src == 0) ? 0 : 1;
}

// Not used -> mainly used in set 0
static inline void c_set_SF(const rtlreg_t* src){
  cpu.EFLAGS.SF = (*src == 0) ? 0 : 1;
}

static inline void c_get_CF(rtlreg_t* dest){
  *dest = cpu.EFLAGS.CF;
}

static inline void c_get_OF(rtlreg_t* dest){
  *dest = cpu.EFLAGS.OF;
}

static inline void c_get_ZF(rtlreg_t* dest){
  *dest = cpu.EFLAGS.ZF;
}

static inline void c_get_SF(rtlreg_t* dest){
  *dest = cpu.EFLAGS.SF;
}

static inline void rtl_mv(rtlreg_t* dest, const rtlreg_t *src1) {
  // dest <- src1
  *dest = *src1;
}

static inline void rtl_not(rtlreg_t* dest) {
  // dest <- ~dest
  *dest = ~(*dest);
}

static inline void rtl_sext(rtlreg_t* dest, const rtlreg_t* src1, int width) {
  // dest <- signext(src1[(width * 8 - 1) .. 0])
  // Log("src1 val is %x", *src1);
  // panic("here");
  // 有符号拓展，由于我们得不到src1的宽度 -> 查看data-mov的处理，我们知道其实width为id_src->width
  // 其实这里应该都拓展满，因为如果是16位，后面会再去截取
  uint32_t shift = (sizeof(*dest) - width) << 3;
  // Log("id_src val is %x", *src1);
  *dest = ((int32_t)(*src1 << shift)) >> shift;
  // Log("id_dest val is %x", *dest);
}

static inline void rtl_push(const rtlreg_t* src1) {
  // esp <- esp - 4
  // M[esp] <- src1
  cpu.esp = cpu.esp - 4;
  // Log("cpu esp is %x", cpu.esp);
  // Log("src1 is %x", *src1);
  vaddr_write(cpu.esp, 4, *src1);
}

static inline void rtl_pop(rtlreg_t* dest) {
  // dest <- M[esp]
  // esp <- esp + 4
  *dest = vaddr_read(cpu.esp, 4);
  cpu.esp = cpu.esp + 4;
}

static inline void rtl_eq0(rtlreg_t* dest, const rtlreg_t* src1) {
  // dest <- (src1 == 0 ? 1 : 0)
  TODO();
}

static inline void rtl_eqi(rtlreg_t* dest, const rtlreg_t* src1, int imm) {
  // dest <- (src1 == imm ? 1 : 0)
  TODO();
}

static inline void rtl_neq0(rtlreg_t* dest, const rtlreg_t* src1) {
  // dest <- (src1 != 0 ? 1 : 0)
  TODO();
}

static inline void rtl_msb(rtlreg_t* dest, const rtlreg_t* src1, int width) {
  // dest <- src1[width * 8 - 1]
  // 其实就是把src右移8*width-1位(?)
  *dest = ((*src1) >> ((width << 3) - 1)) & 1;
}

static inline void rtl_update_ZF(const rtlreg_t* result, int width) {
  // eflags.ZF <- is_zero(result[width * 8 - 1 .. 0])
  
  uint32_t mask = 0;
  // cpu.EFLAGS.ZF = (((*result) & mask) == 0) ? 1 : 0;
  switch (width)
  {
  case 4:
    mask |= 0xffff0000;
  case 2:
    mask |= 0xff00;
  case 1:
    mask |= 0xff;
    break;
  default:
    Assert(0, "width is not 1, 2 or 4");
  }
  // Log("Result is %x", *result);
  // Log("Mask is %x", mask);
  // Log("eflags.ZF is %d", (((*result) & mask) == 0) ? 1 : 0);
  cpu.EFLAGS.ZF = (((*result) & mask) == 0) ? 1 : 0;
}

static inline void rtl_update_SF(const rtlreg_t* result, int width) {
  // eflags.SF <- is_sign(result[width * 8 - 1 .. 0])
  bool res = ((*result) >> ((width << 3) - 1)) & 1; 
  cpu.EFLAGS.SF = (res) ? 1 : 0;
}

static inline void rtl_update_ZFSF(const rtlreg_t* result, int width) {
  rtl_update_ZF(result, width);
  rtl_update_SF(result, width);
}

#endif
