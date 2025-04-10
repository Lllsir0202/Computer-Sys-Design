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
  


}

void dev_raise_intr() {
}
