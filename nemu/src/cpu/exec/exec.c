#include "cpu/exec.h"
#include "all-instr.h"

// 这里其实相当于把这里的所有可能的情况，都通过这里的函数指针进行记录
// 需要执行的时候，其实更换下函数指针，传入eip，就可以一样的执行了
// 其实本质上是避免太多的if-else或者switch-case
typedef struct {
  DHelper decode;
  EHelper execute;
  int width;
} opcode_entry;

#define IDEXW(id, ex, w)   {concat(decode_, id), concat(exec_, ex), w}
#define IDEX(id, ex)       IDEXW(id, ex, 0)
#define EXW(ex, w)         {NULL, concat(exec_, ex), w}
#define EX(ex)             EXW(ex, 0)
#define EMPTY              EX(inv)

static inline void set_width(int width) {
  if (width == 0) {
    width = decoding.is_operand_size_16 ? 2 : 4;
  }
  decoding.src.width = decoding.dest.width = decoding.src2.width = width;
}

/* Instruction Decode and EXecute */
static inline void idex(vaddr_t *eip, opcode_entry *e) {
  /* eip is pointing to the byte next to opcode */
  if (e->decode)
    e->decode(eip);
  e->execute(eip);
}

static make_EHelper(2byte_esc);

#define make_group(name, item0, item1, item2, item3, item4, item5, item6, item7) \
  static opcode_entry concat(opcode_table_, name) [8] = { \
    /* 0x00 */	item0, item1, item2, item3, \
    /* 0x04 */	item4, item5, item6, item7  \
  }; \
static make_EHelper(name) { \
  idex(eip, &concat(opcode_table_, name)[decoding.ext_opcode]); \
}

/* 0x80, 0x81, 0x83 */
// R/M with intermediate
// 00 -> ADD
// 01	-> OR
// 02	-> ADC
// 03	-> SBB
// 04	-> AND
// 05	-> SUB
// 06	-> XOR
// 07	-> CMP
make_group(gp1,
    EX(add), EMPTY, EMPTY, EX(sbb),
    EX(and), EX(sub), EMPTY, EX(cmp))

  /* 0xc0, 0xc1, 0xd0, 0xd1, 0xd2, 0xd3 */
  // 07 -> sar
  // 04 -> shl
  // 05 -> shr
make_group(gp2,
    EMPTY, EMPTY, EMPTY, EMPTY,
    EX(shl), EX(shr), EMPTY, EX(sar))

  /* 0xf6, 0xf7 */
  // 00 -> test IDEX(test_I,test)
  // 02 -> not
  // 07 -> idiv
  // 05 -> imul -> one operater
  // 04 -> mul  
make_group(gp3,
    IDEX(test_I, test), EMPTY, EX(not), EMPTY,
    EX(mul), EX(imul1), EX(div), EX(idiv))

  /* 0xfe */
  // 00 -> inc
  // 01 -> dec
make_group(gp4,
    EX(inc), EX(dec), EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY)

  /* 0xff */
  // 0x06 push
  // 0x05 jmp
  // 0x04 jmp rel
  // 0x02 call r/m16
  // 0x00 inc
make_group(gp5,
    EX(inc), EMPTY, EX(call_rm), EMPTY,
    EX(jmp_rm), EX(jmp_rm), EX(push), EMPTY)

  /* 0x0f 0x01*/
make_group(gp7,
    EMPTY, EMPTY, EMPTY, EMPTY,
    EMPTY, EMPTY, EMPTY, EMPTY)

/* TODO: Add more instructions!!! */

