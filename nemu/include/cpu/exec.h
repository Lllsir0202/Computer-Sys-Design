#ifndef __CPU_EXEC_H__
#define __CPU_EXEC_H__

#include "nemu.h"

#define make_EHelper(name) void concat(exec_, name) (vaddr_t *eip)
typedef void (*EHelper) (vaddr_t *);

#include "cpu/decode.h"

// 这个函数应该是用于取指令的,
// 返回值是uint32_t类型的指令
// -> 其实最终不一定取的是指令，这个函数的用处很多，比如我们只需要一个byte的时候，我们传入len=1即可
static inline uint32_t instr_fetch(vaddr_t *eip, int len) {
  uint32_t instr = vaddr_read(*eip, len);
#ifdef DEBUG
  uint8_t *p_instr = (void *)&instr;
  int i;
  for (i = 0; i < len; i ++) {
    // 写入debug的p，其实用于输出所有的取到的bytes
    decoding.p += sprintf(decoding.p, "%02x ", p_instr[i]);
  }
#endif
  (*eip) += len;
  return instr;
}

void rtl_setcc(rtlreg_t*, uint8_t);

// 现在还不太理解是什么名字。
// 查询后发现，似乎表示的是跳转的一些指令码。
// o ->  overflow;(OF=1)
// b ->  below;(CF=1)
// e ->  equal;(ZF=1)
// s ->  sign(-)(SF=1);
// p ->  parity(奇偶)(PF=1);
// l ->  less(SF!=OF);
// 但说实话现在不是很理解
static inline const char* get_cc_name(int subcode) {
  static const char *cc_name[] = {
    "o", "no", "b", "nb",
    "e", "ne", "be", "nbe",
    "s", "ns", "p", "np",
    "l", "nl", "le", "nle"
  };
  return cc_name[subcode];
}

#ifdef DEBUG
#define print_asm(...) Assert(snprintf(decoding.assembly, 80, __VA_ARGS__) < 80, "buffer overflow!")
#else
#define print_asm(...)
#endif

#define suffix_char(width) ((width) == 4 ? 'l' : ((width) == 1 ? 'b' : ((width) == 2 ? 'w' : '?')))

#define print_asm_template1(instr) \
  print_asm(str(instr) "%c %s", suffix_char(id_dest->width), id_dest->str)

#define print_asm_template2(instr) \
  print_asm(str(instr) "%c %s,%s", suffix_char(id_dest->width), id_src->str, id_dest->str)

#define print_asm_template3(instr) \
  print_asm(str(instr) "%c %s,%s,%s", suffix_char(id_dest->width), id_src->str, id_src2->str, id_dest->str)

#endif
