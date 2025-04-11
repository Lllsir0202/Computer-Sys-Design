#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  // Log("id_dest val is %x",id_dest->val);
  // Log("id_dest reg is %x", id_dest->reg);
  rtl_push(&id_dest->val);

  print_asm_template1(push);
}

make_EHelper(pop) {
  // rtl_pop(&id_dest->val);
  rtl_pop(&t0);
  operand_write(id_dest, &t0);

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  uint32_t temp = cpu.esp;

  rtl_push(&cpu.eax);
  rtl_push(&cpu.ecx);
  rtl_push(&cpu.edx);
  rtl_push(&cpu.ebx);
  rtl_push(&temp);
  rtl_push(&cpu.ebp);
  rtl_push(&cpu.esi);
  rtl_push(&cpu.edi);

  print_asm("pusha");
}

make_EHelper(popa) {
  uint32_t temp;
  rtl_pop(&cpu.edi);
  rtl_pop(&cpu.esi);
  rtl_pop(&cpu.ebp);
  rtl_pop(&temp);
  rtl_pop(&cpu.ebx);
  rtl_pop(&cpu.edx);
  rtl_pop(&cpu.ecx);
  // eax已经被设置为了ret_val，所以不能恢复
  rtl_pop(&temp);

  print_asm("popa");
}

make_EHelper(leave) {
  // 首先需要esp = ebp;
  // 然后ebp = 栈顶弹出的值
  rtl_mv(&cpu.esp, &cpu.ebp);
  rtl_pop(&cpu.ebp);

  print_asm("leave");
}

make_EHelper(cltd) {
  // manual -> cdq/cwd
  if (decoding.is_operand_size_16) {
    if((int16_t)cpu.gpr[0]._16 < 0) {
      cpu.gpr[2]._16 = 0xFFFF;
    }else{
      cpu.gpr[2]._16 = 0;
    }
  }
  else {
    if((int32_t)cpu.eax < 0) {
      cpu.edx = 0xFFFFFFFF;
    }else {
      cpu.edx = 0;
    }
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {
  if (decoding.is_operand_size_16) {
    rtl_lr_b(&t0, R_AL);
    rtl_sext(&t0, &t0, 1);
    rtl_sr_w(R_AX, &t0);
  }
  else {
    rtl_lr_w(&t0, R_AX);
    rtl_sext(&t0, &t0, 2);
    rtl_sr_l(R_EAX, &t0);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  // 从这里知道，其实我们有两个宽度，那么就好做了
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  // Log("id_dest->width is %d", id_dest->width);
  // Log("id_src->width is %d", id_src->width);
  // Log("id_src->val is %x", id_src->val);
  // Log("id_dest->val is %x", id_dest->val);
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
