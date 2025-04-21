#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
  // 其实应该是：NO表示中断号，然后ret_addr表示返回地址
  // 所以我们在这里需要读取NO号的中断，然后将ret_addr压站，因为它是返回地址
  // 懂了，其实就是push(ret_addr)，判断NO是否在limit之内，如果在就进行处理；
  // 反之我们就报问题，虽然i386应该是有特殊的单独处理的。

  // 差点忘了，首先检查下NO
  assert(NO < cpu.IDTR.limit);
  Log("NO is %d",NO);

  rtlreg_t *eflags_ptr = (rtlreg_t*)&cpu.EFLAGS.eflags;
  // 压栈eflags
  rtl_push(eflags_ptr);
  rtlreg_t *cs_ptr = (rtlreg_t*)&cpu.cs;
  // 压栈cs
  rtl_push(cs_ptr);
  // 接下来压栈返回地址
  rtlreg_t *ret_ptr = (rtlreg_t*)&ret_addr;
  rtl_push(ret_ptr);

  // 读出IDT处理
  // 找到表中的位置

  // idt_entry表示所找的那个intr的地址
  uint32_t idt_entry = cpu.IDTR.base + (NO << 3);
  // Log("idt_entry is %x", idt_entry);
  // 首先通过P判断中断是否有效
  // 得到P
  bool P = (paddr_read(idt_entry+5, 1)) >> 7;
  // Log("P is %d", P);
  if(P){
    // Log("idt_entry is %x", idt_entry);
    uint16_t offset_15_0 = paddr_read(idt_entry, 2);
    // Log("offset_15_0 is %x", offset_15_0);
    uint16_t offset_31_16 = paddr_read(idt_entry+6, 2);
    // Log("offset_31_16 is %x", offset_31_16);
    // cpu.eip = (offset_31_16 << 16) | offset_15_0;
    decoding.jmp_eip = (offset_31_16 << 16) | offset_15_0;
    decoding.is_jmp = 1;
    // Log("eip is %x", cpu.eip);
    return;
  }
  panic("Invalid intr: P is 0");
}

void dev_raise_intr() {
}
