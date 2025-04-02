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
  rtl_pop(&id_dest->val);

  print_asm_template1(pop);
}

make_EHelper(pusha) {
  TODO();

  print_asm("pusha");
}

make_EHelper(popa) {
  TODO();

  print_asm("popa");
}

make_EHelper(leave) {
  TODO();

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
    TODO();
  }
  else {
    TODO();
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
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