// 在这里似乎应该就是使用opcode去索引 译码函数, 执行函数, 以及操作数宽度
// 目前不懂如何去索引两个0xfc的数据
opcode_entry opcode_table [512] = {
  /* 0x00 */	EMPTY, IDEX(G2E,add), EMPTY, IDEX(E2G,add), // 0x01
  /* 0x04 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x08 */	IDEXW(G2E,or,1), IDEX(G2E,or), IDEXW(E2G,or,1), EMPTY, // 0x0a or
  /* 0x0c */	EMPTY, EMPTY, EMPTY, EX(2byte_esc),
  /* 0x10 */	EMPTY, EMPTY, EMPTY, IDEX(E2G,adc),
  /* 0x14 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x18 */	IDEX(G2E, sbb), IDEX(G2E, sbb), IDEXW(E2G, sbb, 1), IDEX(E2G, sbb), // 0x18-0x1b sbb
  /* 0x1c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x20 */	IDEXW(G2E, and, 1), IDEX(G2E, and), IDEXW(E2G, and, 1), IDEX(E2G, and), // 0x20-0x23 and
  /* 0x24 */	IDEXW(I2a, and, 1), IDEX(I2a, and), EMPTY, EMPTY, // 0x24-0x25 and  
  /* 0x28 */	IDEXW(G2E, sub, 1), IDEX(G2E, sub), IDEXW(E2G, sub, 1), IDEX(E2G, sub), // 0x28-0x2b sub
  /* 0x2c */	EMPTY, EMPTY, EMPTY, EMPTY, 
  /* 0x30 */	IDEXW(G2E,xor,1), IDEX(G2E,xor), IDEXW(E2G,xor,1), IDEX(E2G,xor), // 0x30-0x34 XOR
  /* 0x34 */	EMPTY, EMPTY, EMPTY, EMPTY, // 0x34-0x35 XOR
  /* 0x38 */	IDEXW(G2E,cmp, 1), IDEX(G2E,cmp), IDEXW(E2G,cmp, 1), IDEX(E2G,cmp), // 0x38 cmp 0x39 cmp 0x3b cmp
  /* 0x3c */	IDEXW(I2a,cmp, 1), IDEX(I2a,cmp), EMPTY, EMPTY, // 0x3c cmp al,imm8 0x3d 
  /* 0x40 */	IDEX(r,inc), IDEX(r,inc), IDEX(r,inc), IDEX(r,inc), // 0x40 - 0x43 inc
  /* 0x44 */	IDEX(r,inc), IDEX(r,inc), IDEX(r,inc), IDEX(r,inc), // 0x44 - 0x47 inc
  /* 0x48 */	IDEX(r,dec), IDEX(r,dec), IDEX(r,dec), IDEX(r,dec), // 0x48 dec 0x4b dec
  /* 0x4c */	IDEX(r,dec), IDEX(r,dec), IDEX(r,dec), IDEX(r,dec),  // 不如一起填了，反正都一样
  /* 0x50 */	IDEX(r,push), IDEX(r,push), IDEX(r,push), IDEX(r,push), // 0x50-0x57  push r32
  /* 0x54 */	IDEX(r,push), IDEX(r,push), IDEX(r,push), IDEX(r,push), // 0x50-0x57  push r32
  /* 0x58 */	IDEX(pop_r, pop), IDEX(pop_r, pop), IDEX(pop_r, pop), IDEX(pop_r, pop), // 0x58-0x5F pop r32
  /* 0x5c */	IDEX(pop_r, pop), IDEX(pop_r, pop), IDEX(pop_r, pop), IDEX(pop_r, pop), // 0x58-0x5F pop r32
  /* 0x60 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x64 */	EMPTY, EMPTY, EX(operand_size), EMPTY,
  /* 0x68 */	IDEX(push_SI, push), EMPTY, IDEXW(push_SI, push, 1), EMPTY, // 0x68 push imm16/imm32 0x6a push
  /* 0x6c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x70 */	EMPTY, EMPTY, IDEXW(J,jcc,1), EMPTY, // 0x72 jb/jnae 0x73 jae/jnb
  /* 0x74 */	IDEXW(J,jcc,1), IDEXW(J,jcc,1), IDEXW(J,jcc,1), IDEXW(J,jcc,1), // 0x74 jz/je 0x75 jnz/jne 0x76 jbe 0x77 ja/jnbe
  /* 0x78 */	EMPTY, EMPTY, EMPTY, EMPTY, 
  /* 0x7c */	IDEXW(J, jcc, 1), IDEXW(J,jcc,1), IDEXW(J,jcc,1), IDEXW(J,jcc,1), // 0x7c jl 0x7d jnl/jge 0x7e jle 0x7f jg/jnle
  /* 0x80 */	IDEXW(I2E, gp1, 1), IDEX(I2E, gp1), EMPTY, IDEX(SI2E, gp1), // 0x83 SUB r/m32,imm8 or SUB r/m16, imm8
  /* 0x84 */	IDEXW(G2E,test,1), IDEX(G2E, test), EMPTY, EMPTY, // 0x84 test 0x85 test
  /* 0x88 */	IDEXW(mov_G2E, mov, 1), IDEX(mov_G2E, mov), IDEXW(mov_E2G, mov, 1), IDEX(mov_E2G, mov),
  /* 0x8c */	EMPTY, IDEX(lea_M2G,lea), EMPTY, EMPTY, // 0x8d lea
  /* 0x90 */	EX(nop), EMPTY, EMPTY, EMPTY,
  /* 0x94 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x98 */	EMPTY, EX(cltd), EMPTY, EMPTY, // 0x99 cltd(cwd,cdq)
  /* 0x9c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xa0 */	IDEXW(O2a, mov, 1), IDEX(O2a, mov), IDEXW(a2O, mov, 1), IDEX(a2O, mov),
  /* 0xa4 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xa8 */	IDEXW(I2a, test, 1), IDEX(I2a, test), EMPTY, EMPTY, // 0xa8 test al,imm8  0xa9 test eax,imm32
  /* 0xac */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xb0 */	IDEXW(mov_I2r, mov, 1), IDEXW(mov_I2r, mov, 1), IDEXW(mov_I2r, mov, 1), IDEXW(mov_I2r, mov, 1),
  /* 0xb4 */	IDEXW(mov_I2r, mov, 1), IDEXW(mov_I2r, mov, 1), IDEXW(mov_I2r, mov, 1), IDEXW(mov_I2r, mov, 1),
  /* 0xb8 */	IDEX(mov_I2r, mov), IDEX(mov_I2r, mov), IDEX(mov_I2r, mov), IDEX(mov_I2r, mov),
  /* 0xbc */	IDEX(mov_I2r, mov), IDEX(mov_I2r, mov), IDEX(mov_I2r, mov), IDEX(mov_I2r, mov),
  /* 0xc0 */	IDEXW(gp2_Ib2E, gp2, 1), IDEX(gp2_Ib2E, gp2), EMPTY, EX(ret), // 0xc3 ret
  /* 0xc4 */	EMPTY, EMPTY, IDEXW(mov_I2E, mov, 1), IDEX(mov_I2E, mov),
  /* 0xc8 */	EMPTY, EX(leave), EMPTY, EMPTY, //0xc9 leave
  /* 0xcc */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xd0 */	IDEXW(gp2_1_E, gp2, 1), IDEX(gp2_1_E, gp2), IDEXW(gp2_cl2E, gp2, 1), IDEX(gp2_cl2E, gp2),
  /* 0xd4 */	EMPTY, EMPTY, EX(nemu_trap), EMPTY,
  /* 0xd8 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xdc */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xe0 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xe4 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xe8 */	IDEX(J,call), IDEX(J, jmp), EMPTY, IDEXW(J,jmp,1),   // 0xe8 CALL rel32/rel16 0xe9 jmp 0xeb 
  /* 0xec */	IDEXW(in_dx2a, in, 1), IDEX(in_dx2a, in), IDEXW(out_a2dx, out, 1), IDEX(out_a2dx, out), // 0xec in al,dx 0xed in eax,dx 0xee out dx,al 0xef out dx,eax
  /* 0xf0 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xf4 */	EMPTY, EMPTY, IDEXW(E, gp3, 1), IDEX(E, gp3),
  /* 0xf8 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xfc */	EMPTY, EMPTY, IDEXW(E, gp4, 1), IDEX(E, gp5),

  /*2 byte_opcode_table */

  /* 0x00 */	EMPTY, IDEX(gp7_E, gp7), EMPTY, EMPTY,
  /* 0x04 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x08 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x0c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x10 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x14 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x18 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x1c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x20 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x24 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x28 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x2c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x30 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x34 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x38 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x3c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x40 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x44 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x48 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x4c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x50 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x54 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x58 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x5c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x60 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x64 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x68 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x6c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x70 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x74 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x78 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x7c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x80 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x84 */	IDEX(J,jcc), IDEX(J,jcc), IDEX(J,jcc), IDEX(J,jcc), // 0x0f84 jz/je 0x0f85 jne/jnz
  /* 0x88 */	IDEX(J,jcc), EMPTY, EMPTY, EMPTY, // 0x0f88 js
  /* 0x8c */	EMPTY, EMPTY, IDEX(J,jcc), EMPTY, // 0x0f8e jng/jle
  /* 0x90 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x94 */	IDEXW(E,setcc,1), IDEXW(E,setcc,1), EMPTY, EMPTY, // 0x0f94 SETcc(E/Z) 0x0f95 SETcc(NE/NZ)
  /* 0x98 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0x9c */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xa0 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xa4 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xa8 */	EMPTY, EMPTY, EMPTY, EMPTY, 
  /* 0xac */	EMPTY, EMPTY, EMPTY, IDEX(E2G, imul2), // 0x0faf imul
  /* 0xb0 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xb4 */	EMPTY, EMPTY, IDEXW(E2G,movzx,1), IDEXW(E2G,movzx,2), // 0x0fb6 movzx 
  /* 0xb8 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xbc */	EMPTY, EMPTY, IDEXW(E2G,movsx,1), IDEXW(E2G,movsx,2), // 0x0fbe 0x0fbf movsx
  /* 0xc0 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xc4 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xc8 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xcc */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xd0 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xd4 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xd8 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xdc */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xe0 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xe4 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xe8 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xec */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xf0 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xf4 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xf8 */	EMPTY, EMPTY, EMPTY, EMPTY,
  /* 0xfc */	EMPTY, EMPTY, EMPTY, EMPTY
};

