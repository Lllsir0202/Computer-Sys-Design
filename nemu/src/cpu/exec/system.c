#include "cpu/exec.h"

void diff_test_skip_qemu();
void diff_test_skip_nemu();

void raise_intr(uint8_t NO, vaddr_t ret_addr);

make_EHelper(lidt) {
  //pa3-1 load base and limit
  uint32_t addr = id_dest->addr;
  // Log("addr is %x", addr);
  cpu.IDTR.limit = vaddr_read(addr, 2);
  // Log("limit is %d", cpu.IDTR.limit);
  addr = addr + 2;
  cpu.IDTR.base = vaddr_read(addr, 4);
  // Log("base is %x", cpu.IDTR.base);
  print_asm_template1(lidt);
}

make_EHelper(mov_r2cr) {
  TODO();

  print_asm("movl %%%s,%%cr%d", reg_name(id_src->reg, 4), id_dest->reg);
}

make_EHelper(mov_cr2r) {
  TODO();

  print_asm("movl %%cr%d,%%%s", id_src->reg, reg_name(id_dest->reg, 4));

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(int) {
  // 这里首先读到的id_dest->val表示的是NO
  // 我们传入的ret_addr应该是指令的eip+4; -> eip+2 --> 因为int是2个字节
  uint8_t NO = (uint8_t)id_dest->val;
  raise_intr(NO, cpu.eip+2);
  // Log("In int inst , NO is %d", NO);
  // Log("In in inst , jmp eip is %x", decoding.jmp_eip);
  print_asm("int %s", id_dest->str);

#ifdef DIFF_TEST
  diff_test_skip_nemu();
#endif
}

make_EHelper(iret) {
  // 栈顶到下分别是：eip->cs->eflags
  // eip
  rtl_pop(&t0);
  decoding.jmp_eip = t0;
  decoding.is_jmp = 1;

  // cs
  rtl_pop(&t0);
  cpu.cs = t0;

  // eflags
  rtl_pop(&t0);
  cpu.EFLAGS.eflags = t0;

  print_asm("iret");
}

uint32_t pio_read(ioaddr_t, int);
void pio_write(ioaddr_t, int, uint32_t);

make_EHelper(in) {
  // Log("id_src->val = %x, id_dest->width = %d", id_src->val, id_dest->width);
  t0 = pio_read(id_src->val, id_dest->width);
  // Log("t0 = 0x%x", t0);
  operand_write(id_dest, &t0);

  print_asm_template2(in);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}

make_EHelper(out) {
  // Log("id_dest->val = %x, id_src->width = %d, id_src->val = %d", id_dest->val, id_src->width, id_src->val);
  pio_write(id_dest->val, id_src->width, id_src->val);

  print_asm_template2(out);

#ifdef DIFF_TEST
  diff_test_skip_qemu();
#endif
}
