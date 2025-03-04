#ifndef __CPU_DECODE_H__
#define __CPU_DECODE_H__

#include "common.h"

#include "rtl.h"

// OP_TYPE_REG  ->  寄存器 \
  OP_TYPE_MEM  -> 内存\
  OP_TYPE_IMM  -> 立即数
enum { OP_TYPE_REG, OP_TYPE_MEM, OP_TYPE_IMM };

#define OP_STR_SIZE 40

// 结构体表示的是操作数，也就是上面提到的三种：reg,mem,imm
typedef struct {
  uint32_t type;  // 表示类型，即寄存器、内存、立即数
  int width;  //  表示数据宽度(字节为单位)
  union {
    uint32_t reg; // 如果是寄存器，则表示寄存器的标号
    rtlreg_t addr;  // 如果是mem，那么用来表示地址
    uint32_t imm; // 如果是立即数，这里标识的是unsigned
    int32_t simm; // 如果是立即数，这里标识的是signed
    // 虽然但是，目前的知识我不是很理解，为什么这里的有无符号需要分开()
  };
  rtlreg_t val; //操作数的值，从体系结构的角度理解，应该是说需要在decode阶段取到某一个操作数的值。  \
  我去查看一下rtlreg_t的类型，其实就是uint32_t，但是这里我没有很理解会怎么去处理立即数(?)
  char str[OP_STR_SIZE];  // 字符串类型，应该是调试用的
} Operand;

typedef struct {
  uint32_t opcode;
  vaddr_t seq_eip;  // sequential eip
  bool is_operand_size_16;
  uint8_t ext_opcode;
  bool is_jmp;
  vaddr_t jmp_eip;
  Operand src, dest, src2;
#ifdef DEBUG
  char assembly[80];
  char asm_buf[128];
  char *p;
#endif
} DecodeInfo;

typedef union {
  struct {
    uint8_t R_M		:3;
    uint8_t reg		:3;
    uint8_t mod		:2;
  };
  struct {
    uint8_t dont_care	:3;
    uint8_t opcode		:3;
  };
  uint8_t val;
} ModR_M;

typedef union {
  struct {
    uint8_t base	:3;
    uint8_t index	:3;
    uint8_t ss		:2;
  };
  uint8_t val;
} SIB;

void load_addr(vaddr_t *, ModR_M *, Operand *);
void read_ModR_M(vaddr_t *, Operand *, bool, Operand *, bool);

void operand_write(Operand *, rtlreg_t *);

/* shared by all helper functions */
extern DecodeInfo decoding;

#define id_src (&decoding.src)
#define id_src2 (&decoding.src2)
#define id_dest (&decoding.dest)

#define make_DHelper(name) void concat(decode_, name) (vaddr_t *eip)
typedef void (*DHelper) (vaddr_t *);

make_DHelper(I2E);
make_DHelper(I2a);
make_DHelper(I2r);
make_DHelper(SI2E);
make_DHelper(SI_E2G);
make_DHelper(I_E2G);
make_DHelper(I_G2E);
make_DHelper(I);
make_DHelper(r);
make_DHelper(E);
make_DHelper(gp7_E);
make_DHelper(test_I);
make_DHelper(SI);
make_DHelper(G2E);
make_DHelper(E2G);

make_DHelper(mov_I2r);
make_DHelper(mov_I2E);
make_DHelper(mov_G2E);
make_DHelper(mov_E2G);
make_DHelper(lea_M2G);

make_DHelper(gp2_1_E);
make_DHelper(gp2_cl2E);
make_DHelper(gp2_Ib2E);

make_DHelper(O2a);
make_DHelper(a2O);

make_DHelper(J);

make_DHelper(push_SI);

make_DHelper(in_I2a);
make_DHelper(in_dx2a);
make_DHelper(out_a2I);
make_DHelper(out_a2dx);

#endif
