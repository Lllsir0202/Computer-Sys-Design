#include "cpu/exec.h"

make_EHelper(add) {
  rtl_add(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);
  // 首先考虑CF，CF处理的是：本质上其实是无符号计算，
  // 不同于减法，这里的计算考虑是否会超出，那么其实就是，如果超出，那么相加后的数字小于相加前的，
  // 相对来说，sltu则是无符号数比较，t0 < id_dest->val，那么说明加法进位，则为1
  rtl_sltu(&t1, &t0, &id_dest->val);
  rtl_set_CF(&t1);

  // 接下来考虑OF，OF处理的是：在有符号计算中，
  // 如果有符号加法出现了进位
  // 公式：OF = (sign(src1) == sign(src2)) && (sign(result) != sign(src1))
  // 也就是说，当两个操作数的符号一样，并且结果符号和操作数的不同，
  // 那么说明 (+num) + (+num) and result is - OR (-num) + (-num) and result is +
  // 其实本质上是因为无符号表示溢出后，最高位为0->1 OR 1->0
  
  // 如果异或结果最高位为1,那么说明这两个数是同符号，

  rtl_xor(&t2, &id_dest->val, &id_src->val);
  rtl_not(&t2);
  rtl_xor(&t3, &id_dest->val, &t0);
  rtl_and(&t2, &t2, &t3);
  rtl_msb(&t2, &t2, id_dest->width);
  rtl_set_OF(&t2);
  rtl_update_ZFSF(&t0,id_dest->width);

  print_asm_template2(add);
}

make_EHelper(sub) {
  rtl_sub(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);

  // 首先考虑CF，CF处理的是：本质上其实是无符号计算，
  // 这里的CF考虑这里无符号是否够减，不够就需要去借位，因为无符号，所以结果如果为负数，那么就是大数。
  // 相对来说，sltu则是无符号数比较，id_dest->val < t0，那么说明减法借位，则为1
  rtl_sltu(&t1, &id_dest->val, &t0);
  rtl_set_CF(&t1);

  // 接下来考虑OF，OF处理的是：在有符号计算中，
  // 如果有符号减法出现了进位
  // 公式：OF = (sign(src1) != sign(src2)) && (sign(result) != sign(src1))
  // 也就是说，当两个操作数的符号不一样，并且结果符号和第一个的不同，
  // 那么说明 (+num) - (-num) and result is - OR (-num) - (+num) and result is +
  // 其实本质上是因为无符号表示溢出后，最高位为0->1 OR 1->0
  
  // 如果异或结果最高位为1,那么说明这两个数是同符号，
  rtl_xor(&t2, &id_dest->val, &id_src->val);
  rtl_xor(&t3, &id_dest->val, &t0);
  rtl_and(&t2, &t2, &t3);
  rtl_msb(&t2, &t2, id_dest->width);
  rtl_set_OF(&t2);
  rtl_update_ZFSF(&t0,id_dest->width);
  // Log("id_dest->val is %x, id_src->val is %x, t0 is %x", id_dest->val, id_src->val, t0);

  print_asm_template2(sub);
}

make_EHelper(cmp) {
  rtl_sub(&t0, &id_dest->val, &id_src->val);
  // Log("id_dest->val is %x, id_src->val is %x, t0 is %x", id_dest->val, id_src->val, t0);
  // 接下来进行置位即可
  // 本质上就是减法
  // CF:无符号溢出
  // CF = (sign(src1) != sign(src2)) & (sign(result) != sign(src1))
  // (+num) - (-num) -> - || (-num) - (+num) -> +
  rtl_sltu(&t1, &id_dest->val, &t0);
  rtl_set_CF(&t1);
  
  // 对于OF
  // OF:有符号溢出
  // OF = (sign(src1) == sign(src2)) & (sign(result) != sign(src1))
  // (+num) - (+num) -> - || (-num) - (-num) -> +
  rtl_xor(&t1, &id_dest->val, &id_src->val);
  rtl_xor(&t2, &t0, &id_dest->val);
  rtl_and(&t3, &t1, &t2);
  rtl_msb(&t3, &t3, id_dest->width);
  rtl_set_OF(&t3);
  // Log("id_dest->val is %x, id_src->val is %x, t0 is %x", id_dest->val, id_src->val, t0);
  rtl_update_ZFSF(&t0, id_dest->width);

  print_asm_template2(cmp);
}

