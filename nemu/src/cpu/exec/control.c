#include "cpu/exec.h"

make_EHelper(jmp) {
  // the target address is calculated at the decode stage
  decoding.is_jmp = 1;

  print_asm("jmp %x", decoding.jmp_eip);
}

make_EHelper(jcc) {
  // the target address is calculated at the decode stage
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  decoding.is_jmp = t2;

  print_asm("j%s %x", get_cc_name(subcode), decoding.jmp_eip);
}

make_EHelper(jmp_rm) {
  decoding.jmp_eip = id_dest->val;
  decoding.is_jmp = 1;

  print_asm("jmp *%s", id_dest->str);
}

make_EHelper(call) {
  // the target address is calculated at the decode stage
  decoding.is_jmp = 1;
  rtl_push(&decoding.seq_eip);

  print_asm("call %x", decoding.jmp_eip);
}

make_EHelper(ret) {
  rtl_pop(&t0);
  decoding.jmp_eip = t0;
  // Log("jmp_eip is %x", t0);
  decoding.is_jmp = 1;

  print_asm("ret");
}

make_EHelper(call_rm) {
  decoding.is_jmp = 1;
  rtl_push(&decoding.seq_eip);
  // Log("id_src is %x", id_src->val);
  // Log("id_dest val is %x, width is %d, seq_eip is %x", id_dest->val, id_dest->width, decoding.seq_eip);
  decoding.jmp_eip = id_dest->val;
  // Log("jmp dest is %x", decoding.jmp_eip);

  print_asm("call *%s", id_dest->str);
}
