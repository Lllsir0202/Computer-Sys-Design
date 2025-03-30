#include "cpu/exec.h"

make_EHelper(test) {
  rtl_and(&t0, &id_dest->val, &id_src->val);
  Log("t0 is %x", t0);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  rtl_update_ZFSF(&t0, id_dest->width);
  operand_write(id_dest, &t0);

  print_asm_template2(test);
}

make_EHelper(and) {
  rtl_and(&t0, &id_dest->val, &id_src->val);
  // 接下来考虑EFLAGS的变化
  // 基于i386手册,CF=0,OF=0
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  rtl_update_ZFSF(&t0, id_dest->width);
  operand_write(id_dest, &t0);

  print_asm_template2(and);
}

make_EHelper(xor) {
  // 我们得到了两个寄存器并读到了值
  // 我们只需要进行xor，并进行EFLAGS的更新即可
  rtl_xor(&t0, &id_dest->val, &id_src->val);
  // 接下来设定CF、OF为0
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  // 设置ZF、SF
  rtl_update_ZFSF(&t0, id_dest->width);
  operand_write(id_dest, &t0);

  print_asm_template2(xor);
}

make_EHelper(or) {
  rtl_or(&t0, &id_dest->val, &id_src->val);
  rtl_set_CF(&tzero);
  rtl_set_OF(&tzero);
  rtl_update_ZFSF(&t0, id_dest->width);
  operand_write(id_dest, &t0);

  print_asm_template2(or);
}

make_EHelper(sar) {
  TODO();
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(sar);
}

make_EHelper(shl) {
  TODO();
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shl);
}

make_EHelper(shr) {
  TODO();
  // unnecessary to update CF and OF in NEMU

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  TODO();

  print_asm_template1(not);
}