make_EHelper(inc) {
  t1 = 1;
  rtl_add(&t0, &id_dest->val, &t1);
  // Log("id_dest val is %x, after inc is %x", id_dest->val, t0);
  operand_write(id_dest, &t0);
  // id_dest->val = t0;


  // 首先考虑CF，CF处理的是：本质上其实是无符号计算，
  // 不同于减法，这里的计算考虑是否会超出，那么其实就是，如果超出，那么相加后的数字小于相加前的，
  // 相对来说，sltu则是无符号数比较，t0 < id_dest->val，那么说明加法进位，则为1
  // rtl_sltu(&t1, &t0, &id_dest->val);
  // rtl_set_CF(&t1);

  // 接下来考虑OF，OF处理的是：在有符号计算中，
  // 如果有符号加法出现了进位
  // 公式：OF = (sign(src1) == sign(src2)) && (sign(result) != sign(src1))
  // 也就是说，当两个操作数的符号一样，并且结果符号和操作数的不同，
  // 那么说明 (+num) + (+num) and result is - OR (-num) + (-num) and result is +
  // 其实本质上是因为无符号表示溢出后，最高位为0->1 OR 1->0
  
  // 如果异或结果最高位为1,那么说明这两个数是同符号，
  rtl_xor(&t2, &id_dest->val, &t1);
  rtl_not(&t2);
  rtl_xor(&t3, &id_dest->val, &t0);
  rtl_and(&t2, &t2, &t3);
  rtl_msb(&t2, &t2, id_dest->width);
  rtl_set_OF(&t2);
  rtl_update_ZFSF(&t0,id_dest->width);

  print_asm_template1(inc);
}

make_EHelper(dec) {
  t1 = 1;
  rtl_sub(&t0, &id_dest->val, &t1);
  // Log("id_dest val is %x, after dec is %x", id_dest->val, t0);
  operand_write(id_dest, &t0);
  // id_dest->val = t0;
  // Log("now id_dest val is %x", id_dest->val);

  // 首先考虑CF，CF处理的是：本质上其实是无符号计算，
  // 这里的CF考虑这里无符号是否够减，不够就需要去借位，因为无符号，所以结果如果为负数，那么就是大数。
  // 相对来说，sltu则是无符号数比较，id_dest->val < t0，那么说明减法借位，则为1
  // rtl_sltu(&t1, &id_dest->val, &t0);
  // rtl_set_CF(&t1);

  // 接下来考虑OF，OF处理的是：在有符号计算中，
  // 如果有符号减法出现了进位
  // 公式：OF = (sign(src1) != sign(src2)) && (sign(result) != sign(src1))
  // 也就是说，当两个操作数的符号不一样，并且结果符号和第一个的不同，
  // 那么说明 (+num) - (-num) and result is - OR (-num) - (+num) and result is +
  // 其实本质上是因为无符号表示溢出后，最高位为0->1 OR 1->0
  
  // 如果异或结果最高位为1,那么说明这两个数是同符号，
  rtl_xor(&t2, &id_dest->val, &t1);
  rtl_xor(&t3, &id_dest->val, &t0);
  rtl_and(&t2, &t2, &t3);
  rtl_msb(&t2, &t2, id_dest->width);
  rtl_set_OF(&t2);
  // Log("id_dest->val is %x",id_dest->val);
  rtl_update_ZFSF(&t0,id_dest->width);

  print_asm_template1(dec);
}

make_EHelper(neg) {
  if(id_dest->val == 0) {
    rtl_set_CF(&tzero);
  } else {
    rtl_set_CF(&t1);
  }
  rtl_sub(&t0, &tzero, &id_dest->val);
  operand_write(id_dest, &t0);

  print_asm_template1(neg);
}

make_EHelper(adc) {
  rtl_add(&t2, &id_dest->val, &id_src->val);
  rtl_sltu(&t3, &t2, &id_dest->val);
  rtl_get_CF(&t1);
  rtl_add(&t2, &t2, &t1);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_sltu(&t0, &t2, &id_dest->val);
  rtl_or(&t0, &t3, &t0);
  rtl_set_CF(&t0);

  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_not(&t0);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(adc);
}

make_EHelper(sbb) {
  rtl_sub(&t2, &id_dest->val, &id_src->val);
  rtl_sltu(&t3, &id_dest->val, &t2);
  rtl_get_CF(&t1);
  rtl_sub(&t2, &t2, &t1);
  operand_write(id_dest, &t2);

  rtl_update_ZFSF(&t2, id_dest->width);

  rtl_sltu(&t0, &id_dest->val, &t2);
  rtl_or(&t0, &t3, &t0);
  rtl_set_CF(&t0);

  rtl_xor(&t0, &id_dest->val, &id_src->val);
  rtl_xor(&t1, &id_dest->val, &t2);
  rtl_and(&t0, &t0, &t1);
  rtl_msb(&t0, &t0, id_dest->width);
  rtl_set_OF(&t0);

  print_asm_template2(sbb);
}