static make_EHelper(2byte_esc) {
  uint32_t opcode = instr_fetch(eip, 1) | 0x100;
  decoding.opcode = opcode;
  set_width(opcode_table[opcode].width);
  idex(eip, &opcode_table[opcode]);
}

make_EHelper(real) {
  uint32_t opcode = instr_fetch(eip, 1);
  decoding.opcode = opcode;
  set_width(opcode_table[opcode].width);
  idex(eip, &opcode_table[opcode]);
}

static inline void update_eip(void) {
  cpu.eip = (decoding.is_jmp ? (decoding.is_jmp = 0, decoding.jmp_eip) : decoding.seq_eip);
}

void exec_wrapper(bool print_flag) {
#ifdef DEBUG
  decoding.p = decoding.asm_buf;
  decoding.p += sprintf(decoding.p, "%8x:   ", cpu.eip);
#endif

  decoding.seq_eip = cpu.eip;
  exec_real(&decoding.seq_eip);

#ifdef DEBUG
  int instr_len = decoding.seq_eip - cpu.eip;
  sprintf(decoding.p, "%*.s", 50 - (12 + 3 * instr_len), "");
  strncat(decoding.asm_buf, decoding.assembly, 80);
  Log_write("%s\n", decoding.asm_buf);
  if (print_flag) {
    puts(decoding.asm_buf);
  }
#endif

#ifdef DIFF_TEST
  uint32_t eip = cpu.eip;
#endif

  update_eip();

#ifdef DIFF_TEST
  void difftest_step(uint32_t);
  difftest_step(eip);
#endif
}
