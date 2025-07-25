#----|-------entry-------|-errorcode-|---irq id---|---handler---|
.globl vecsys;    vecsys:  pushl $0;  pushl $0x80; jmp asm_trap
.globl vecnull;  vecnull:  pushl $0;  pushl   $-1; jmp asm_trap
.globl vectrap;  vectrap:  pushl $0;  pushl $0x81; jmp asm_trap
# ADD in pa4-3
.globl vectime;  vectime:  pushl $0;  pushl $0x20; jmp asm_trap

asm_trap:
  pushal

  pushl %esp
  call irq_handle

  addl $4, %esp

# Add in pa4-2
# 先将栈顶指针切换到新进程的陷阱帧, 然后才根据陷阱帧的内容恢复现场
  movl %eax, %esp

  popal
  addl $8, %esp

  iret
