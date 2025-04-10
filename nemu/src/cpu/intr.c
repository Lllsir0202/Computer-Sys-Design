#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */
   // 其实应该是：NO表示中断号，然后ret_addr表示返回地址，所以其实在这里我们应该说的是

}

void dev_raise_intr() {
}