make_EHelper(mul) {
  rtl_lr(&t0, R_EAX, id_dest->width);
  rtl_mul(&t0, &t1, &id_dest->val, &t0);

  switch (id_dest->width) {
    case 1:
      rtl_sr_w(R_AX, &t1);
      break;
    case 2:
      rtl_sr_w(R_AX, &t1);
      rtl_shri(&t1, &t1, 16);
      rtl_sr_w(R_DX, &t1);
      break;
    case 4:
      rtl_sr_l(R_EDX, &t0);
      rtl_sr_l(R_EAX, &t1);
      break;
    default: assert(0);
  }

  print_asm_template1(mul);
}

// imul with one operand
make_EHelper(imul1) {
  rtl_lr(&t0, R_EAX, id_dest->width);
  rtl_imul(&t0, &t1, &id_dest->val, &t0);

  switch (id_dest->width) {
    case 1:
      rtl_sr_w(R_AX, &t1);
      break;
    case 2:
      rtl_sr_w(R_AX, &t1);
      rtl_shri(&t1, &t1, 16);
      rtl_sr_w(R_DX, &t1);
      break;
    case 4:
      rtl_sr_l(R_EDX, &t0);
      rtl_sr_l(R_EAX, &t1);
      break;
    default: assert(0);
  }

  print_asm_template1(imul);
}

// imul with two operands
make_EHelper(imul2) {
  rtl_sext(&id_src->val, &id_src->val, id_src->width);
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

  rtl_imul(&t0, &t1, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t1);

  print_asm_template2(imul);
}

// imul with three operands
make_EHelper(imul3) {
  rtl_sext(&id_src->val, &id_src->val, id_src->width);
  rtl_sext(&id_src2->val, &id_src2->val, id_src->width);
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

  rtl_imul(&t0, &t1, &id_src2->val, &id_src->val);
  operand_write(id_dest, &t1);

  print_asm_template3(imul);
}

make_EHelper(div) {
  switch (id_dest->width) {
    case 1:
      rtl_li(&t1, 0);
      rtl_lr_w(&t0, R_AX);
      break;
    case 2:
      rtl_lr_w(&t0, R_AX);
      rtl_lr_w(&t1, R_DX);
      rtl_shli(&t1, &t1, 16);
      rtl_or(&t0, &t0, &t1);
      rtl_li(&t1, 0);
      break;
    case 4:
      rtl_lr_l(&t0, R_EAX);
      rtl_lr_l(&t1, R_EDX);
      break;
    default: assert(0);
  }

  rtl_div(&t2, &t3, &t1, &t0, &id_dest->val);

  rtl_sr(R_EAX, id_dest->width, &t2);
  if (id_dest->width == 1) {
    rtl_sr_b(R_AH, &t3);
  }
  else {
    rtl_sr(R_EDX, id_dest->width, &t3);
  }

  print_asm_template1(div);
}

make_EHelper(idiv) {
  rtl_sext(&id_dest->val, &id_dest->val, id_dest->width);

  switch (id_dest->width) {
    case 1:
      rtl_lr_w(&t0, R_AX);
      rtl_sext(&t0, &t0, 2);
      rtl_msb(&t1, &t0, 4);
      rtl_sub(&t1, &tzero, &t1);
      break;
    case 2:
      rtl_lr_w(&t0, R_AX);
      rtl_lr_w(&t1, R_DX);
      rtl_shli(&t1, &t1, 16);
      rtl_or(&t0, &t0, &t1);
      rtl_msb(&t1, &t0, 4);
      rtl_sub(&t1, &tzero, &t1);
      break;
    case 4:
      rtl_lr_l(&t0, R_EAX);
      rtl_lr_l(&t1, R_EDX);
      break;
    default: assert(0);
  }

  rtl_idiv(&t2, &t3, &t1, &t0, &id_dest->val);

  rtl_sr(R_EAX, id_dest->width, &t2);
  if (id_dest->width == 1) {
    rtl_sr_b(R_AH, &t3);
  }
  else {
    rtl_sr(R_EDX, id_dest->width, &t3);
  }

  print_asm_template1(idiv);
}
